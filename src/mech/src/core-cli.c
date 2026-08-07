/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <signal.h>
#include <cligen/cligen.h>
#include <clixon/clixon.h>
#include <clixon/clixon_cli.h>
#include <clixon/cli_generate.h>

#define _PATH_LOG "/var/log/"
#define IETF_SYSTEM_NS "urn:ietf:params:xml:ns:yang:ietf-system"

/* Not exported in the common Clixon API, needed to run external commands */
void cli_signal_block(clicon_handle h);
void cli_signal_unblock(clicon_handle h);

static int exec(clicon_handle h, char *cmd)
{
	int rc;

	cli_signal_unblock(h);
	rc = system(cmd);
	cli_signal_block(h);

	return rc;
}

int cli_exec_shell(clicon_handle h, cvec *cvv, cvec *argv)
{
	return exec(h, "$SHELL -l");
}

int cli_exec_show(clicon_handle h, cvec *cvv, cvec *argv)
{
	char cmd[256] = "show -n ";
	char *arg;

	arg = cv_string_get(cvec_i(cvv, 0));
	if (!arg)
		return -1;

	strncat(cmd, &arg[5], sizeof(cmd) - strlen(cmd) - 1);
	return exec(h, cmd);
}

int cli_show_factory(clicon_handle h, cvec *cvv, cvec *argv)
{
	/* The factory config is a static seed, not a live datastore Clixon
	 * can render, so dump it in its stored form. */
	return exec(h, "cat /run/clixon/db/factory_db 2>/dev/null");
}

/*
 * Reboot/poweroff go through the backend, which runs as root, so they
 * work for any admin -- Finit only lets uid 0 issue the privileged
 * halt/reboot, and the CLI runs as the logged-in user.
 */
static int send_system_rpc(clicon_handle h, const char *op)
{
	cxobj *xtop = NULL, *xrpc, *xret = NULL, *xerr;
	int rc = -1;

	if (clixon_xml_parse_va(YB_NONE, NULL, &xtop, NULL,
				"<rpc xmlns=\"%s\" username=\"%s\" %s>"
				"<%s xmlns=\"%s\"/></rpc>",
				NETCONF_BASE_NAMESPACE, clicon_username_get(h),
				NETCONF_MESSAGE_ID_ATTR, op, IETF_SYSTEM_NS) < 0)
		goto done;

	xrpc = xml_child_i(xtop, 0);
	if (clicon_rpc_netconf_xml(h, xrpc, &xret, NULL) < 0)
		goto done;

	if ((xerr = xpath_first(xret, NULL, "//rpc-error")) != NULL) {
		clixon_netconf_error(xerr, op, NULL);
		goto done;
	}

	rc = 0;
done:
	if (xret)
		xml_free(xret);
	if (xtop)
		xml_free(xtop);

	return rc;
}

int cli_system_rpc(clicon_handle h, cvec *cvv, cvec *argv)
{
	if (cvec_len(argv) != 1) {
		clicon_err(OE_PLUGIN, EINVAL, "usage: cli_system_rpc(\"<rpc>\")");
		return -1;
	}

	return send_system_rpc(h, cv_string_get(cvec_i(argv, 0)));
}

int cli_factory_reset(clicon_handle h, cvec *cvv, cvec *argv)
{
	/* Arm the flag on the persistent store (wheel-writable); /libexec/mnt
	 * wipes /mnt/rw on the way back up.  The shell exits non-zero if the
	 * operator declines, so we only reboot on confirmation. */
	if (exec(h,
		 "printf 'Reset to factory defaults and reboot now (y/N)? '; "
		 "read ans; case \"$ans\" in [Yy]*) touch /mnt/rw/factory-reset.now ;; "
		 "*) echo 'Aborted.'; exit 1 ;; esac") != 0)
		return 0;

	return send_system_rpc(h, "system-restart");
}

int cli_exec_log(clicon_handle h, cvec *cvv, cvec *argv)
{
	const char *tool, *fn = "";
	char cmd[128];
	cg_var *cv;

	/* Reuse the shell log/follow tools.  cvv[0] is the whole command, so
	 * 'follow ...' picks follow and 'show log ...' picks log. */
	tool = strncmp("follow", cv_string_get(cvec_i(cvv, 0)), 6) ? "log" : "follow";
	if ((cv = cvec_find(cvv, "fn")))
		fn = cv_string_get(cv);

	snprintf(cmd, sizeof(cmd), "%s %s", tool, fn);
	return exec(h, cmd);
}

int logfiles(cligen_handle h, char *fn_str, cvec *cvv, cg_var *argv,
	     cvec *commands, cvec *helptexts)
{
	glob_t gl;
	size_t i;
	int rc;

	rc = glob(_PATH_LOG "*", 0, NULL, &gl);
	if (rc && rc != GLOB_NOMATCH)
		return 0;

	for (i = 0; i < gl.gl_pathc; i++) {
		char *fn;

		fn = rindex(gl.gl_pathv[i], '/');
		if (fn)
			fn++;
		else
			fn = gl.gl_pathv[i];
		cvec_add_string(commands, NULL, fn);
	}
	globfree(&gl);

	return 0;
}

static clixon_plugin_api cli_api = {
	.ca_name = "mylinux-cli",
	.ca_init = clixon_plugin_init,
};

clixon_plugin_api *clixon_plugin_init(clicon_handle h)
{
	return &cli_api;
}
