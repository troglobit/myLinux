/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Print the configuration part of a YANG model as JSON, for the web
 * interface to build its forms from.  Runs on the build host.
 *
 *     yang2json [-F MODULE:FEATURE]... [-p DIR]... MODULE
 *
 * MODULE and what it imports are loaded the way clixon does it, all of
 * them implemented, with the features given.  The output is
 *
 *     { "identities": { "mod:base": ["mod:derived", ...], ... },
 *       "data":       { "mod:node": NODE, ... } }
 *
 * where NODE has "kind" and, as they apply, "type", "keys", "children"
 * and the rest of what print_node() writes.  Names are as in RESTCONF:
 * with the module in front where it differs from that of the parent.
 * Of a type there is what a form has use for, so no patterns and not
 * the members of a union.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libyang/libyang.h>

#define MAXFEAT 64

static const char *features[MAXFEAT];	/* "module:feature" */
static int         nfeatures;

static void print_str(const char *str)
{
	putchar('"');
	for (; *str; str++) {
		switch (*str) {
		case '"':  fputs("\\\"", stdout); break;
		case '\\': fputs("\\\\", stdout); break;
		case '\n': fputs("\\n", stdout);  break;
		case '\t': fputs("\\t", stdout);  break;
		default:
			if ((unsigned char)*str < 0x20)
				printf("\\u%04x", *str);
			else
				putchar(*str);
		}
	}
	putchar('"');
}

/* "key": with a comma ahead of it for all but the first of an object */
static void print_key(int *num, const char *key)
{
	if ((*num)++)
		putchar(',');
	print_str(key);
	putchar(':');
}

static void print_member(int *num, const char *key, const char *val)
{
	print_key(num, key);
	print_str(val);
}

static void print_qname(const struct lys_module *mod, const char *name)
{
	printf("\"%s:%s\"", mod->name, name);
}

static void print_range(int *num, const char *key, const struct lysc_range *range, int is_signed)
{
	LY_ARRAY_COUNT_TYPE i;

	if (!range)
		return;

	print_key(num, key);
	putchar('[');
	LY_ARRAY_FOR(range->parts, i) {
		if (i)
			putchar(',');
		if (is_signed)
			printf("[%lld,%lld]", (long long)range->parts[i].min_64,
			       (long long)range->parts[i].max_64);
		else
			printf("[%llu,%llu]", (unsigned long long)range->parts[i].min_u64,
			       (unsigned long long)range->parts[i].max_u64);
	}
	putchar(']');
}

/* An XPath with the prefixes it was written with replaced by module names */
static void print_path(const char *path, const struct lysc_prefix *prefixes)
{
	LY_ARRAY_COUNT_TYPE i;
	const char *end;

	putchar('"');
	while (*path) {
		end = path + strcspn(path, ":/[]()=,'\" ");
		if (*end == ':') {
			LY_ARRAY_FOR(prefixes, i) {
				if (prefixes[i].prefix &&
				    strlen(prefixes[i].prefix) == (size_t)(end - path) &&
				    !strncmp(prefixes[i].prefix, path, end - path))
					break;
			}
			if (i < LY_ARRAY_COUNT(prefixes))
				fputs(prefixes[i].mod->name, stdout);
			else
				fwrite(path, 1, end - path, stdout);
		} else {
			fwrite(path, 1, end - path, stdout);
		}
		if (*end) {
			if (*end == '"')
				putchar('\\');
			putchar(*end++);
		}
		path = end;
	}
	putchar('"');
}

static const char *base(const struct lysc_type *type)
{
	switch (type->basetype) {
	case LY_TYPE_BINARY:  return "binary";
	case LY_TYPE_UINT8:   return "uint8";
	case LY_TYPE_UINT16:  return "uint16";
	case LY_TYPE_UINT32:  return "uint32";
	case LY_TYPE_UINT64:  return "uint64";
	case LY_TYPE_STRING:  return "string";
	case LY_TYPE_BITS:    return "bits";
	case LY_TYPE_BOOL:    return "boolean";
	case LY_TYPE_DEC64:   return "decimal64";
	case LY_TYPE_EMPTY:   return "empty";
	case LY_TYPE_ENUM:    return "enumeration";
	case LY_TYPE_IDENT:   return "identityref";
	case LY_TYPE_INST:    return "instance-identifier";
	case LY_TYPE_LEAFREF: return "leafref";
	case LY_TYPE_UNION:   return "union";
	case LY_TYPE_INT8:    return "int8";
	case LY_TYPE_INT16:   return "int16";
	case LY_TYPE_INT32:   return "int32";
	case LY_TYPE_INT64:   return "int64";
	default:              return "unknown";
	}
}

/*
 * The typedef is the name the model gives the type, less its prefix.
 * It tells an IP address or a password hash from any other string.
 */
static void print_type(const struct lysc_type *type, const char *typedef_name)
{
	LY_ARRAY_COUNT_TYPE i;
	const char *name;
	int num = 0;

	putchar('{');
	print_member(&num, "base", base(type));
	if (typedef_name) {
		name = strchr(typedef_name, ':');
		name = name ? name + 1 : typedef_name;
		if (strcmp(name, base(type)))
			print_member(&num, "typedef", name);
	}

	switch (type->basetype) {
	case LY_TYPE_INT8:
	case LY_TYPE_INT16:
	case LY_TYPE_INT32:
	case LY_TYPE_INT64:
		print_range(&num, "range", ((const struct lysc_type_num *)type)->range, 1);
		break;

	case LY_TYPE_UINT8:
	case LY_TYPE_UINT16:
	case LY_TYPE_UINT32:
	case LY_TYPE_UINT64:
		print_range(&num, "range", ((const struct lysc_type_num *)type)->range, 0);
		break;

	case LY_TYPE_STRING:
		print_range(&num, "length", ((const struct lysc_type_str *)type)->length, 0);
		break;

	case LY_TYPE_ENUM: {
		const struct lysc_type_enum *en = (const struct lysc_type_enum *)type;

		print_key(&num, "enum");
		putchar('[');
		LY_ARRAY_FOR(en->enums, i) {
			if (i)
				putchar(',');
			print_str(en->enums[i].name);
		}
		putchar(']');
		break;
	}

	case LY_TYPE_IDENT: {
		const struct lysc_type_identityref *id = (const struct lysc_type_identityref *)type;

		print_key(&num, "bases");
		putchar('[');
		LY_ARRAY_FOR(id->bases, i) {
			if (i)
				putchar(',');
			print_qname(id->bases[i]->module, id->bases[i]->name);
		}
		putchar(']');
		break;
	}

	case LY_TYPE_LEAFREF: {
		const struct lysc_type_leafref *ref = (const struct lysc_type_leafref *)type;

		print_key(&num, "path");
		print_path(lyxp_get_expr(ref->path), ref->prefixes);
		print_key(&num, "type");
		print_type(ref->realtype, NULL);
		break;
	}

	default:
		break;
	}
	putchar('}');
}

static const char *kind(const struct lysc_node *node)
{
	switch (node->nodetype) {
	case LYS_CONTAINER: return "container";
	case LYS_LIST:      return "list";
	case LYS_LEAF:      return "leaf";
	case LYS_LEAFLIST:  return "leaf-list";
	case LYS_CHOICE:    return "choice";
	case LYS_CASE:      return "case";
	default:            return NULL;
	}
}

static void print_siblings(int *num, const struct lysc_node *first, const struct lys_module *parent);

static void print_node(const struct lysc_node *node)
{
	const struct lysc_node *child;
	struct lysc_when **when;
	LY_ARRAY_COUNT_TYPE i;
	int num = 0, keys = 0;

	putchar('{');
	print_member(&num, "kind", kind(node));
	if (node->dsc)
		print_member(&num, "description", node->dsc);
	if (node->flags & LYS_MAND_TRUE) {
		print_key(&num, "mandatory");
		fputs("true", stdout);
	}

	when = lysc_node_when(node);
	if (when) {
		print_key(&num, "when");
		putchar('[');
		LY_ARRAY_FOR(when, i) {
			if (i)
				putchar(',');
			print_path(lyxp_get_expr(when[i]->cond), when[i]->prefixes);
		}
		putchar(']');
	}

	switch (node->nodetype) {
	case LYS_CONTAINER:
		if (node->flags & LYS_PRESENCE) {
			print_key(&num, "presence");
			fputs("true", stdout);
		}
		break;

	case LYS_LIST:
		print_key(&num, "keys");
		putchar('[');
		for (child = lysc_node_child(node); child; child = child->next) {
			if (!(child->flags & LYS_KEY))
				continue;
			if (keys++)
				putchar(',');
			print_str(child->name);
		}
		putchar(']');
		break;

	case LYS_LEAF: {
		const struct lysc_node_leaf *leaf = (const struct lysc_node_leaf *)node;

		print_key(&num, "type");
		print_type(leaf->type, ((const struct lysp_node_leaf *)node->priv)->type.name);
		if (leaf->units)
			print_member(&num, "units", leaf->units);
		if (leaf->dflt)
			print_member(&num, "default", lyd_value_get_canonical(node->module->ctx, leaf->dflt));
		break;
	}

	case LYS_LEAFLIST: {
		const struct lysc_node_leaflist *list = (const struct lysc_node_leaflist *)node;

		print_key(&num, "type");
		print_type(list->type, ((const struct lysp_node_leaflist *)node->priv)->type.name);
		if (list->units)
			print_member(&num, "units", list->units);
		break;
	}

	default:
		break;
	}

	if (lysc_node_child(node)) {
		int kids = 0;

		print_key(&num, "children");
		putchar('{');
		print_siblings(&kids, lysc_node_child(node), node->module);
		putchar('}');
	}
	putchar('}');
}

/* Siblings that are configuration, as members of the object we are in */
static void print_siblings(int *num, const struct lysc_node *first, const struct lys_module *parent)
{
	const struct lysc_node *node;

	for (node = first; node; node = node->next) {
		if (!kind(node) || !(node->flags & LYS_CONFIG_W))
			continue;

		if ((*num)++)
			putchar(',');
		if (node->module != parent)
			print_qname(node->module, node->name);
		else
			print_str(node->name);
		putchar(':');
		print_node(node);
	}
}

/* The features asked for that are of this module, NULL terminated */
static const char **features_of(const struct lys_module *mod)
{
	static const char *list[MAXFEAT + 1];
	size_t len = strlen(mod->name);
	int i, num = 0;

	for (i = 0; i < nfeatures; i++) {
		if (!strncmp(features[i], mod->name, len) && features[i][len] == ':')
			list[num++] = &features[i][len + 1];
	}
	list[num] = NULL;

	return list;
}

static int usage(void)
{
	fputs("usage: yang2json [-F MODULE:FEATURE]... [-p DIR]... MODULE\n", stderr);

	return 1;
}

int main(int argc, char *argv[])
{
	const struct lys_module *mod;
	struct lys_module *main_mod;
	struct ly_ctx *ctx;
	LY_ARRAY_COUNT_TYPE i, j;
	uint32_t idx = 0;
	int c, num = 0;

	/* The parsed nodes are kept for the names of the types */
	if (ly_ctx_new(NULL, LY_CTX_SET_PRIV_PARSED, &ctx))
		return 1;

	while ((c = getopt(argc, argv, "F:p:")) != -1) {
		switch (c) {
		case 'F':
			if (nfeatures == MAXFEAT)
				return usage();
			features[nfeatures++] = optarg;
			break;
		case 'p':
			ly_ctx_set_searchdir(ctx, optarg);
			break;
		default:
			return usage();
		}
	}
	if (optind + 1 != argc)
		return usage();

	/* No features to give the main module before we have its name */
	main_mod = ly_ctx_load_module(ctx, argv[optind], NULL, NULL);
	if (!main_mod)
		return 1;
	LY_ARRAY_FOR(main_mod->parsed->imports, i) {
		struct lys_module *imp = main_mod->parsed->imports[i].module;

		if (lys_set_implemented(imp, features_of(imp)))
			return 1;
	}

	fputs("{\"identities\":{", stdout);
	while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
		LY_ARRAY_FOR(mod->identities, i) {
			const struct lysc_ident *id = &mod->identities[i];

			if (!id->derived)
				continue;

			if (num++)
				putchar(',');
			print_qname(id->module, id->name);
			fputs(":[", stdout);
			LY_ARRAY_FOR(id->derived, j) {
				if (j)
					putchar(',');
				print_qname(id->derived[j]->module, id->derived[j]->name);
			}
			putchar(']');
		}
	}

	fputs("},\"data\":{", stdout);
	num = idx = 0;
	while ((mod = ly_ctx_get_module_iter(ctx, &idx))) {
		if (mod->implemented && mod->compiled)
			print_siblings(&num, mod->compiled->data, NULL);
	}
	fputs("}}\n", stdout);

	ly_ctx_destroy(ctx);

	return 0;
}
