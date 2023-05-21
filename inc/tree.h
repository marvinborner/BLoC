// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

#ifndef BLOC_TREE_H
#define BLOC_TREE_H

#include <stdint.h>

#include <term.h>
#include <hash.h>

#define VALIDATED_TREE ((hash_t)0x0)
#define INVALIDATED_TREE ((hash_t)0xffffffff)
#define FREEABLE_TREE(t)                                                       \
	((t)->state != VALIDATED_TREE && (t)->state != INVALIDATED_TREE)

struct tree {
	term_type type;
	hash_t hash;
	hash_t state; // zero or index to ref
	size_t size; // blc length
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
			hash_t hash;
			size_t table_index;
		} ref;
	} u;
};

struct list {
	int val; // length or priority
	void *data;
	struct list *next;
};

struct list *list_add(struct list *list, void *data);
struct tree *tree_merge_duplicates(struct term *term, void **all_trees);
void tree_destroy(struct list *table);

#endif
