// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <print.h>
#include <log.h>

void print_bruijn(struct term *term)
{
	switch (term->type) {
	case ABS:
		fprintf(stderr, "[");
		print_bruijn(term->u.abs.term);
		fprintf(stderr, "]");
		break;
	case APP:
		fprintf(stderr, "(");
		print_bruijn(term->u.app.lhs);
		fprintf(stderr, " ");
		print_bruijn(term->u.app.rhs);
		fprintf(stderr, ")");
		break;
	case VAR:
		fprintf(stderr, "%d", term->u.var.index);
		break;
	case REF:
		fprintf(stderr, "<%ld>", term->u.ref.index);
		break;
	default:
		fatal("invalid type %d\n", term->type);
	}
}

void print_blc(struct term *term)
{
	switch (term->type) {
	case ABS:
		printf("00");
		print_blc(term->u.abs.term);
		break;
	case APP:
		printf("01");
		print_blc(term->u.app.lhs);
		print_blc(term->u.app.rhs);
		break;
	case VAR:
		for (int i = 0; i <= term->u.var.index; i++)
			printf("1");
		printf("0");
		break;
	default:
		fatal("invalid type %d\n", term->type);
	}
}

void print_bloc(struct bloc_parsed *bloc)
{
	fprintf(stderr, "\n=== START BLOC ===\n");
	fprintf(stderr, "| entries:\t%ld\n", bloc->length);
	for (size_t i = 0; i < bloc->length - 1; i++) {
		fprintf(stderr, "| entry %ld:\t", bloc->length - i - 2);
		print_bruijn(bloc->entries[i]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "| final:\t");
	print_bruijn(bloc->entries[bloc->length - 1]);
	fprintf(stderr, "\n");
	fprintf(stderr, "=== END BLOC ===\n\n");
}
