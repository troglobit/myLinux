/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MECH_COMMON_H_
#define MECH_COMMON_H_

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

/* debug.c */
void show_xml_node     (char *tag, cxobj *node);
void show_xml_node_vec (char *tag, cxobj **node_vec, size_t len);

void show_transaction  (char *tag, transaction_data td, bool show_data);

#endif /* MECH_COMMON_H_ */
