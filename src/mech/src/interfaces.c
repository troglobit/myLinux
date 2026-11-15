/* SPDX-License-Identifier: Apache-2.0 */

/*
 * ifupdown-ng-based interface management.  mech renders one drop-in per
 * configured interface in /etc/network/interfaces.d/, all of them anew
 * on every commit, then runs `ifreload` once to reconcile the running
 * state with them -- ifreload brings up the new, tears down the removed,
 * re-applies the changed, and leaves everything else alone.
 *
 * Bridges, VLAN interfaces, veth pairs and dummies are created, and
 * removed again, by ifupdown-ng.  An interface of any other type is a
 * port of the device: one that is not there has its configuration kept
 * for when it is.
 */
#include <ctype.h>
#include <errno.h>
#include <jansson.h>
#include <net/if.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"

#define IFDIR   "/etc/network/interfaces.d"
#define IETF_IP_NS "urn:ietf:params:xml:ns:yang:ietf-ip"
#define IANA_NS "urn:ietf:params:xml:ns:yang:iana-if-type"
#define MYIF_NS "urn:mylinux:interfaces"

static int run(const char *fmt, ...)
{
	char cmd[256];
	va_list ap;
	int rc;

	va_start(ap, fmt);
	vsnprintf(cmd, sizeof(cmd), fmt, ap);
	va_end(ap);

	rc = system(cmd);
	if (rc == -1)
		return -1;

	return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

static char *get_str(cxobj *xp, char *name)
{
	cxobj *obj = xp ? xml_find(xp, name) : NULL;

	return obj ? xml_body(obj) : NULL;
}

/* ietf-interfaces 'enabled' defaults to true; only explicit false disables. */
static bool is_disabled(cxobj *iface)
{
	char *val = get_str(iface, "enabled");

	return val && !strcmp(val, "false");
}

/*
 * The interface name reaches a shell, a file path, and an ifupdown hook, so
 * constrain it to a kernel-legal name (letters, digits, and _-. up to
 * IFNAMSIZ-1) before use -- no '/', no shell metacharacters.
 */
static bool is_valid_ifname(const char *name)
{
	size_t i, n;

	if (!name)
		return false;
	n = strlen(name);
	if (n == 0 || n > 15 || !strcmp(name, ".") || !strcmp(name, ".."))
		return false;
	for (i = 0; i < n; i++) {
		char c = name[i];

		if (!isalnum((unsigned char)c) && c != '_' && c != '-' && c != '.')
			return false;
	}

	return true;
}


/* True if the interface is of the given type, an identity less its prefix */
static bool is_type(cxobj *iface, const char *type)
{
	char *val = get_str(iface, "type");
	char *name = val ? strchr(val, ':') : NULL;

	return val && !strcmp(name ? name + 1 : val, type);
}

/* The addresses of one family, "ipv4" or "ipv6", as ifupdown wants them */
static void cprint_addresses(cbuf *cb, cxobj *iface, char *family)
{
	cxobj *af = xml_find(iface, family);
	cxobj *addr = NULL;

	while (af && (addr = xml_child_each(af, addr, CX_ELMNT))) {
		char *ip = get_str(addr, "ip");
		char *len = get_str(addr, "prefix-length");

		if (!strcmp(xml_name(addr), "address") && ip && len)
			cprintf(cb, "\taddress %s/%s\n", ip, len);
	}
}

/*
 * What ifupdown-ng needs to create the interface, if it is one of those
 * we create.  Returns false for the others, the ports of the device.
 */
static bool cprint_link(cbuf *cb, cxobj *iface)
{
	cxobj *vlan = xml_find(iface, "vlan");
	char *peer = get_str(xml_find(iface, "veth"), "peer");

	if (is_type(iface, "dummy")) {
		cprintf(cb, "\tlink-type dummy\n");
	} else if (is_type(iface, "veth") && peer) {
		/* Whichever end comes up first creates the pair */
		cprintf(cb, "\tlink-type veth\n\tveth-peer-name %s\n", peer);
	} else if (is_type(iface, "l2vlan") && vlan) {
		cprintf(cb, "\tvlan-raw-device %s\n", get_str(vlan, "lower-layer-if"));
		cprintf(cb, "\tvlan-id %s\n", get_str(vlan, "id"));
	} else if (is_type(iface, "bridge")) {
		/* Ports join by themselves, see if_render() */
		cprintf(cb, "\tbridge-ports none\n");
	} else {
		return false;
	}

	return true;
}

/*
 * Render the drop-in of one interface, by a name that ifupdown-ng does
 * not read yet: ietf_if_tr_commit() puts them in place all at once.
 */
static int if_render(cxobj *iface)
{
	char *ifname = get_str(iface, "name");
	cxobj *ipv4 = xml_find(iface, "ipv4");
	char *desc, *bridge;
	char path[128];
	int rc = -1;
	cbuf *cb;
	FILE *fp;

	/* The loopback is in /etc/network/interfaces.  One that is disabled
	 * gets no drop-in, and ifreload takes it down. */
	if (is_type(iface, "softwareLoopback") || is_disabled(iface))
		return 0;

	cb = cbuf_new();
	if (!cb)
		return -1;

	cprintf(cb, "auto %s\n", ifname);
	cprintf(cb, "iface %s\n", ifname);
	if (!cprint_link(cb, iface) && !if_nametoindex(ifname)) {
		NOTE("ietf-interfaces: no %s here, its configuration waits for it", ifname);
		rc = 0;
		goto done;
	}
	/*
	 * A port joins its bridge, and not the bridge its ports: a port
	 * that comes or goes then leaves the bridge and the others alone.
	 */
	bridge = get_str(xml_find(iface, "bridge-port"), "bridge");
	if (bridge) {
		cprintf(cb, "\trequires %s\n", bridge);
		cprintf(cb, "\tpost-up ip link set dev %s master %s\n", ifname, bridge);
		cprintf(cb, "\tpre-down ip link set dev %s nomaster || true\n", ifname);
	}
	if (ipv4 && xml_find(ipv4, "dhcp"))
		cprintf(cb, "\tuse dhcp\n");
	cprint_addresses(cb, iface, "ipv4");
	cprint_addresses(cb, iface, "ipv6");

	/* The description is free text from the datastore and lands in a file
	 * whose grammar has executable hooks (pre-up ...), so a newline would
	 * let it inject a directive.  Drop it rather than write a broken-out
	 * line; everything else is fine inside a quoted ifupdown alias. */
	desc = get_str(iface, "description");
	if (desc && strpbrk(desc, "\r\n"))
		WARN("ietf-interfaces: %s: ignoring description with a newline", ifname);
	else if (desc)
		cprintf(cb, "\talias %s\n", desc);

	snprintf(path, sizeof(path), "%s/%s.conf+", IFDIR, ifname);
	fp = fopen(path, "w");
	if (!fp) {
		WARN("ietf-interfaces: cannot write %s: %s", path, strerror(errno));
		goto done;
	}
	rc = fputs(cbuf_get(cb), fp) < 0 ? -1 : 0;
	if (fclose(fp))
		rc = -1;
done:
	cbuf_free(cb);

	return rc;
}

int ietf_if_tr_begin(clixon_handle h, transaction_data td)
{
	return 0;
}

static cxobj *if_find(cxobj *ifs, const char *ifname)
{
	cxobj *iface = NULL;

	while ((iface = xml_child_each(ifs, iface, CX_ELMNT))) {
		char *name = get_str(iface, "name");

		if (name && !strcmp(name, ifname))
			return iface;
	}

	return NULL;
}

static bool has_addresses(cxobj *iface)
{
	cxobj *ipv4 = xml_find(iface, "ipv4");
	cxobj *ipv6 = xml_find(iface, "ipv6");

	return (ipv4 && (xml_find(ipv4, "address") || xml_find(ipv4, "dhcp"))) ||
	       (ipv6 && xml_find(ipv6, "address"));
}

/*
 * What an interface says of another has to hold.  That the other one is
 * there, the leafref sees to.  Returns what is wrong with it, or NULL.
 */
static const char *if_check(cxobj *ifs, cxobj *iface)
{
	char *ifname = get_str(iface, "name");
	char *peer = get_str(xml_find(iface, "veth"), "peer");
	char *lower = get_str(xml_find(iface, "vlan"), "lower-layer-if");
	char *bridge = get_str(xml_find(iface, "bridge-port"), "bridge");
	cxobj *other;

	/* It ends up in a file that ifupdown-ng reads, and in a path */
	if (!is_valid_ifname(ifname))
		return "not a name an interface can have";
	if (peer) {
		other = if_find(ifs, peer);
		peer = other ? get_str(xml_find(other, "veth"), "peer") : NULL;
		if (!peer || strcmp(peer, ifname))
			return "its veth peer has to have it as peer in turn";
	}
	if (lower && !strcmp(lower, ifname))
		return "it cannot be a VLAN on top of itself";
	if (bridge) {
		other = if_find(ifs, bridge);
		if (other == iface || !other || !is_type(other, "bridge"))
			return "it can only be a port of another interface, a bridge";
		if (has_addresses(iface))
			return "a bridge port has no addresses, they go on the bridge";
	}

	return NULL;
}

int ietf_if_tr_validate(clixon_handle h, transaction_data td)
{
	cxobj *ifs = xpath_first(transaction_target(td), NULL, "interfaces");
	cxobj *iface = NULL;
	const char *err;

	while (ifs && (iface = xml_child_each(ifs, iface, CX_ELMNT))) {
		err = if_check(ifs, iface);
		if (err) {
			clixon_err(OE_CFG, 0, "%s: %s", get_str(iface, "name"), err);
			return -1;
		}
	}

	return 0;
}

int ietf_if_tr_commit(clixon_handle h, transaction_data td)
{
	cxobj *ifs = xpath_first(transaction_target(td), NULL, "interfaces");
	cxobj *iface = NULL;
	int err = 0;

	while (ifs && (iface = xml_child_each(ifs, iface, CX_ELMNT)))
		err |= if_render(iface);

	/* One that we failed to render would look removed, and be taken
	 * down.  Better to leave everything as it is. */
	if (err) {
		run("rm -f %s/*.conf+", IFDIR);
		clixon_err(OE_CFG, 0, "ietf-interfaces: failed writing to %s", IFDIR);
		return -1;
	}

	/* What is not in the configuration has no drop-in */
	run("cd %s && rm -f *.conf && for f in *.conf+; do "
	    "[ -e \"$f\" ] && mv \"$f\" \"${f%%+}\"; done", IFDIR);

	/* Reconcile the running state with the drop-ins just rendered.
	 * What it has to say about that is for the log. */
	if (run("out=$(ifreload -a 2>&1); rc=$?; "
		"[ -z \"$out\" ] || echo \"$out\" | logger -t ifreload -p daemon.warning; exit $rc"))
		WARN("ietf-interfaces: ifreload failed");

	return 0;
}

/*
 * Operational state, read straight from the kernel via `ip -j addr show`, so
 * it reflects every interface -- not just the ones in the datastore -- and
 * carries what a NETCONF client would see: type, index, admin/oper status,
 * mtu, addresses (with origin) and octet counters.
 */
static const char *if_type(json_t *link)
{
	json_t *li = json_object_get(link, "linkinfo");
	json_t *kind = li ? json_object_get(li, "info_kind") : NULL;
	json_t *lt;

	if (kind && json_is_string(kind)) {
		const char *k = json_string_value(kind);

		if (!strcmp(k, "vlan"))
			return "ianaift:l2vlan";
		if (!strcmp(k, "bridge"))
			return "ianaift:bridge";
		if (!strcmp(k, "veth"))
			return "myif:veth";
		if (!strcmp(k, "dummy"))
			return "myif:dummy";
		/* the rest have no type of their own; fall back to ethernet */
	}

	lt = json_object_get(link, "link_type");
	if (lt && json_is_string(lt) && !strcmp(json_string_value(lt), "loopback"))
		return "ianaift:softwareLoopback";

	return "ianaift:ethernetCsmacd";
}

/* What the kernel knows of the mylinux-interfaces augments */
static void cprint_myif(cbuf *cb, json_t *link, const char *type)
{
	json_t *li = json_object_get(link, "linkinfo");
	const char *slave = json_string_value(json_object_get(li, "info_slave_kind"));
	const char *master = json_string_value(json_object_get(link, "master"));
	const char *lower = json_string_value(json_object_get(link, "link"));
	json_t *id = json_object_get(json_object_get(li, "info_data"), "id");

	if (!strcmp(type, "myif:veth") && lower)
		cprintf(cb, "<veth xmlns=\"%s\"><peer>%s</peer></veth>", MYIF_NS, lower);
	if (!strcmp(type, "ianaift:l2vlan") && lower && json_is_integer(id))
		cprintf(cb, "<vlan xmlns=\"%s\"><id>%lld</id><lower-layer-if>%s</lower-layer-if></vlan>",
			MYIF_NS, (long long)json_integer_value(id), lower);
	if (slave && master && !strcmp(slave, "bridge"))
		cprintf(cb, "<bridge-port xmlns=\"%s\"><bridge>%s</bridge></bridge-port>", MYIF_NS, master);
}

/* Admin state is the configured up/down; the kernel reports it in flags. */
static const char *admin_status(json_t *link)
{
	json_t *flags = json_object_get(link, "flags");
	json_t *flag;
	size_t i;

	if (json_is_array(flags)) {
		json_array_foreach(flags, i, flag) {
			if (json_is_string(flag) &&
			    !strcmp(json_string_value(flag), "UP"))
				return "up";
		}
	}

	return "down";
}

/*
 * Best-effort address origin: link-local by scope, DHCP/RA by the kernel's
 * dynamic flag, otherwise assumed static.
 */
static const char *addr_origin(json_t *addr)
{
	json_t *scope = json_object_get(addr, "scope");

	if (scope && json_is_string(scope) &&
	    !strcmp(json_string_value(scope), "link"))
		return "link-layer";
	if (json_is_true(json_object_get(addr, "dynamic")))
		return "dhcp";

	return "static";
}

static void cprint_lower(cbuf *cb, const char *fmt, json_t *obj, const char *key)
{
	json_t *val = json_object_get(obj, key);
	char *str;
	size_t i;

	if (!val || !json_is_string(val))
		return;
	str = strdup(json_string_value(val));
	if (!str)
		return;
	for (i = 0; i < strlen(str); i++)
		str[i] = tolower(str[i]);
	cprintf(cb, fmt, str);
	free(str);
}

static void cprint_stats(cbuf *cb, json_t *link)
{
	json_t *s = json_object_get(link, "stats64");
	json_t *rx = s ? json_object_get(s, "rx") : NULL;
	json_t *tx = s ? json_object_get(s, "tx") : NULL;
	json_t *rxb = rx ? json_object_get(rx, "bytes") : NULL;
	json_t *txb = tx ? json_object_get(tx, "bytes") : NULL;

	if (!json_is_integer(rxb) && !json_is_integer(txb))
		return;
	cprintf(cb, "<statistics>");
	if (json_is_integer(rxb))
		cprintf(cb, "<in-octets>%lld</in-octets>",
			(long long)json_integer_value(rxb));
	if (json_is_integer(txb))
		cprintf(cb, "<out-octets>%lld</out-octets>",
			(long long)json_integer_value(txb));
	cprintf(cb, "</statistics>");
}

/*
 * ietf-ip augment: one address family's mtu and addresses.  `fam` is the
 * iproute2 family string ("inet"/"inet6"), `tag` the YANG container name.
 */
static void cprint_af(cbuf *cb, json_t *link, const char *fam, const char *tag, int mtu)
{
	json_t *addrs = json_object_get(link, "addr_info");
	json_t *addr;
	size_t i;
	int open = 0;

	if (mtu > 0) {
		cprintf(cb, "<%s xmlns=\"%s\"><mtu>%d</mtu>", tag, IETF_IP_NS, mtu);
		open = 1;
	}

	if (json_is_array(addrs)) {
		json_array_foreach(addrs, i, addr) {
			json_t *f = json_object_get(addr, "family");
			json_t *loc = json_object_get(addr, "local");
			json_t *len = json_object_get(addr, "prefixlen");

			if (!f || !json_is_string(f) ||
			    strcmp(json_string_value(f), fam))
				continue;
			if (!json_is_string(loc) || !json_is_integer(len))
				continue;

			if (!open) {
				cprintf(cb, "<%s xmlns=\"%s\">", tag, IETF_IP_NS);
				open = 1;
			}
			cprintf(cb, "<address><ip>%s</ip>"
				"<prefix-length>%lld</prefix-length>"
				"<origin>%s</origin></address>",
				json_string_value(loc),
				(long long)json_integer_value(len),
				addr_origin(addr));
		}
	}

	if (open)
		cprintf(cb, "</%s>", tag);
}

static void cprint_ifdata(cbuf *cb, json_t *link)
{
	json_t *name = json_object_get(link, "ifname");
	json_t *idx = json_object_get(link, "ifindex");
	json_t *mtu = json_object_get(link, "mtu");
	int mtuval = json_is_integer(mtu) ? (int)json_integer_value(mtu) : 0;
	const char *type = if_type(link);

	if (!name || !json_is_string(name))
		return;

	cprintf(cb,
		"<interface>"
		"<name>%s</name>"
		"<type xmlns:ianaift=\"%s\" xmlns:myif=\"%s\">%s</type>",
		json_string_value(name), IANA_NS, MYIF_NS, type);
	if (json_is_integer(idx))
		cprintf(cb, "<if-index>%lld</if-index>",
			(long long)json_integer_value(idx));
	cprintf(cb, "<admin-status>%s</admin-status>", admin_status(link));
	cprint_lower(cb, "<oper-status>%s</oper-status>", link, "operstate");
	cprint_lower(cb, "<phys-address>%s</phys-address>", link, "address");
	cprint_stats(cb, link);
	cprint_myif(cb, link, type);
	cprint_af(cb, link, "inet", "ipv4", mtuval);
	cprint_af(cb, link, "inet6", "ipv6", mtuval);
	cprintf(cb, "</interface>");
}

int ietf_if_statedata(clixon_handle h, cvec *nsc, char *xpath, cxobj *xstate)
{
	json_t *root = NULL, *link;
	cbuf *cb = NULL;
	FILE *fp;
	size_t i;
	int rc = -1;

	fp = popen("ip -s -d -j addr show", "r");
	if (!fp)
		return -1;
	root = json_loadf(fp, 0, NULL);
	pclose(fp);
	if (!root || !json_is_array(root)) {
		rc = 0;
		goto done;
	}

	cb = cbuf_new();
	if (!cb) {
		clixon_err(OE_UNIX, errno, "cbuf_new");
		goto done;
	}

	cprintf(cb, "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">");
	json_array_foreach(root, i, link)
		cprint_ifdata(cb, link);
	cprintf(cb, "</interfaces>");

	if (clixon_xml_parse_string(cbuf_get(cb), YB_NONE, NULL, &xstate, NULL) < 0)
		goto done;
	rc = 0;
done:
	if (cb)
		cbuf_free(cb);
	if (root)
		json_decref(root);
	return rc;
}

static clixon_plugin_api ietf_interfaces_api = {
	.ca_name = "ietf-interfaces",
	.ca_init = clixon_plugin_init,

	.ca_trans_begin = ietf_if_tr_begin,
	.ca_trans_validate = ietf_if_tr_validate,
	.ca_trans_commit = ietf_if_tr_commit,

	.ca_statedata = ietf_if_statedata,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	return &ietf_interfaces_api;
}
