// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

// most of the actual optimizing is done in tree.c
// this file does some extra steps to optimize the tree even further

#include <search.h>
#include <assert.h>
#include <stdlib.h>

#include <hash.h>
#include <log.h>
#include <optimize.h>
#include <pqueue.h>

struct tree_tracker {
	hash_t hash;
	struct tree *tree;
	int count; // reference/occurrence count
	size_t position; // in queue
};

struct hash_to_tree {
	hash_t hash;
	struct tree *tree;
};

// comparison_fn_t for tsearch
static int hash_compare(const void *_a, const void *_b)
{
	const struct tree_tracker *a = _a;
	const struct tree_tracker *b = _b;

	if (a->hash < b->hash)
		return -1;
	if (a->hash > b->hash)
		return 1;
	return 0;
}

// constructs a tree/map/set of all hashes and their occurrence count
// this is needed because the index count changes (currently untracked, see README)
// during tree invalidation and less used indices should get shorter encodings
static void generate_index_mappings(struct tree *tree, void **all_trees,
				    void **set)
{
	switch (tree->type) {
	case ABS:
		generate_index_mappings(tree->u.abs.term, all_trees, set);
		break;
	case APP:
		generate_index_mappings(tree->u.app.lhs, all_trees, set);
		generate_index_mappings(tree->u.app.rhs, all_trees, set);
		break;
	case VAR:
		break;
	case REF:;
		// increase count of reference
		struct tree_tracker *element = malloc(sizeof(*element));
		if (!element)
			fatal("out of memory!\n");
		element->hash = tree->u.ref.hash;
		struct tree_tracker **handle =
			tsearch(element, set, hash_compare);
		if (*handle == element) { // first of its kind
			struct hash_to_tree *tree_element =
				malloc(sizeof(*tree_element));
			tree_element->hash = tree->u.ref.hash;
			struct hash_to_tree **ref_tree =
				tfind(element, all_trees, hash_compare);
			if (!ref_tree)
				fatal("referred tree not found!\n");
			free(tree_element);

			element->count = 1;
			element->tree = (*ref_tree)->tree;
			assert(element->tree);
			generate_index_mappings(element->tree, all_trees, set);
		} else {
			free(element); // already exists, not needed
			(*handle)->count++;
		}
		break;
	default:
		fatal("invalid type %d\n", tree->type);
	}
}

// sets corresponding table_index of references
static void fix_tree(struct tree *tree, void **set)
{
	switch (tree->type) {
	case ABS:
		fix_tree(tree->u.abs.term, set);
		break;
	case APP:
		fix_tree(tree->u.app.lhs, set);
		fix_tree(tree->u.app.rhs, set);
		break;
	case VAR:
		break;
	case REF:;
		struct tree_tracker *element = malloc(sizeof(*element));
		element->hash = tree->u.ref.hash;
		if (!element)
			fatal("out of memory!\n");
		struct tree_tracker **handle =
			tfind(element, set, hash_compare);
		assert(handle); // must exist
		free(element);
		tree->u.ref.table_index = (*handle)->position;
		fix_tree((*handle)->tree, set);
		break;
	default:
		fatal("invalid type %d\n", tree->type);
	}
}

// priority of candidate -> occurrence count
static pqueue_pri_t get_pri(void *a)
{
	return ((struct tree_tracker *)a)->count;
}

static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr)
{
	return next < curr;
}

static void set_pos(void *a, size_t pos)
{
	((struct tree_tracker *)a)->position = pos - 1;
}

static struct pqueue *set_queue; // i know this is stupid but tsearch is stupider
static void walk(void *data, VISIT which, int depth)
{
	(void)depth;
	if (which != preorder && which != leaf)
		return;

	struct tree_tracker *element = *(struct tree_tracker **)data;
	// TODO: Merge elements with =1 references
	if (element->count) { // only insert elements with references
		pqueue_insert(set_queue, element);
	}
}

struct list *optimize_tree(struct tree *tree, void **all_trees)
{
	void *set = 0;
	generate_index_mappings(tree, all_trees, &set);

	// pqueue from mappings: hash -> tree_tracker
	set_queue = pqueue_init(2 << 7, cmp_pri, get_pri, set_pos);
	twalk(set, walk);

	fix_tree(tree, &set);

	struct list *list = list_add(0, tree);

	for (size_t i = 1; i < pqueue_size(set_queue) + 1; i++) {
		struct tree_tracker *element = set_queue->d[i];
		list = list_add(list, element->tree);
	}
	pqueue_free(set_queue);

	return list;
}
