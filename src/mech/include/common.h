/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MECH_COMMON_H_
#define MECH_COMMON_H_

#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <cligen/cligen.h>
#include <clixon/clixon.h>
#include <clixon/clixon_backend.h>

/*
 * Concise logging: the severity is in the name and the NULL handle and
 * priority argument are tucked away, so a call site reads WARN("...")
 * rather than clixon_log(NULL, LOG_WARNING, "...") -- and usually fits
 * on one line.
 */
#define CRIT(fmt, ...) clixon_log(NULL, LOG_CRIT,    fmt, ##__VA_ARGS__)
#define ERR(fmt, ...)  clixon_log(NULL, LOG_ERR,     fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) clixon_log(NULL, LOG_WARNING, fmt, ##__VA_ARGS__)
#define NOTE(fmt, ...) clixon_log(NULL, LOG_NOTICE,  fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) clixon_log(NULL, LOG_INFO,    fmt, ##__VA_ARGS__)
#define DBG(fmt, ...)  clixon_log(NULL, LOG_DEBUG,   fmt, ##__VA_ARGS__)

#define ADMIN_GROUP "wheel"		/* UNIX group with admin access */

#define NELEMS(v) (sizeof(v) / sizeof((v)[0]))

/*
 * User names reach system(), file paths and XPath expressions, so constrain them to
 * a safe character set (letters, digits, and _-.) before use -- no '/'
 * to escape /home, no shell metacharacters to inject commands.
 */
static inline int is_valid_username(const char *user)
{
	size_t i;

	if (!user || !user[0])
		return 0;

	if (!isalpha((unsigned char)user[0]) && user[0] != '_')
		return 0;

	for (i = 1; user[i]; i++) {
		char c = user[i];

		if (!isalnum((unsigned char)c) && c != '_' && c != '-' && c != '.')
			return 0;
	}

	return 1;
}

/* debug.c */
void show_xml_node     (char *tag, cxobj *node);
void show_xml_node_vec (char *tag, cxobj **node_vec, size_t len);

void show_transaction  (char *tag, transaction_data td, bool show_data);

#endif /* MECH_COMMON_H_ */
