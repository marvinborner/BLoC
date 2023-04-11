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

static void rec_write_bblc(struct term *term, FILE *file, char *byte, int *bit)
{
	switch (term->type) {
	case ABS:
		write_bit(0, file, byte, bit);
		write_bit(0, file, byte, bit);
		rec_write_bblc(term->u.abs.term, file, byte, bit);
		break;
	case APP:
		write_bit(0, file, byte, bit);
		write_bit(1, file, byte, bit);
		rec_write_bblc(term->u.app.lhs, file, byte, bit);
		rec_write_bblc(term->u.app.rhs, file, byte, bit);
		break;
	case VAR:
		for (int i = 0; i <= term->u.var.index; i++)
			write_bit(1, file, byte, bit);
		write_bit(0, file, byte, bit);
		break;
	default:
		fprintf(stderr, "Invalid type %d\n", term->type);
	}
}

// writes bit-encoded blc into file
static void write_bblc(struct term *term, FILE *file)
{
	char byte = 0;
	int bit = 0;
	rec_write_bblc(term, file, &byte, &bit);

	if (bit) // flush final
		fwrite(&byte, 1, 1, file);
}

void write_bloc(struct term *term, const char *path)
{
	(void)term; // TODO

	// example data
	short length = 2;
	struct term *M = parse_blc("0000000001100111100111100111100111011110");
	struct term *N = parse_blc("00000000011001111001111001100111011110");

	FILE *file = fopen(path, "wb");
	fwrite(BLOC_IDENTIFIER, BLOC_IDENTIFIER_LENGTH, 1, file);
	fwrite(&length, 2, 1, file);

	write_bblc(M, file);
	write_bblc(N, file);

	// TODO
	short table_index = 0;
	char byte = 0;
	int bit = 0;
	write_bit(0, file, &byte, &bit);
	write_bit(0, file, &byte, &bit);
	write_bit(0, file, &byte, &bit);
	write_bit(1, file, &byte, &bit);
	write_bit(0, file, &byte, &bit);
	write_bit(1, file, &byte, &bit);
	write_bit(0, file, &byte, &bit);
	write_bit(1, file, &byte, &bit);
	write_bit(1, file, &byte, &bit);

	for (int i = 0; i < 16; i++)
		write_bit(0, file, &byte, &bit);

	write_bit(0, file, &byte, &bit);
	write_bit(0, file, &byte, &bit);
	write_bit(1, file, &byte, &bit);

	for (int i = 0; i < 16; i++)
		write_bit(0, file, &byte, &bit);

	write_bit(1, file, &byte, &bit);

	for (int i = 0; i < 16; i++)
		write_bit(0, file, &byte, &bit);

	write_bit(1, file, &byte, &bit);

	for (int i = 0; i < 16; i++)
		write_bit(0, file, &byte, &bit);

	if (bit) // flush final
		fwrite(&byte, 1, 1, file);

	fclose(file);

	free_term(M);
	free_term(N);
}
