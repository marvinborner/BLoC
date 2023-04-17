// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <print.h>
#include <log.h>

void print_bruijn(struct term *term)
{
	switch (term->type) {
	case ABS:
		printf("[");
		print_bruijn(term->u.abs.term);
		printf("]");
		break;
	case APP:
		printf("(");
		print_bruijn(term->u.app.lhs);
		printf(" ");
		print_bruijn(term->u.app.rhs);
		printf(")");
		break;
	case VAR:
		printf("%d", term->u.var.index);
		break;
	case REF:
		printf("<%ld>", term->u.ref.index);
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
	printf("\n=== START BLOC ===\n");
	printf("| entries:\t%ld\n", bloc->length);
	for (size_t i = 0; i < bloc->length - 1; i++) {
		printf("| entry %ld:\t", bloc->length - i - 2);
		print_bruijn(bloc->entries[i]);
		printf("\n");
	}
	printf("| final:\t");
	print_bruijn(bloc->entries[bloc->length - 1]);
	printf("\n");
	printf("=== END BLOC ===\n\n");
}
