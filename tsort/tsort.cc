
/*
 * Copyright 2008-2009 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef HAVE_CONFIG_H
#error missing autoconf-generated config.h.
#endif
#include "posixutils-config.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <libpu.h>

struct order {
	char		*pred;
	char		*succ;
};

static char linebuf[LINE_MAX + 1];
static char *extra_token;

static char **dict, **out_vals;
static unsigned int dict_size, dict_alloc;

struct order *order;
static unsigned int order_size, order_alloc;

static const char doc[] =
N_("tsort - topological sort");

static const char args_doc[] = N_("file");

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static const struct argp argp = { NULL, parse_opt, args_doc, doc };
static char *opt_filename = NULL;

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case ARGP_KEY_ARG:
		if (state->arg_num == 0)
			opt_filename = arg;
		else
			argp_usage(state);
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static char *add_token(const char *buf, size_t buflen, bool ok_add)
{
	unsigned int i;

	/* linear search for token in dictionary */
	/* TODO: use a hash table or tree */
	for (i = 0; i < dict_size; i++) {
		size_t slen;
		
		slen = strlen(dict[i]);
		if (slen == buflen && !memcmp(buf, dict[i], slen))
			return dict[i];
	}

	if (!ok_add)
		return NULL;

	/* not found, add token to dictionary, create new id */

	if (dict_size == dict_alloc) {
		dict_alloc *= 2;
		dict = (char **) realloc(dict, sizeof(char *) * dict_alloc);
		if (!dict)
			return NULL;
	}

	dict[dict_size] = strndup(buf, buflen);
	if (!dict[dict_size])
		return NULL;

	return dict[dict_size++];
}

static int push_pair(char *a, char *b)
{
	if (order_size == order_alloc) {
		order_alloc *= 2;
		order = (struct order *) xrealloc(order, sizeof(struct order) * order_alloc);
	}

	order[order_size].pred = a;
	order[order_size].succ = b;
	order_size++;

	return 0;
}

static int push_token(const char *_s, size_t slen)
{
	char *id = add_token(_s, slen, true);
	if (!id)
		return 1;

	if (!extra_token)
		extra_token = id;
	else {
		if (push_pair(extra_token, id))
			return 1;
		extra_token = NULL;
	}

	return 0;
}

static int process_line(void)
{
	size_t mlen;
	char *p = linebuf, *end;

	while (*p) {
		while (isspace(*p))
			p++;

		end = p;
		while (*end && !isspace(*end))
			end++;

		mlen = end - p;

		if (mlen > 0) {
			if (push_token(p, mlen))
				return 1;
			p = end;
		}
	}

	return 0;
}

static int max_recurse;

static bool item_precedes(const char *a, const char *b)
{
	unsigned int i;

	max_recurse--;			/* crude hack! */
	if (max_recurse <= 0) {
		fprintf(stderr, _("tsort: max recursion limit reached\n"));
		exit(1);
	}

	for (i = 0; i < order_size; i++) {
		struct order *o;

		o = &order[i];
		if (o->pred != a)
			continue;
		if (o->succ == b)
			return true;
		if ((o->succ != a) && item_precedes(o->succ, b))
			return true;
	}

	return false;
}

static int compare_items(const void *_a, const void *_b)
{
	const char *a = *(const char **)_a;
	const char *b = *(const char **)_b;

	if (a == b)
		return 0;
	
	max_recurse = 4000000;		/* crude hack! */
	if (item_precedes(a, b))
		return -1;
	
	return 1;
}

static int do_sort(void)
{
	out_vals = (char **) calloc(dict_size, sizeof(char *));
	if (!out_vals)
		return 1;

	memcpy(out_vals, dict, sizeof(char *) * dict_size);

	qsort(out_vals, dict_size, sizeof(char *), compare_items);
	
	return 0;
}

static int init_arrays(void)
{
	dict = (char **) xcalloc(512, sizeof(char *));
	dict_alloc = 512;

	order = (struct order *) xcalloc(512, sizeof(struct order));
	order_alloc = 512;

	return 0;
}

static void write_vals(void)
{
	unsigned int i;

	for (i = 0; i < dict_size; i++)
		printf("%s\n", out_vals[i]);
}

int main (int argc, char *argv[])
{
	FILE *f = NULL;

	pu_init();

	if (init_arrays())
		return 1;

	error_t argp_rc = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (argp_rc) {
		fprintf(stderr, _("%s: argp_parse failed: %s\n"),
			argv[0], strerror(argp_rc));
		return EXIT_FAILURE;
	}

	if (opt_filename == NULL)
		f = stdin;
	else {
		if (ro_file_open(&f, opt_filename)) {
			perror(opt_filename);
			return EXIT_FAILURE;
		}
	}

	while (fgets_unlocked(linebuf, sizeof(linebuf), f)) {
		if (process_line())
			return EXIT_FAILURE;
	}

	if (f != stdin)
		fclose(f);

	if (do_sort())
		return EXIT_FAILURE;

	write_vals();

	return EXIT_SUCCESS;
}

