// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <term.h>
#include <spec.h>
#include <parse.h>

static struct term *rec_blc(const char **term)
{
	struct term *res = 0;
	if (!**term) {
		fprintf(stderr, "invalid parsing state!\n");
	} else if (**term == '0' && *(*term + 1) == '0') {
		(*term) += 2;
		res = new_term(ABS);
		res->u.abs.term = rec_blc(term);
	} else if (**term == '0' && *(*term + 1) == '1') {
		(*term) += 2;
		res = new_term(APP);
		res->u.app.lhs = rec_blc(term);
		res->u.app.rhs = rec_blc(term);
	} else if (**term == '1') {
		const char *cur = *term;
		while (**term == '1')
			(*term)++;
		res = new_term(VAR);
		res->u.var.index = *term - cur - 1;
		(*term)++;
	} else {
		(*term)++;
		res = rec_blc(term);
	}
	return res;
}

struct term *parse_blc(const char *term)
{
	return rec_blc(&term);
}

#define BIT_AT(i) (term[(i) / 8] & (1 << (7 - ((i) % 8))))

// parses normal bit-encoded blc
static struct term *parse_bblc(const char *term, size_t *bit)
{
	struct term *res = 0;
	if (!BIT_AT(*bit) && !BIT_AT(*bit + 1)) {
		(*bit) += 2;
		res = new_term(ABS);
		res->u.abs.term = parse_bblc(term, bit);
	} else if (!BIT_AT(*bit) && BIT_AT(*bit + 1)) {
		(*bit) += 2;
		res = new_term(APP);
		res->u.app.lhs = parse_bblc(term, bit);
		res->u.app.rhs = parse_bblc(term, bit);
	} else if (BIT_AT(*bit)) {
		const size_t cur = *bit;
		while (BIT_AT(*bit))
			(*bit)++;
		res = new_term(VAR);
		res->u.var.index = *bit - cur - 1;
		(*bit)++;
	} else {
		(*bit)++;
		res = parse_bblc(term, bit);
	}
	return res;
}

// parses bloc's bit-encoded blc (1I => 2B index)
static struct term *parse_bloc_bblc(const char *term, size_t *bit)
{
	struct term *res = 0;
	if (!BIT_AT(*bit) && !BIT_AT(*bit + 1)) {
		(*bit) += 2;
		res = new_term(ABS);
		res->u.abs.term = parse_bloc_bblc(term, bit);
	} else if (!BIT_AT(*bit) && BIT_AT(*bit + 1)) {
		(*bit) += 2;
		res = new_term(APP);
		res->u.app.lhs = parse_bloc_bblc(term, bit);
		res->u.app.rhs = parse_bloc_bblc(term, bit);
	} else if (BIT_AT(*bit)) {
		(*bit) += 1;
		res = new_term(REF);
		short index = 0;
		for (int i = 0; i < 16; i++)
			index |= (BIT_AT(*bit) >> (7 - (*bit % 8))) << i;
		res->u.ref.index = index;
		(*bit) += 16;
	} else {
		(*bit)++;
		res = parse_bloc_bblc(term, bit);
	}
	return res;
}

struct bloc_parsed *parse_bloc(const void *bloc)
{
	const struct bloc_header *header = bloc;
	if (memcmp(header->identifier, BLOC_IDENTIFIER,
		   (size_t)BLOC_IDENTIFIER_LENGTH)) {
		fprintf(stderr, "invalid BLoC identifier!\n");
		return 0;
	}

	struct bloc_parsed *parsed = malloc(sizeof(*parsed));
	parsed->length = header->length;
	parsed->entries = malloc(header->length * sizeof(struct term *));

	const struct bloc_entry *current = (const void *)&header->data;
	for (size_t i = 0; i < parsed->length; i++) {
		size_t len = 0;
		parsed->entries[i] = parse_bblc((const char *)current, &len);
		current =
			(const struct bloc_entry *)(((const char *)current) +
						    (len / 8) + (len % 8 != 0));
	}

	size_t len = 0;
	const char *term = (const char *)current;
	parsed->term = parse_bloc_bblc(term, &len);

	return parsed;
}

static struct term *rec_bloc(struct term *term, struct bloc_parsed *bloc)
{
	switch (term->type) {
	case ABS:
		rec_bloc(term->u.abs.term, bloc);
		break;
	case APP:
		rec_bloc(term->u.app.lhs, bloc);
		rec_bloc(term->u.app.rhs, bloc);
		break;
	case VAR:
		fprintf(stderr, "bloc can't have vars\n");
		return 0;
	case REF:
		if (term->u.ref.index >= bloc->length) {
			fprintf(stderr, "invalid entry reference\n");
			return 0;
		}
		memcpy(term, bloc->entries[term->u.ref.index], sizeof(*term));
		break;
	default:
		fprintf(stderr, "invalid type %d\n", term->type);
		return 0;
	}
	return term;
}

struct term *from_bloc(struct bloc_parsed *bloc)
{
	return rec_bloc(bloc->term, bloc);
}