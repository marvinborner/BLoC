// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_BUILD_H
#define BLOC_BUILD_H

#include <spec.h>
#include <tree.h>
#include <parse.h>

void write_bloc(struct list *table, const char *path);
void write_blc(struct bloc_parsed *bloc, const char *path);

#endif
