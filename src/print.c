// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <print.h>

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
		fprintf(stderr, "Invalid type %d\n", term->type);
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
		fprintf(stderr, "Invalid type %d\n", term->type);
	}
}
