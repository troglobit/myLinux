/* SPDX-License-Identifier: Apache-2.0 */

#include <syslog.h>

#include "common.h"

#define FACTORY_NS "urn:ietf:params:xml:ns:yang:ietf-factory-default"
#define NCN_NS     "urn:ietf:params:xml:ns:yang:ietf-netconf-notifications"

int core_commit_done(clixon_handle h, transaction_data td)
{
	/*
	 * Always reload, even on the boot-time startup->running commit
	 * (which has no source): that is precisely when a service enabled
	 * from the stored config -- sntpd, say -- must be started.
	 */
	if (system("initctl -b reload"))
		CRIT("Failed to reload services");

	/*
	 * RFC 6470.  Tells a subscriber, the web UI for one, that what it
	 * shows is out of date.  The model wants changed-by as well, user
	 * and session, and the session is not something we are told here.
	 */
	if (stream_notify(h, "NETCONF", "<netconf-config-change xmlns=\"%s\">"
			  "<datastore>running</datastore>"
			  "</netconf-config-change>", NCN_NS) < 0)
		WARN("core: failed sending netconf-config-change");

	return 0;
}

/*
 * factory-reset (RFC 8808): reapply the factory datastore as the new
 * running config.  candidate_commit() runs the full transaction so every
 * backend's apply logic fires -- users, interfaces and the rest are
 * brought back to the factory state.  prep-db lays down the factory
 * datastore ($CLICON_XMLDB_DIR/factory_db) at boot.
 */
static int core_factory_reset(clixon_handle h, cxobj *xe, cbuf *cbret,
			      void *arg, void *regarg)
{
	int ret;

	if (xmldb_exists(h, "factory") != 1) {
		WARN("factory-reset: no factory datastore");
		return netconf_operation_failed(cbret, "application",
						"no factory configuration");
	}

	ret = candidate_commit(h, NULL, "factory", 0, 0, cbret);
	if (ret < 0)
		return -1;
	if (ret == 0)		/* validation failed; cbret holds the error */
		return 0;

	cprintf(cbret, "<rpc-reply xmlns=\"%s\"><ok/></rpc-reply>",
		NETCONF_BASE_NAMESPACE);

	return 0;
}

/* Is the node something that was set, and not just defaults that clixon filled in? */
static int is_set(cxobj *x)
{
	cxobj *child = NULL;
	int leaf = 1;

	if (xml_flag(x, XML_FLAG_DEFAULT))
		return 0;

	while ((child = xml_child_each(x, child, CX_ELMNT))) {
		if (is_set(child))
			return 1;
		leaf = 0;
	}

	return leaf;
}

static int unsupported(cxobj *x, void *arg)
{
	yang_stmt *ys = xml_spec(x);

	if (ys && yang_flag_get(ys, YANG_FLAG_NOT_SUPPORT) && is_set(x)) {
		clixon_err(OE_CFG, 0, "%s is not supported", xml_name(x));
		return -1;
	}

	return 0;
}

/*
 * To clixon "deviate not-supported" means that the node's must
 * statements go unchecked.  The node itself it takes as before, so
 * turning it down is left to us.
 */
static int core_validate(clixon_handle h, transaction_data td)
{
	return xml_apply(transaction_target(td), CX_ELMNT, unsupported, NULL) < 0 ? -1 : 0;
}

static clixon_plugin_api core_api = {
	.ca_name = "core",
	.ca_init = clixon_plugin_init,

	.ca_trans_validate = core_validate,
	.ca_trans_commit_done = core_commit_done,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	if (rpc_callback_register(h, core_factory_reset, NULL,
				  FACTORY_NS, "factory-reset") < 0)
		WARN("core: failed registering factory-reset");

	/*
	 * The default event stream of RFC 5277.  Clixon does not create
	 * it, and without it create-subscription and RESTCONF
	 * /streams/NETCONF have nothing to attach to.
	 */
	if (stream_add(h, "NETCONF", "default NETCONF event stream", 0, NULL) < 0)
		WARN("core: failed registering NETCONF event stream");

	return &core_api;
}
