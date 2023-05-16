// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <term.h>
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
