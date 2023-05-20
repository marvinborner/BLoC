// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <term.h>
#include <print.h>
#include <log.h>

struct term *new_term(term_type type)
{
	struct term *term = malloc(sizeof(*term));
	if (!term)
		fatal("out of memory!\n");
	term->type = type;
	return term;
}

void free_term(struct term *term)
{
	switch (term->type) {
	case ABS:
		free_term(term->u.abs.term);
		free(term);
		break;
	case APP:
		free_term(term->u.app.lhs);
		free_term(term->u.app.rhs);
		free(term);
		break;
	case VAR:
		free(term);
		break;
	case REF:
		free(term);
		break;
	default:
		fatal("invalid type %d\n", term->type);
	}
}

void diff_term(struct term *a, struct term *b)
{
	if (a->type != b->type) {
		fprintf(stderr, "Term a: ");
		print_bruijn(a);
		fprintf(stderr, "\nTerm b: ");
		print_bruijn(b);
		fatal("\ntype mismatch %d %d\n", a->type, b->type);
	}

	switch (a->type) {
	case ABS:
		diff_term(a->u.abs.term, b->u.abs.term);
		break;
	case APP:
		diff_term(a->u.app.lhs, b->u.app.lhs);
		diff_term(a->u.app.rhs, b->u.app.rhs);
		break;
	case VAR:
		if (a->u.var.index != b->u.var.index)
			fatal("var mismatch %d=%d\n", a->u.var.index,
			      b->u.var.index);
		break;
	default:
		fatal("invalid type %d\n", a->type);
	}
}
