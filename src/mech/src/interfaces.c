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

/* Render an interface's drop-in; the ifreload in tr_commit applies it. */
static int if_configure(cxobj *iface)
{
	char *ifname = get_str(iface, "name");
	char *ip, *len, *desc;
	cxobj *ipv4, *address;
	char path[128];
	int missing;
	FILE *fp;

	if (!is_valid_ifname(ifname)) {
		clixon_log(NULL, LOG_WARNING, "ietf-interfaces: rejecting invalid name \"%s\"",
			   ifname ? ifname : "(null)");
		return 0;
	}
	if (!strcmp(ifname, MGMT_IF)) {
		clixon_log(NULL, LOG_WARNING, "ietf-interfaces: refusing to manage %s", MGMT_IF);
		return 0;
	}
	if (is_disabled(iface)) {
		if_unconfigure(ifname);
		return 0;
	}

	/* XXX: single IPv4 address for now; IPv6 and multiple addresses TODO. */
	ipv4 = xml_find(iface, "ipv4");
	address = ipv4 ? xml_find(ipv4, "address") : NULL;
	ip = address ? get_str(address, "ip") : NULL;
	len = address ? get_str(address, "prefix-length") : NULL;
	if (!ip || !len) {
		if_unconfigure(ifname);		/* nothing to set, clear stale */
		return 0;
	}

	/* If the interface does not exist yet, declare it a dummy so
	 * ifupdown-ng creates it (and holds its operstate up). */
	snprintf(path, sizeof(path), "/sys/class/net/%s", ifname);
	missing = access(path, F_OK);

	snprintf(path, sizeof(path), "%s/%s.conf", IFDIR, ifname);
	fp = fopen(path, "w");
	if (!fp) {
		clixon_log(NULL, LOG_WARNING, "ietf-interfaces: cannot write %s: %s",
			   path, strerror(errno));
		return -1;
	}
	fprintf(fp, "auto %s\n", ifname);
	fprintf(fp, "iface %s inet static\n", ifname);
	if (missing)
		fprintf(fp, "\tlink-type dummy\n");
	fprintf(fp, "\taddress %s/%s\n", ip, len);
	/* The description is free text from the datastore and lands in a file
	 * whose grammar has executable hooks (pre-up ...), so a newline would
	 * let it inject a directive.  Drop it rather than write a broken-out
	 * line; everything else is fine inside a quoted ifupdown alias. */
	desc = get_str(iface, "description");
	if (desc && strpbrk(desc, "\r\n"))
		clixon_log(NULL, LOG_WARNING,
			   "ietf-interfaces: %s: ignoring description with a newline", ifname);
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
		clixon_log(h, LOG_WARNING, "ietf-interfaces: ifreload failed");

	return 0;
}

/*
 * Operational state, read straight from the kernel via `ip -j addr show`, so
 * it reflects every interface -- not just the ones in the datastore.
 */
static const char *iana_type(json_t *link)
{
	json_t *val = json_object_get(link, "link_type");

	if (val && json_is_string(val) &&
	    !strcmp(json_string_value(val), "loopback"))
		return "softwareLoopback";
	return "ethernetCsmacd";
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

/* ietf-ip augment: the IPv4 addresses the kernel has on this interface. */
static void cprint_addrs(cbuf *cb, json_t *link)
{
	json_t *addrs = json_object_get(link, "addr_info");
	json_t *addr;
	size_t i;
	int open = 0;

	if (!addrs || !json_is_array(addrs))
		return;

	json_array_foreach(addrs, i, addr) {
		json_t *fam = json_object_get(addr, "family");
		json_t *loc = json_object_get(addr, "local");
		json_t *len = json_object_get(addr, "prefixlen");

		if (!fam || !json_is_string(fam) ||
		    strcmp(json_string_value(fam), "inet"))
			continue;
		if (!loc || !json_is_string(loc) || !len || !json_is_integer(len))
			continue;

		if (!open) {
			cprintf(cb, "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">");
			open = 1;
		}
		cprintf(cb, "<address><ip>%s</ip><prefix-length>%lld</prefix-length></address>",
			json_string_value(loc), (long long)json_integer_value(len));
	}

	if (open)
		cprintf(cb, "</ipv4>");
}

static void cprint_ifdata(cbuf *cb, json_t *link)
{
	json_t *name = json_object_get(link, "ifname");

	if (!name || !json_is_string(name))
		return;

	cprintf(cb,
		"<interface>"
		"<name>%s</name>"
		"<type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">"
		"ianaift:%s</type>",
		json_string_value(name), iana_type(link));
	cprint_lower(cb, "<phys-address>%s</phys-address>", link, "address");
	cprint_lower(cb, "<oper-status>%s</oper-status>", link, "operstate");
	cprint_addrs(cb, link);
	cprintf(cb, "</interface>");
}

int ietf_if_statedata(clixon_handle h, cvec *nsc, char *xpath, cxobj *xstate)
{
	json_t *root = NULL, *link;
	cbuf *cb = NULL;
	FILE *fp;
	size_t i;
	int rc = -1;

	fp = popen("ip -j addr show", "r");
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
