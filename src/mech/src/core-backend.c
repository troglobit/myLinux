/* SPDX-License-Identifier: Apache-2.0 */

#include <syslog.h>

#include "common.h"

#define FACTORY_NS "urn:ietf:params:xml:ns:yang:ietf-factory-default"

int core_commit_done(clixon_handle h, transaction_data td)
{
	/*
	 * Always reload, even on the boot-time startup->running commit
	 * (which has no source): that is precisely when a service enabled
	 * from the stored config -- sntpd, say -- must be started.
	 */
	if (system("initctl -b reload"))
		CRIT("Failed to reload services");

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

static clixon_plugin_api core_api = {
	.ca_name = "core",
	.ca_init = clixon_plugin_init,

	.ca_trans_commit_done = core_commit_done,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	if (rpc_callback_register(h, core_factory_reset, NULL,
				  FACTORY_NS, "factory-reset") < 0)
		WARN("core: failed registering factory-reset");

	return &core_api;
}
