// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_HASH_H
#define BLOC_HASH_H

#include <stdint.h>
#include <stddef.h>

typedef uint32_t hash_t;

hash_t hash(const uint8_t *key, size_t len, uint32_t seed);

#endif
