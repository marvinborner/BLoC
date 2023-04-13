// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <term.h>
#include <print.h>
#include <tree.h>
#include <parse.h>
#include <build.h>
#include <free.h>

// automatically generated using gengetopt
#include "cmdline.h"

#define BUF_SIZE 1024
static char *read_stdin(void)
{
	char buffer[BUF_SIZE];
	size_t size = 1;
	char *string = malloc(sizeof(char) * BUF_SIZE);
	if (!string)
		return 0;
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
		fprintf(stderr, "Couldn't read from stdin\n");
		return 0;
	}
	return string;
}

static char *read_file(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f) {
		fprintf(stderr, "Can't open file %s: %s\n", path,
			strerror(errno));
		return 0;
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = malloc(fsize + 1);
	int ret = fread(string, fsize, 1, f);
	fclose(f);

	if (ret != 1) {
		fprintf(stderr, "Can't read file %s: %s\n", path,
			strerror(errno));
		return 0;
	}

	string[fsize] = 0;
	return string;
}

int main(int argc, char **argv)
{
	struct gengetopt_args_info args;
	if (cmdline_parser(argc, argv, &args))
		exit(1);

	char *input;
	if (args.input_arg[0] == '-') {
		input = read_stdin();
	} else {
		input = read_file(args.input_arg);
	}

	if (!input)
		return 1;

	if (args.from_blc_flag && !args.from_bloc_flag) {
		struct term *parsed = parse_blc(input);
		struct list *table = tree_merge_duplicates(parsed);
		write_bloc(table, args.output_arg);
		tree_destroy(table);
		free_term(parsed);
		free(input);
		return 0;
	}

	if (args.from_bloc_flag) {
		struct bloc_parsed *bloc = parse_bloc(input);
		if (args.dump_flag)
			print_bloc(bloc);
		printf("%d\n", bloc->length);
		// TODO: Write file as BLC
		free(input);
		free_bloc(bloc);
	}

	return 0;
}
