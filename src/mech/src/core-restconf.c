/* SPDX-License-Identifier: Apache-2.0 */

#include <crypt.h>
#include <grp.h>
#include <pwd.h>
#include <syslog.h>

#include "common.h"
#include <clixon/clixon_restconf.h>

#define IETF_SYS_NS "urn:ietf:params:xml:ns:yang:ietf-system"

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
	char *auth, *user, *pass;

	if (type != CLIXON_AUTH_USER)
		return 0;

	*authp = NULL;

	auth = restconf_param_get(h, "HTTP_AUTHORIZATION");
	if (!auth || strncasecmp(auth, "Basic ", 6))
		return 1;
	user = strdup(auth + 6);
	if (!user)
		return -1;
	b64_decode(user);

	pass = strchr(user, ':');
	if (pass) {
		*pass++ = 0;
		if (is_valid_username(user) && check_password(h, user, pass) && is_admin(user)) {
			*authp = user;
			return 1;
		}
		/* What we were sent goes in the log only if it is tame */
		NOTE("restconf: login failed for user %s", is_valid_username(user) ? user : "(invalid)");
	}
	free(user);

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
