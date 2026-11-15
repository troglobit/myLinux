/* SPDX-License-Identifier: Apache-2.0 */

/*
 * ifupdown-ng-based interface management.  mech only renders (and removes)
 * per-interface drop-ins in /etc/network/interfaces.d/, then runs `ifreload`
 * once to reconcile the running state with them -- ifreload brings up the
 * new, tears down the removed, re-applies the changed, and leaves everything
 * else alone.  A missing interface is declared `link-type dummy` so ifupdown
 * creates it, holds its operstate up, and deletes it again on teardown.
 */
#include <ctype.h>
#include <errno.h>
#include <jansson.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"

#define IFDIR   "/etc/network/interfaces.d"
#define MGMT_IF "eth0"
#define IETF_IP_NS "urn:ietf:params:xml:ns:yang:ietf-ip"
#define DUMMY   "\tlink-type dummy\n"

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


/* Drop an interface's config; the ifreload in tr_commit takes it down. */
static void if_unconfigure(const char *ifname)
{
	if (!is_valid_ifname(ifname) || !strcmp(ifname, MGMT_IF))
		return;
	run("rm -f %s/%s.conf", IFDIR, ifname);
}

/*
 * An interface that does not exist is declared a dummy, so ifupdown-ng
 * creates it (and holds its operstate up).  It has to stay declared as
 * one for as long as it is configured: without that line ifreload
 * removes it, and has nothing to put the new configuration on.
 */
static bool is_dummy(const char *ifname, const char *conf)
{
	char line[128];
	bool dummy = false;
	FILE *fp;

	snprintf(line, sizeof(line), "/sys/class/net/%s", ifname);
	if (access(line, F_OK))
		return true;

	fp = fopen(conf, "r");
	if (!fp)
		return false;
	while (!dummy && fgets(line, sizeof(line), fp))
		dummy = !strcmp(line, DUMMY);
	fclose(fp);

	return dummy;
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

/* Render an interface's drop-in; the ifreload in tr_commit applies it. */
static int if_configure(cxobj *iface)
{
	char *ifname = get_str(iface, "name");
	char path[128];
	bool dummy;
	char *desc;
	cbuf *cb;
	FILE *fp;

	if (!is_valid_ifname(ifname)) {
		WARN("ietf-interfaces: rejecting invalid name \"%s\"", ifname ? ifname : "(null)");
		return 0;
	}
	if (!strcmp(ifname, MGMT_IF)) {
		WARN("ietf-interfaces: refusing to manage %s", MGMT_IF);
		return 0;
	}
	if (is_disabled(iface)) {
		if_unconfigure(ifname);
		return 0;
	}

	cb = cbuf_new();
	if (!cb)
		return -1;
	cprint_addresses(cb, iface, "ipv4");
	cprint_addresses(cb, iface, "ipv6");
	if (!cbuf_len(cb)) {
		if_unconfigure(ifname);		/* nothing to set, clear stale */
		cbuf_free(cb);
		return 0;
	}

	snprintf(path, sizeof(path), "%s/%s.conf", IFDIR, ifname);
	dummy = is_dummy(ifname, path);
	fp = fopen(path, "w");
	if (!fp) {
		WARN("ietf-interfaces: cannot write %s: %s", path, strerror(errno));
		cbuf_free(cb);
		return -1;
	}
	fprintf(fp, "auto %s\n", ifname);
	fprintf(fp, "iface %s\n", ifname);
	if (dummy)
		fputs(DUMMY, fp);
	fputs(cbuf_get(cb), fp);
	cbuf_free(cb);
	/* The description is free text from the datastore and lands in a file
	 * whose grammar has executable hooks (pre-up ...), so a newline would
	 * let it inject a directive.  Drop it rather than write a broken-out
	 * line; everything else is fine inside a quoted ifupdown alias. */
	desc = get_str(iface, "description");
	if (desc && strpbrk(desc, "\r\n"))
		WARN("ietf-interfaces: %s: ignoring description with a newline", ifname);
	else if (desc)
		fprintf(fp, "\talias %s\n", desc);
	fclose(fp);

	return 0;			/* ifreload in tr_commit applies it */
}

int ietf_if_tr_begin(clixon_handle h, transaction_data td)
{
	return 0;
}

int ietf_if_tr_commit_interface(cxobj *src, cxobj *tgt)
{
	cxobj *iface;

	/* Deletions come from the source (running) tree ... */
	iface = NULL;
	while (src && (iface = xml_child_each(src, iface, CX_ELMNT))) {
		if (strcmp(xml_name(iface), "interface"))
			continue;
		if (xml_flag(iface, XML_FLAG_DEL))
			if_unconfigure(get_str(iface, "name"));
	}

	/* ... additions and changes from the target (candidate). */
	iface = NULL;
	while (tgt && (iface = xml_child_each(tgt, iface, CX_ELMNT))) {
		if (strcmp(xml_name(iface), "interface"))
			continue;
		if (xml_flag(iface, XML_FLAG_ADD | XML_FLAG_CHANGE))
			if_configure(iface);
	}

	return 0;
}

int ietf_if_tr_commit(clixon_handle h, transaction_data td)
{
	cxobj *src = transaction_src(td), *tgt = transaction_target(td);
	yang_stmt *yspec = clicon_dbspec_yang(h);
	int slen = 0, tlen = 0;
	cxobj **sif, **tif;

	if (src && clixon_xml_find_instance_id(src, yspec, &sif, &slen,
					       "/if:interfaces") < 0)
		return -1;
	if (tgt && clixon_xml_find_instance_id(tgt, yspec, &tif, &tlen,
					       "/if:interfaces") < 0)
		return -1;

	ietf_if_tr_commit_interface(slen ? sif[0] : NULL,
				    tlen ? tif[0] : NULL);

	/* Reconcile the running state with the drop-ins just rendered. */
	if (run("ifreload -a"))
		WARN("ietf-interfaces: ifreload failed");

	return 0;
}

/*
 * Operational state, read straight from the kernel via `ip -j addr show`, so
 * it reflects every interface -- not just the ones in the datastore -- and
 * carries what a NETCONF client would see: type, index, admin/oper status,
 * mtu, addresses (with origin) and octet counters.
 */
static const char *iana_type(json_t *link)
{
	json_t *li = json_object_get(link, "linkinfo");
	json_t *kind = li ? json_object_get(li, "info_kind") : NULL;
	json_t *lt;

	if (kind && json_is_string(kind)) {
		const char *k = json_string_value(kind);

		if (!strcmp(k, "vlan"))
			return "l2vlan";
		if (!strcmp(k, "bridge"))
			return "bridge";
		/* dummy/veth/... have no iana type; fall back to ethernet */
	}

	lt = json_object_get(link, "link_type");
	if (lt && json_is_string(lt) && !strcmp(json_string_value(lt), "loopback"))
		return "softwareLoopback";

	return "ethernetCsmacd";
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

	if (!name || !json_is_string(name))
		return;

	cprintf(cb,
		"<interface>"
		"<name>%s</name>"
		"<type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">"
		"ianaift:%s</type>",
		json_string_value(name), iana_type(link));
	if (json_is_integer(idx))
		cprintf(cb, "<if-index>%lld</if-index>",
			(long long)json_integer_value(idx));
	cprintf(cb, "<admin-status>%s</admin-status>", admin_status(link));
	cprint_lower(cb, "<oper-status>%s</oper-status>", link, "operstate");
	cprint_lower(cb, "<phys-address>%s</phys-address>", link, "address");
	cprint_stats(cb, link);
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
	.ca_trans_commit = ietf_if_tr_commit,

	.ca_statedata = ietf_if_statedata,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	return &ietf_interfaces_api;
}
