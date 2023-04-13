// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_SPEC_H
#define BLOC_SPEC_H

#define BLOC_IDENTIFIER "BLoC"
#define BLOC_IDENTIFIER_LENGTH 4

struct bloc_header {
	char identifier[BLOC_IDENTIFIER_LENGTH];
	short length;
	void *entries;
} __attribute__((packed));

struct bloc_entry {
	void *expression;
} __attribute__((packed));

#endif
