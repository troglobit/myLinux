/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Admin-exec 'show' renderers.  Each reads operational data the way a
 * remote NETCONF/RESTCONF client would -- clicon_rpc_get() with
 * content=nonconfig, the same path the tests use -- and prints it for a
 * human.  The CLI never queries the kernel directly; mech's statedata
 * callbacks are the single source of truth.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <cligen/cligen.h>
#include <clixon/clixon.h>
#include <clixon/clixon_cli.h>

#define IETF_IF_NS  "urn:ietf:params:xml:ns:yang:ietf-interfaces"
#define IETF_SYS_NS "urn:ietf:params:xml:ns:yang:ietf-system"

#define C_RESET "\033[0m"
#define C_GREEN "\033[32m"
#define C_RED   "\033[31m"
#define C_INV   "\033[7m"

#define W_IFACE 16
#define W_PROTO 11
#define W_STATE 12
#define W_DATA  41		/* pads the header so the inverse bar spans the table */

/* Fetch a subtree of operational (nonconfig) data; caller frees *xret. */
static int rpc_get(clixon_handle h, const char *xpath, cvec *nsc, cxobj **xret)
{
	cxobj *xerr;

	if (clicon_rpc_get(h, xpath, nsc, CONTENT_NONCONFIG, -1, NULL, xret) < 0)
		return -1;
	if ((xerr = xpath_first(*xret, NULL, "/rpc-error")) != NULL) {
		clixon_err_netconf(h, OE_XML, 0, xerr, "get operational");
		return -1;
	}

	return 0;
}

/* iana-if-type body is prefixed ("ianaift:ethernetCsmacd"); make it a word. */
static const char *proto_str(const char *type)
{
	const char *t = type ? strrchr(type, ':') : NULL;

	t = t ? t + 1 : type;
	if (!t)
		return "";
	if (!strcmp(t, "softwareLoopback"))
		return "loopback";
	if (!strcmp(t, "ethernetCsmacd"))
		return "ethernet";
	if (!strcmp(t, "l2vlan"))
		return "vlan";

	return t;			/* bridge, or whatever we don't map */
}

/* STATE column: admin-down reads DISABLED, a carrier-less link (unknown)
 * reads UP; colour green when usable, red otherwise. */
static void pr_state(int color, const char *admin, const char *oper)
{
	char buf[16];
	const char *s;
	int good = 0;

	if (admin && !strcmp(admin, "down"))
		s = "DISABLED";
	else if (oper && (!strcmp(oper, "up") || !strcmp(oper, "unknown"))) {
		s = "UP";
		good = 1;
	} else if (oper) {
		size_t i;

		for (i = 0; oper[i] && i < sizeof(buf) - 1; i++)
			buf[i] = toupper((unsigned char)oper[i]);
		buf[i] = 0;
		s = buf;
	} else
		s = "";

	if (color)
		cligen_output(stdout, "%s%-*s%s", good ? C_GREEN : C_RED,
			      W_STATE, s, C_RESET);
	else
		cligen_output(stdout, "%-*s", W_STATE, s);
}

/* Address continuation rows, one per address in the given family. */
static void pr_addrs(cxobj *iface, const char *fam)
{
	cxobj **vec = NULL;
	size_t len = 0, i;
	char path[24];

	snprintf(path, sizeof(path), "%s/address", fam);
	if (xpath_vec(iface, NULL, path, &vec, &len) < 0)
		return;

	for (i = 0; i < len; i++) {
		char *ip = xml_find_body(vec[i], "ip");
		char *plen = xml_find_body(vec[i], "prefix-length");
		char *orig = xml_find_body(vec[i], "origin");

		cligen_output(stdout, "%-*s%-*s%-*s%s/%s",
			      W_IFACE, "", W_PROTO, fam, W_STATE, "",
			      ip ? ip : "", plen ? plen : "");
		if (orig)
			cligen_output(stdout, " (%s)", orig);
		cligen_output(stdout, "\n");
	}

	free(vec);
}

static void pr_summary(cxobj *iface, int color)
{
	char *name = xml_find_body(iface, "name");
	char *type = xml_find_body(iface, "type");
	char *admin = xml_find_body(iface, "admin-status");
	char *oper = xml_find_body(iface, "oper-status");
	char *mac = xml_find_body(iface, "phys-address");

	cligen_output(stdout, "%-*s%-*s", W_IFACE, name ? name : "",
		      W_PROTO, proto_str(type));
	pr_state(color, admin, oper);
	cligen_output(stdout, "%s\n", mac ? mac : "");

	pr_addrs(iface, "ipv4");
	pr_addrs(iface, "ipv6");
}

static void pr_kv(const char *key, const char *val)
{
	if (val)
		cligen_output(stdout, "%-20s: %s\n", key, val);
}

/* Detail: one address family's addresses, aligned under a labelled row. */
static void pr_detail_addrs(cxobj *iface, const char *fam, const char *label)
{
	cxobj **vec = NULL;
	size_t len = 0, i;
	char path[24];

	snprintf(path, sizeof(path), "%s/address", fam);
	xpath_vec(iface, NULL, path, &vec, &len);

	if (len == 0) {
		cligen_output(stdout, "%-20s:\n", label);
		free(vec);
		return;
	}

	for (i = 0; i < len; i++) {
		char *ip = xml_find_body(vec[i], "ip");
		char *plen = xml_find_body(vec[i], "prefix-length");
		char *orig = xml_find_body(vec[i], "origin");

		cligen_output(stdout, "%-20s%s ", i ? "" : label, i ? " " : ":");
		cligen_output(stdout, "%s/%s", ip ? ip : "", plen ? plen : "");
		if (orig)
			cligen_output(stdout, " (%s)", orig);
		cligen_output(stdout, "\n");
	}

	free(vec);
}

static void pr_detail(cxobj *iface)
{
	char *type = xml_find_body(iface, "type");
	cxobj *x;

	pr_kv("name", xml_find_body(iface, "name"));
	if (type)
		cligen_output(stdout, "%-20s: %s\n", "type", proto_str(type));
	pr_kv("index", xml_find_body(iface, "if-index"));
	if ((x = xpath_first(iface, NULL, "ipv4/mtu")) ||
	    (x = xpath_first(iface, NULL, "ipv6/mtu")))
		pr_kv("mtu", xml_body(x));
	pr_kv("operational status", xml_find_body(iface, "oper-status"));
	pr_kv("physical address", xml_find_body(iface, "phys-address"));
	pr_detail_addrs(iface, "ipv4", "ipv4 addresses");
	pr_detail_addrs(iface, "ipv6", "ipv6 addresses");
	if ((x = xpath_first(iface, NULL, "statistics/in-octets")))
		pr_kv("in-octets", xml_body(x));
	if ((x = xpath_first(iface, NULL, "statistics/out-octets")))
		pr_kv("out-octets", xml_body(x));
}

static int is_loopback(cxobj *iface)
{
	char *type = xml_find_body(iface, "type");

	return type && strstr(type, "softwareLoopback") != NULL;
}

static long if_index(cxobj *iface)
{
	char *idx = xml_find_body(iface, "if-index");

	return idx ? atol(idx) : 0x7fffffff;	/* no index sorts last */
}

/* Datastore order is by name; restore if-index order, loopback always first. */
static int if_cmp(const void *a, const void *b)
{
	cxobj *xa = *(cxobj *const *)a, *xb = *(cxobj *const *)b;
	int la = is_loopback(xa), lb = is_loopback(xb);
	long ia, ib;

	if (la != lb)
		return lb - la;
	ia = if_index(xa);
	ib = if_index(xb);

	return (ia > ib) - (ia < ib);
}

int cli_show_interface(clixon_handle h, cvec *cvv, cvec *argv)
{
	cxobj *xret = NULL, **vec = NULL;
	cvec *nsc = NULL;
	size_t len = 0, i;
	char *name;
	cg_var *cv;
	int color, retval = -1;

	color = isatty(fileno(stdout));
	if ((nsc = xml_nsctx_init("if", IETF_IF_NS)) == NULL)
		goto done;
	if (rpc_get(h, "if:interfaces", nsc, &xret) < 0)
		goto done;
	if (xpath_vec(xret, NULL, "interfaces/interface", &vec, &len) < 0)
		goto done;

	cv = cvec_find(cvv, "name");
	name = cv ? cv_string_get(cv) : NULL;

	if (name) {
		for (i = 0; i < len; i++) {
			char *n = xml_find_body(vec[i], "name");

			if (n && !strcmp(n, name)) {
				pr_detail(vec[i]);
				break;
			}
		}
		if (i == len)
			cligen_output(stdout, "Interface \"%s\" not found\n", name);
	} else {
		if (color)
			cligen_output(stdout, "%s%-*s%-*s%-*s%-*s%s\n", C_INV,
				      W_IFACE, "INTERFACE", W_PROTO, "PROTOCOL",
				      W_STATE, "STATE", W_DATA, "DATA", C_RESET);
		else
			cligen_output(stdout, "%-*s%-*s%-*s%s\n",
				      W_IFACE, "INTERFACE", W_PROTO, "PROTOCOL",
				      W_STATE, "STATE", "DATA");
		qsort(vec, len, sizeof(vec[0]), if_cmp);
		for (i = 0; i < len; i++)
			pr_summary(vec[i], color);
	}

	retval = 0;
done:
	if (vec)
		free(vec);
	if (xret)
		xml_free(xret);
	if (nsc)
		cvec_free(nsc);

	return retval;
}

int cli_show_system(clixon_handle h, cvec *cvv, cvec *argv)
{
	cxobj *xsys = NULL, *xstate = NULL, *x;
	cvec *nsc = NULL;
	int retval = -1;

	if ((nsc = xml_nsctx_init("sys", IETF_SYS_NS)) == NULL)
		goto done;
	if (rpc_get(h, "sys:system", nsc, &xsys) < 0)
		goto done;
	if (rpc_get(h, "sys:system-state", nsc, &xstate) < 0)
		goto done;

	if ((x = xpath_first(xsys, NULL, "system/hostname")))
		pr_kv("hostname", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-name")))
		pr_kv("os name", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-release")))
		pr_kv("os release", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-version")))
		pr_kv("os version", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/machine")))
		pr_kv("machine", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/clock/current-datetime")))
		pr_kv("current time", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/clock/boot-datetime")))
		pr_kv("boot time", xml_body(x));

	retval = 0;
done:
	if (xsys)
		xml_free(xsys);
	if (xstate)
		xml_free(xstate);
	if (nsc)
		cvec_free(nsc);

	return retval;
}

int cli_show_version(clixon_handle h, cvec *cvv, cvec *argv)
{
	cxobj *xstate = NULL, *x;
	cvec *nsc = NULL;
	int retval = -1;

	if ((nsc = xml_nsctx_init("sys", IETF_SYS_NS)) == NULL)
		goto done;
	if (rpc_get(h, "sys:system-state/sys:platform", nsc, &xstate) < 0)
		goto done;

	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-name")))
		pr_kv("name", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-version")))
		pr_kv("version", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/os-release")))
		pr_kv("release", xml_body(x));
	if ((x = xpath_first(xstate, NULL, "system-state/platform/machine")))
		pr_kv("machine", xml_body(x));

	retval = 0;
done:
	if (xstate)
		xml_free(xstate);
	if (nsc)
		cvec_free(nsc);

	return retval;
}

/* Tab-completion for interface names, from the same operational data. */
int expand_ifaces(clixon_handle h, char *name, cvec *cvv, cg_var *argv,
		  cvec *commands, cvec *helptexts)
{
	cxobj *xret = NULL, **vec = NULL;
	cvec *nsc = NULL;
	size_t len = 0, i;

	if ((nsc = xml_nsctx_init("if", IETF_IF_NS)) == NULL)
		goto done;
	if (clicon_rpc_get(h, "if:interfaces", nsc, CONTENT_NONCONFIG, -1,
			   NULL, &xret) < 0)
		goto done;
	if (xpath_vec(xret, NULL, "interfaces/interface", &vec, &len) < 0)
		goto done;

	for (i = 0; i < len; i++) {
		char *n = xml_find_body(vec[i], "name");

		if (n)
			cvec_add_string(commands, NULL, n);
	}

done:
	if (vec)
		free(vec);
	if (xret)
		xml_free(xret);
	if (nsc)
		cvec_free(nsc);

	return 0;			/* best-effort: no completion on error */
}
