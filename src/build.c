// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <build.h>
#include <free.h>
#include <parse.h>

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

		// TODO: The bit-order of encoded shorts is kinda arbitrary
		short ref = tree->u.ref.index;
		for (int i = 0; i < 16; i++)
			write_bit((ref >> i) & 1, file, byte, bit);
		break;
	default:
		fprintf(stderr, "invalid type %d\n", tree->type);
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

void write_bloc(struct list *table, const char *path)
{
	short length = table->val;

	FILE *file = fopen(path, "wb");
	fwrite(BLOC_IDENTIFIER, BLOC_IDENTIFIER_LENGTH, 1, file);
	fwrite(&length, 2, 1, file);

	struct list *iterator = table;
	while (iterator) {
		write_bblc(iterator->data, file);
		iterator = iterator->next;
	}

	fclose(file);
}
