// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <term.h>
#include <log.h>
#include <print.h>
#include <tree.h>
#include <parse.h>
#include <build.h>

// automatically generated using gengetopt
#include "cmdline.h"

#define BUF_SIZE 1024
static char *read_stdin(void)
{
	debug("reading from stdin\n");
	char buffer[BUF_SIZE];
	size_t size = 1;
	char *string = malloc(sizeof(char) * BUF_SIZE);
	if (!string)
		fatal("out of memory!\n");
	string[0] = '\0';
	while (fgets(buffer, BUF_SIZE, stdin)) {
		char *old = string;
		size += strlen(buffer);
		string = realloc(string, size);
		if (!string) {
			free(old);
			return 0;
		}
		strcat(string, buffer);
	}

	if (ferror(stdin)) {
		free(string);
		fatal("can't read from stdin\n");
	}
	return string;
}

static char *read_file(const char *path)
{
	debug("reading from %s\n", path);
	FILE *f = fopen(path, "rb");
	if (!f)
		fatal("can't open file %s: %s\n", path, strerror(errno));

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	if (!string)
		fatal("out of memory!\n");
	int ret = fread(string, fsize, 1, f);
	fclose(f);

	if (ret != 1) {
		free(string);
		fatal("can't read file %s: %s\n", path, strerror(errno));
	}

	string[fsize] = 0;
	return string;
}

int main(int argc, char **argv)
{
	struct gengetopt_args_info args;
	if (cmdline_parser(argc, argv, &args))
		exit(1);

	debug_enable(args.verbose_flag);

	char *input;
	if (args.input_arg[0] == '-') {
		input = read_stdin();
	} else {
		input = read_file(args.input_arg);
	}

	if (!input)
		return 1;

	if (args.from_blc_flag && !args.from_bloc_flag) {
		debug("parsing as blc\n");

		struct term *parsed = parse_blc(input);
		debug("parsed blc\n");

		debug("merging duplicates\n");
		struct list *table = tree_merge_duplicates(parsed);

		write_bloc(table, args.output_arg);

		tree_destroy(table);
		free_term(parsed);
		free(input);

		debug("done!\n");
		return 0;
	}

	if (args.from_bloc_flag && !args.from_blc_flag) {
		debug("parsing as bloc\n");

		struct bloc_parsed *bloc = parse_bloc(input);
		if (args.dump_flag)
			print_bloc(bloc);
		write_blc(bloc, args.output_arg);
		free(input);
		free_bloc(bloc);
		return 0;
	}

	fatal("invalid options: use --help for information\n");
	return 1;
}
