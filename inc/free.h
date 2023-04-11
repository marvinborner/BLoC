// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_FREE_H
#define BLOC_FREE_H

#include <parse.h>
#include <term.h>

void free_term(struct term *term);
void free_bloc(struct bloc_parsed *bloc);

#endif
