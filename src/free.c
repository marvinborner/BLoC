// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

#include <free.h>
#include <log.h>

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

void free_bloc(struct bloc_parsed *bloc)
{
	for (size_t i = 0; i < bloc->length; i++) {
		free_term(bloc->entries[i]);
	}

	free(bloc->entries);
	free(bloc);
}
