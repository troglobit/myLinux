/* SPDX-License-Identifier: Apache-2.0 */

#include <syslog.h>
#include <signal.h>
#include <cligen/cligen.h>
#include <clixon/clixon.h>
#include <clixon/clixon_backend.h>

int core_commit_done(clixon_handle h, transaction_data td)
{
	if (transaction_src(td) && system("initctl -b reload"))
		clixon_log(NULL, LOG_CRIT, "Failed to reload services");

	return 0;
}

static clixon_plugin_api core_api = {
	.ca_name = "core",
	.ca_init = clixon_plugin_init,

	.ca_trans_commit_done = core_commit_done,
};

clixon_plugin_api *clixon_plugin_init(clixon_handle h)
{
	return &core_api;
}
