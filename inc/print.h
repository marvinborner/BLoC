// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_PRINT_H
#define BLOC_PRINT_H

#include <term.h>
#include <parse.h>

void print_bruijn(struct term *term);
void print_blc(struct term *term);
void print_bloc(struct bloc_parsed *bloc);

#endif
