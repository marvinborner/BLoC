// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_HASH_H
#define BLOC_HASH_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t hash_t;

hash_t hash(const void *data, size_t len, uint64_t seed);

#endif
