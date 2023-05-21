// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <term.h>
#include <optimize.h>
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

static char *read_file(FILE *f)
{
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	if (!string)
		fatal("out of memory!\n");
	int ret = fread(string, fsize, 1, f);

	if (ret != 1) {
		free(string);
		fatal("can't read file: %s\n", strerror(errno));
	}

	string[fsize] = 0;
	return string;
}

static char *read_path(const char *path)
{
	debug("reading from %s\n", path);
	FILE *f = fopen(path, "rb");
	if (!f)
		fatal("can't open file %s: %s\n", path, strerror(errno));
	char *string = read_file(f);
	fclose(f);
	return string;
}

static void test(char *input)
{
	debug("parsing as blc\n");

	struct term *parsed_1 = parse_blc(input);
	free(input);
	debug("parsed original blc\n");

	debug("merging duplicates\n");
	void *all_trees = 0;
	struct tree *tree = tree_merge_duplicates(parsed_1, &all_trees);

	debug("optimizing tree\n");
	struct list *table = optimize_tree(tree, &all_trees);

	FILE *temp_bloc = tmpfile();
	write_bloc(table, temp_bloc);
	tree_destroy(table);

	debug("parsing as bloc\n");
	char *temp = read_file(temp_bloc);
	struct bloc_parsed *bloc = parse_bloc(temp);
	fseek(temp_bloc, 0, SEEK_END);
	fprintf(stderr, "size bloc: %lu\n", ftell(temp_bloc));
	fclose(temp_bloc);

	FILE *temp_blc = tmpfile();
	write_blc(bloc, temp_blc);
	char *input_2 = read_file(temp_blc);
	struct term *parsed_2 = parse_blc(input_2);
	fseek(temp_blc, 0, SEEK_END);
	fprintf(stderr, "size blc: ~%lu\n", ftell(temp_blc) / 8);
	fclose(temp_blc);
	free(input_2);
	debug("parsed reconstructed blc\n");

	diff_term(parsed_1, parsed_2);
	debug("diffed two terms\n");

	free_term(parsed_1);
	free_term(parsed_2);
	debug("done!\n");
}

static void from_blc(char *input, char *output_path)
{
	debug("parsing as blc\n");

	struct term *parsed = parse_blc(input);
	debug("parsed blc\n");

	debug("merging duplicates\n");
	void *all_trees = 0;
	struct tree *tree = tree_merge_duplicates(parsed, &all_trees);

	debug("optimizing tree\n");
	struct list *table = optimize_tree(tree, &all_trees);

	FILE *file = fopen(output_path, "wb");
	write_bloc(table, file);
	fclose(file);

	tree_destroy(table);
	free_term(parsed);
	free(input);

	debug("done!\n");
}

static void from_bloc(char *input, char *output_path, int dump)
{
	debug("parsing as bloc\n");

	struct bloc_parsed *bloc = parse_bloc(input);
	if (dump)
		print_bloc(bloc);

	FILE *file = fopen(output_path, "wb");
	write_blc(bloc, file);
	fclose(file);

	free(input);
	free_bloc(bloc);
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
		input = read_path(args.input_arg);
	}

	if (!input)
		return 1;

	if (args.test_flag && args.from_blc_flag && !args.from_bloc_flag) {
		test(input);
		return 0;
	}

	if (args.from_blc_flag && !args.from_bloc_flag) {
		from_blc(input, args.output_arg);
		return 0;
	}

	if (args.from_bloc_flag && !args.from_blc_flag) {
		from_bloc(input, args.output_arg, args.dump_flag);
		return 0;
	}

	fatal("invalid options: use --help for information\n");
	return 1;
}
