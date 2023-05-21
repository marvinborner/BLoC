// murmur3 originally by Austin Appleby
// Copyright (c) 2013, ksss
// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include <hash.h>

hash_t hash(const void *key, int len, uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995ULL;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t *data = (const uint64_t *)key;
	const uint64_t *end = data + (len / 8);

	while (data != end) {
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char *data2 = (const unsigned char *)data;

	int b = len & 7;
	if (b >= 7) {
		h ^= ((uint64_t)data2[6]) << 48;
	}
	if (b >= 6) {
		h ^= ((uint64_t)data2[5]) << 40;
	}
	if (b >= 5) {
		h ^= ((uint64_t)data2[4]) << 32;
	}
	if (b >= 4) {
		h ^= ((uint64_t)data2[3]) << 24;
	}
	if (b >= 3) {
		h ^= ((uint64_t)data2[2]) << 16;
	}
	if (b >= 2) {
		h ^= ((uint64_t)data2[1]) << 8;
	}
	if (b >= 1) {
		h ^= ((uint64_t)data2[0]);
		h *= m;
	}

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
