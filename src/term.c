// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <term.h>

struct term *new_term(term_type type)
{
	struct term *term = malloc(sizeof(*term));
	if (!term) {
		fprintf(stderr, "Out of memory!\n");
		abort();
	}
	term->type = type;
	return term;
}
