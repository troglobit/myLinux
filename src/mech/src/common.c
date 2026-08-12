/* SPDX-License-Identifier: Apache-2.0 */

#include <syslog.h>
#include "common.h"

#define MAX_INDENT 128


static char* indent_str(size_t indent)
{
	static char str[MAX_INDENT + 1];

	for (size_t i = 0; i < indent; i++)
		str[i] = ' ';
	str[indent] = '\0';

	return str;
}

static void show_indented_xml_node(char *tag, cxobj *node, size_t indent)
{
	cxobj *child = NULL;

	if (!xml_body_get(node))
		DBG("%s: %s%s", tag, indent_str(indent), xml_name(node));
	else
		DBG("%s: %s%s = %s", tag, indent_str(indent), xml_name(node), xml_body(node));

	while ((child = xml_child_each(node, child, CX_ELMNT)))
		show_indented_xml_node(tag, child, indent + 2);
}

void show_xml_node(char *tag, cxobj *node)
{
	show_indented_xml_node(tag, node, 4);
}

void show_xml_node_vec(char *tag, cxobj **node_vec, size_t len)
{
	for (size_t i = 0; i < len; i++)
		show_indented_xml_node(tag, node_vec[i], 4);
}

void show_transaction(char *tag, transaction_data td, bool show_data)
{
	DBG("%s:   transaction ID = %lu", tag, transaction_id(td));
	if (show_data) {
		DBG("%s:   transaction source:", tag);
		show_xml_node(tag, transaction_src(td));
		DBG("%s:   transaction target:", tag);
		show_xml_node(tag, transaction_target(td));
		DBG("%s:   transaction deleted:", tag);
		show_xml_node_vec(tag, transaction_dvec(td), transaction_dlen(td));
		DBG("%s:   transaction added:", tag);
		show_xml_node_vec(tag, transaction_avec(td), transaction_alen(td));
		DBG("%s:   transaction changed source:", tag);
		show_xml_node_vec(tag, transaction_scvec(td), transaction_clen(td));
		DBG("%s:   transaction changed target:", tag);
		show_xml_node_vec(tag, transaction_tcvec(td), transaction_clen(td));
	}
}
