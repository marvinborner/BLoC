// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <build.h>
#include <log.h>

static void write_bit(char val, FILE *file, char *byte, int *bit)
{
	if (*bit > 7) { // flush byte
		fwrite(byte, 1, 1, file);
		*byte = 0;
		*bit = 0;
	}

	if (val)
		*byte |= 1UL << (7 - *bit);
	(*bit)++;
}

static void rec_write_bblc(struct tree *tree, FILE *file, char *byte, int *bit)
{
	switch (tree->type) {
	case ABS:
		write_bit(0, file, byte, bit);
		write_bit(0, file, byte, bit);
		rec_write_bblc(tree->u.abs.term, file, byte, bit);
		break;
	case APP:
		write_bit(0, file, byte, bit);
		write_bit(1, file, byte, bit);
		write_bit(0, file, byte, bit);
		rec_write_bblc(tree->u.app.lhs, file, byte, bit);
		rec_write_bblc(tree->u.app.rhs, file, byte, bit);
		break;
	case VAR:
		for (int i = 0; i <= tree->u.var.index; i++)
			write_bit(1, file, byte, bit);
		write_bit(0, file, byte, bit);
		break;
	case REF:
		write_bit(0, file, byte, bit);
		write_bit(1, file, byte, bit);
		write_bit(1, file, byte, bit);

		int ref = tree->u.ref.table_index;
		int bits = 0;

		// write index length bit prefixes
		if (ref < 2 << 7) {
			bits = 8;
			write_bit(0, file, byte, bit);
			write_bit(0, file, byte, bit);
		} else if (ref < 2 << 15) {
			bits = 16;
			write_bit(0, file, byte, bit);
			write_bit(1, file, byte, bit);
		} else if (ref < 2 << 31) {
			bits = 32;
			write_bit(1, file, byte, bit);
			write_bit(0, file, byte, bit);
		} else if (ref < 2 << 63) {
			bits = 64; // i wanna see that program lol
			write_bit(1, file, byte, bit);
			write_bit(1, file, byte, bit);
		}
		for (int i = 0; i < bits; i++)
			write_bit((ref >> i) & 1, file, byte, bit);
		break;
	default:
		fatal("invalid type %d\n", tree->type);
	}
}

// writes bit-encoded blc into file
static void write_bblc(struct tree *tree, FILE *file)
{
	char byte = 0;
	int bit = 0;
	rec_write_bblc(tree, file, &byte, &bit);

	if (bit) // flush final
		fwrite(&byte, 1, 1, file);
}

static void write_bloc_file(struct list *table, FILE *file)
{
	short length = table->val;
	fwrite(BLOC_IDENTIFIER, BLOC_IDENTIFIER_LENGTH, 1, file);
	fwrite(&length, 2, 1, file);

	struct list *iterator = table;
	while (iterator) {
		write_bblc(iterator->data, file);
		iterator = iterator->next;
	}
}

void write_bloc(struct list *table, FILE *file)
{
	short length = table->val;
	debug("writing bloc with %ld elements\n", length);

	write_bloc_file(table, file);
}

static void fprint_bloc_blc(struct term *term, struct bloc_parsed *bloc,
			    FILE *file)
{
	switch (term->type) {
	case ABS:
		fprintf(file, "00");
		fprint_bloc_blc(term->u.abs.term, bloc, file);
		break;
	case APP:
		fprintf(file, "01");
		fprint_bloc_blc(term->u.app.lhs, bloc, file);
		fprint_bloc_blc(term->u.app.rhs, bloc, file);
		break;
	case VAR:
		for (int i = 0; i <= term->u.var.index; i++)
			fprintf(file, "1");
		fprintf(file, "0");
		break;
	case REF:
		if (term->u.ref.index + 1 >= bloc->length)
			fatal("invalid ref index %ld\n", term->u.ref.index);
		fprint_bloc_blc(
			bloc->entries[bloc->length - term->u.ref.index - 2],
			bloc, file);
		break;
	default:
		fatal("invalid type %d\n", term->type);
	}
}

void write_blc(struct bloc_parsed *bloc, FILE *file)
{
	fprint_bloc_blc(bloc->entries[bloc->length - 1], bloc, file);
	fprintf(file, "\n");
}
