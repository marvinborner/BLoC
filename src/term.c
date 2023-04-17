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
		fatal("Out of memory!\n");
	term->type = type;
	return term;
}
