// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_TREE_H
#define BLOC_TREE_H

#include <stdint.h>

#include <term.h>

#define VALIDATED_TREE ((int)0x0)
#define INVALIDATED_TREE ((int)0xffffffff)
#define FREEABLE_TREE(t)                                                       \
	((t)->state != VALIDATED_TREE && (t)->state != INVALIDATED_TREE)

struct tree {
	term_type type;
	uint32_t hash;
	int state; // zero or index to ref
	int size; // blc length
	int duplication_count; // needed count to be considered for deduplication
	union {
		struct {
			struct tree *term;
		} abs;
		struct {
			struct tree *lhs;
			struct tree *rhs;
		} app;
		struct {
			int index;
		} var;
		struct {
			size_t index;
		} ref;
	} u;
};

struct list {
	int val; // length or priority
	void *data;
	struct list *next;
};

struct list *tree_merge_duplicates(struct term *term);
void tree_destroy(struct list *table);

#endif
