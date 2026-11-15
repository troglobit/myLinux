/* SPDX-License-Identifier: Apache-2.0 */

#include <arpa/inet.h>
#include <crypt.h>
#include <grp.h>
#include <pwd.h>
#include <syslog.h>
#include <time.h>

#include "common.h"
#include <clixon/clixon_restconf.h>

#define IETF_SYS_NS "urn:ietf:params:xml:ns:yang:ietf-system"

#define FREE_TRIES  5		/* Failed logins before a client has to wait */
#define PENALTY     30		/* Seconds to wait for each try after those */

/* libclixon_restconf has it, clixon_restconf.h does not */
int restconf_reply_header(void *req, const char *name, const char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));

/* Clients with failed logins behind them, by address */
static struct client {
	char   addr[INET6_ADDRSTRLEN];
	int    fails;
	time_t last;		/* Time of the latest failure */
} clients[32];
static time_t flood;		/* More clients failing than we can hold, until then */

static time_t uptime(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec;
}

/*
 * Who is asking, as Merecat saw it.  An IPv6 client has a whole /64 to
 * pick addresses from, so there the prefix is what we go by.
 */
static void client_addr(clixon_handle h, char *buf, size_t len)
{
	char *addr = restconf_param_get(h, "HTTP_X_REAL_IP");
	struct in6_addr in6;
	struct in_addr in;

	if (addr && inet_pton(AF_INET6, addr, &in6) == 1) {
		memset(&in6.s6_addr[8], 0, 8);
		inet_ntop(AF_INET6, &in6, buf, len);
		return;
	}

	/* Not through Merecat, then: someone on the device itself */
	if (!addr || inet_pton(AF_INET, addr, &in) != 1)
		addr = "localhost";
	snprintf(buf, len, "%s", addr);
}

static struct client *client_find(const char *addr)
{
	struct client *cl;

	for (cl = clients; cl < &clients[NELEMS(clients)]; cl++) {
		if (cl->fails && !strcmp(cl->addr, addr))
			return cl;
	}

	return NULL;
}

/*
 * A new client pushes out whoever failed longest ago.  If that was just
 * now, somebody is guessing from more addresses than we keep track of,
 * and forgetting them one by one gives each its free tries back.  Then
 * nobody gets a try for a while.
 */
static struct client *client_new(const char *addr, time_t now)
{
	struct client *cl, *old = &clients[0];

	for (cl = clients; cl < &clients[NELEMS(clients)]; cl++) {
		if (cl->last < old->last)
			old = cl;
	}
	if (old->fails && now - old->last < PENALTY)
		flood = now + PENALTY;
	snprintf(old->addr, sizeof(old->addr), "%s", addr);
	old->fails = 0;

	return old;
}

/* Decode base64 in place, to a string.  Stops at padding or anything odd. */
static void b64_decode(char *str)
{
	static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				  "abcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned int acc = 0;
	char *out = str, *p;
	int bits = 0;

	for (; *str && (p = strchr(tbl, *str)); str++) {
		acc = (acc << 6) | (unsigned int)(p - tbl);
		bits += 6;
		if (bits >= 8) {
			bits -= 8;
			*out++ = (char)(acc >> bits);
		}
	}
	*out = 0;
}

static int is_admin(const char *user)
{
	struct passwd *pw = getpwnam(user);
	struct group *gr = getgrnam(ADMIN_GROUP);
	char **mem;

	if (!pw || !gr)
		return 0;
	if (pw->pw_gid == gr->gr_gid)
		return 1;

	for (mem = gr->gr_mem; *mem; mem++) {
		if (!strcmp(*mem, user))
			return 1;
	}

	return 0;
}

/*
 * We run as www-data and cannot read /etc/shadow.  What the backend puts
 * there is the user's password leaf as it is, so ask for that.
 */
static char *password_hash(clixon_handle h, const char *user)
{
	cxobj *xt = NULL, *xp;
	cvec *nsc = NULL;
	char *hash = NULL;
	cbuf *cb;

	cb = cbuf_new();
	if (!cb)
		return NULL;
	cprintf(cb, "/sys:system/sys:authentication/sys:user[sys:name='%s']/sys:password", user);

	nsc = xml_nsctx_init("sys", IETF_SYS_NS);
	if (!nsc)
		goto done;
	if (clicon_rpc_get(h, cbuf_get(cb), nsc, CONTENT_CONFIG, -1, NULL, &xt) < 0)
		goto done;

	xp = xpath_first(xt, nsc, "%s", cbuf_get(cb));
	if (xp && xml_body(xp))
		hash = strdup(xml_body(xp));
done:
	if (xt)
		xml_free(xt);
	if (nsc)
		xml_nsctx_free(nsc);
	cbuf_free(cb);

	return hash;
}

static int check_password(clixon_handle h, const char *user, const char *pass)
{
	char *hash, *res;
	int ok = 0;

	hash = password_hash(h, user);
	if (!hash)
		return 0;

	/* "*", "!" and the like: no password login for this user */
	if (hash[0] == '$') {
		res = crypt(pass, hash);
		ok = res && !strcmp(res, hash);
	}
	free(hash);

	return ok;
}

/*
 * HTTP Basic authentication against the local users of ietf-system.
 * Until NACM says who may do what, only an administrator gets in: with
 * NACM disabled every authenticated user has the run of the datastore.
 *
 * Returns 1 with the user in authp, or NULL there for a 401.
 */
static int core_auth(clixon_handle h, void *req, clixon_auth_type_t type, char **authp)
{
	char addr[INET6_ADDRSTRLEN];
	char *auth, *user, *pass;
	time_t now = uptime();
	struct client *cl;
	time_t wait = 0;

	if (type != CLIXON_AUTH_USER)
		return 0;

	*authp = NULL;

	auth = restconf_param_get(h, "HTTP_AUTHORIZATION");
	if (!auth || strncasecmp(auth, "Basic ", 6))
		return 1;

	/* Too many failed logins: turned away unchecked, right password or not */
	client_addr(h, addr, sizeof(addr));
	cl = client_find(addr);
	if (cl && cl->fails >= FREE_TRIES && now - cl->last < PENALTY)
		wait = PENALTY - (now - cl->last);
	else if (now < flood)
		wait = flood - now;
	if (wait) {
		restconf_reply_header(req, "Retry-After", "%ld", (long)wait);
		return 1;
	}

	user = strdup(auth + 6);
	if (!user)
		return -1;
	b64_decode(user);

	pass = strchr(user, ':');
	if (pass) {
		*pass++ = 0;
		if (is_valid_username(user) && check_password(h, user, pass) && is_admin(user)) {
			if (cl)
				cl->fails = 0;
			*authp = user;
			return 1;
		}
		/* What we were sent goes in the log only if it is tame */
		NOTE("restconf: login failed for user %s from %s",
		     is_valid_username(user) ? user : "(invalid)", addr);
	}
	free(user);

	if (!cl)
		cl = client_new(addr, now);
	cl->fails++;
	cl->last = now;

	return 1;
}

static clixon_plugin_api core_api = {
	.ca_name = "core",
	.ca_init = clixon_plugin_init,
	.ca_auth = core_auth,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	return &core_api;
}
