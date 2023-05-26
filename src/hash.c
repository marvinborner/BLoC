//-----------------------------------------------------------------------------
// xxHash Library
// Copyright (c) 2012-2021 Yann Collet
// All rights reserved.
//
// BSD 2-Clause License (https://www.opensource.org/licenses/bsd-license.php)
//
// xxHash3
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

#include <hash.h>

#define XXH_PRIME_1 11400714785074694791ULL
#define XXH_PRIME_2 14029467366897019727ULL
#define XXH_PRIME_3 1609587929392839161ULL
#define XXH_PRIME_4 9650029242287828579ULL
#define XXH_PRIME_5 2870177450012600261ULL

static uint64_t XXH_read64(const void *memptr)
{
	uint64_t val;
	memcpy(&val, memptr, sizeof(val));
	return val;
}

static uint32_t XXH_read32(const void *memptr)
{
	uint32_t val;
	memcpy(&val, memptr, sizeof(val));
	return val;
}

static uint64_t XXH_rotl64(uint64_t x, int r)
{
	return (x << r) | (x >> (64 - r));
}

hash_t hash(const void *data, size_t len, uint64_t seed)
{
	const uint8_t *p = (const uint8_t *)data;
	const uint8_t *const end = p + len;
	uint64_t h64;

	if (len >= 32) {
		const uint8_t *const limit = end - 32;
		uint64_t v1 = seed + XXH_PRIME_1 + XXH_PRIME_2;
		uint64_t v2 = seed + XXH_PRIME_2;
		uint64_t v3 = seed + 0;
		uint64_t v4 = seed - XXH_PRIME_1;

		do {
			v1 += XXH_read64(p) * XXH_PRIME_2;
			v1 = XXH_rotl64(v1, 31);
			v1 *= XXH_PRIME_1;

			v2 += XXH_read64(p + 8) * XXH_PRIME_2;
			v2 = XXH_rotl64(v2, 31);
			v2 *= XXH_PRIME_1;

			v3 += XXH_read64(p + 16) * XXH_PRIME_2;
			v3 = XXH_rotl64(v3, 31);
			v3 *= XXH_PRIME_1;

			v4 += XXH_read64(p + 24) * XXH_PRIME_2;
			v4 = XXH_rotl64(v4, 31);
			v4 *= XXH_PRIME_1;

			p += 32;
		} while (p <= limit);

		h64 = XXH_rotl64(v1, 1) + XXH_rotl64(v2, 7) +
		      XXH_rotl64(v3, 12) + XXH_rotl64(v4, 18);

		v1 *= XXH_PRIME_2;
		v1 = XXH_rotl64(v1, 31);
		v1 *= XXH_PRIME_1;
		h64 ^= v1;
		h64 = h64 * XXH_PRIME_1 + XXH_PRIME_4;

		v2 *= XXH_PRIME_2;
		v2 = XXH_rotl64(v2, 31);
		v2 *= XXH_PRIME_1;
		h64 ^= v2;
		h64 = h64 * XXH_PRIME_1 + XXH_PRIME_4;

		v3 *= XXH_PRIME_2;
		v3 = XXH_rotl64(v3, 31);
		v3 *= XXH_PRIME_1;
		h64 ^= v3;
		h64 = h64 * XXH_PRIME_1 + XXH_PRIME_4;

		v4 *= XXH_PRIME_2;
		v4 = XXH_rotl64(v4, 31);
		v4 *= XXH_PRIME_1;
		h64 ^= v4;
		h64 = h64 * XXH_PRIME_1 + XXH_PRIME_4;
	} else {
		h64 = seed + XXH_PRIME_5;
	}

	h64 += (uint64_t)len;

	while (p + 8 <= end) {
		uint64_t k1 = XXH_read64(p);
		k1 *= XXH_PRIME_2;
		k1 = XXH_rotl64(k1, 31);
		k1 *= XXH_PRIME_1;
		h64 ^= k1;
		h64 = XXH_rotl64(h64, 27) * XXH_PRIME_1 + XXH_PRIME_4;
		p += 8;
	}

	if (p + 4 <= end) {
		h64 ^= (uint64_t)(XXH_read32(p)) * XXH_PRIME_1;
		h64 = XXH_rotl64(h64, 23) * XXH_PRIME_2 + XXH_PRIME_3;
		p += 4;
	}

	while (p < end) {
		h64 ^= (*p) * XXH_PRIME_5;
		h64 = XXH_rotl64(h64, 11) * XXH_PRIME_1;
		p++;
	}

	h64 ^= h64 >> 33;
	h64 *= XXH_PRIME_2;
	h64 ^= h64 >> 29;
	h64 *= XXH_PRIME_3;
	h64 ^= h64 >> 32;

	return h64;
}
