// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

// We need to find the longest repeating subexpressions.
// We do this by creating a kind-of merkle tree out of the expressions
// and finding the largest repeating subtrees.

#include <stdio.h>
#include <search.h>
#include <string.h>
#include <stdlib.h>

#include <log.h>
#include <pqueue.h>
#include <tree.h>
#include <hash.h>

static struct list *list_end = 0;
struct list *list_add(struct list *list, void *data)
{
	struct list *new = malloc(sizeof(*new));
	if (!new)
		fatal("out of memory!\n");
	new->next = list;
	new->data = data;
	new->val = list ? list->val + 1 : 1; // amount of trees in list
	return new;
}

// element of the tsearch tree
struct hash_to_list {
	hash_t hash;
	struct list *list;
};

// element of the tsearch tree
struct hash_to_tree {
	hash_t hash;
	struct tree *tree;
};

// comparison_fn_t for tsearch
static int hash_compare(const void *_a, const void *_b)
{
	const struct hash_to_list *a = _a;
	const struct hash_to_list *b = _b;

	if (a->hash < b->hash)
		return -1;
	if (a->hash > b->hash)
		return 1;
	return 0;
}

// applies the hash function to the tree's elements (similar to merkle trees)
// also creates a set of lists with deduplication candidates
// TODO: as above: rethink hash choice
extern size_t min_size;
static struct tree *build_tree(struct term *term, void **set)
{
	struct tree *tree = malloc(sizeof(*tree));
	if (!tree)
		fatal("out of memory!\n");
	tree->type = term->type;
	tree->state = VALIDATED_TREE;
	tree->duplication_count = 1;

	switch (term->type) {
	case ABS:
		tree->u.abs.term = build_tree(term->u.abs.term, set);
		tree->hash = hash((const uint8_t *)&tree->type,
				  sizeof(tree->type), tree->u.abs.term->hash);
		tree->size = tree->u.abs.term->size + 2;
		break;
	case APP:
		tree->u.app.lhs = build_tree(term->u.app.lhs, set);
		tree->u.app.rhs = build_tree(term->u.app.rhs, set);
		tree->hash = hash((const uint8_t *)&tree->type,
				  sizeof(tree->type), tree->u.app.lhs->hash);
		tree->hash = hash((const uint8_t *)&tree->hash,
				  sizeof(tree->hash), tree->u.app.rhs->hash);
		tree->size = tree->u.app.lhs->size + tree->u.app.rhs->size + 3;
		break;
	case VAR:
		tree->u.var.index = term->u.var.index;
		tree->hash = hash((const uint8_t *)&tree->type,
				  sizeof(tree->type), tree->u.var.index);
		tree->size = term->u.var.index;
		break;
	default:
		fatal("invalid type %d\n", term->type);
	}

	if (tree->size < min_size) // not suitable for deduplication
		return tree;

	struct hash_to_list *element = malloc(sizeof(*element));
	if (!element)
		fatal("out of memory!\n");
	element->hash = tree->hash;

	struct hash_to_list **handle = tsearch(element, set, hash_compare);
	if (*handle == element) { // first of its kind
		element->list = list_add(list_end, tree);
		return tree;
	}

	free(element); // already exists, not needed
	(*handle)->list = list_add((*handle)->list, tree);

	return tree;
}

static struct tree *clone_tree_root(struct tree *tree)
{
	struct tree *new = malloc(sizeof(*new));
	if (!new)
		fatal("out of memory!\n");
	new->type = tree->type;
	new->hash = tree->hash;
	new->duplication_count = tree->duplication_count;

	switch (tree->type) {
	case ABS:
		new->u.abs.term = tree->u.abs.term;
		break;
	case APP:
		new->u.app.lhs = tree->u.app.lhs;
		new->u.app.rhs = tree->u.app.rhs;
		break;
	case VAR:
		new->u.var.index = tree->u.var.index;
		break;
	default:
		free(new);
		fatal("invalid type %d\n", tree->type);
	}

	return new;
}

static void invalidate_tree(struct tree *tree, int duplication_count)
{
	tree->state = INVALIDATED_TREE;
	tree->duplication_count = duplication_count;
	switch (tree->type) {
	case ABS:
		invalidate_tree(tree->u.abs.term, duplication_count);
		break;
	case APP:
		invalidate_tree(tree->u.app.lhs, duplication_count);
		invalidate_tree(tree->u.app.rhs, duplication_count);
		break;
	case VAR:
		break;
	default:
		fatal("invalid type %d\n", tree->type);
	}
}

static void free_tree(struct tree *tree, int ref_only)
{
	switch (tree->type) {
	case ABS:
		free_tree(tree->u.abs.term, ref_only);
		break;
	case APP:
		free_tree(tree->u.app.lhs, ref_only);
		free_tree(tree->u.app.rhs, ref_only);
		break;
	case VAR:
		break;
	case REF:
		break;
	default:
		fatal("invalid type %d\n", tree->type);
		return;
	}

	if (!ref_only || (ref_only && FREEABLE_TREE(tree)))
		free(tree);
}

static void ref_invalidated_tree(struct tree *tree)
{
	switch (tree->type) {
	case ABS:
		free_tree(tree->u.abs.term, 1);
		break;
	case APP:
		free_tree(tree->u.app.lhs, 1);
		free_tree(tree->u.app.rhs, 1);
		break;
	case VAR:
		break;
	default:
		fatal("invalid type %d\n", tree->type);
	}
	if (tree->state != INVALIDATED_TREE &&
	    tree->state != VALIDATED_TREE) { // is reffed
		tree->type = REF;
		tree->u.ref.hash = tree->state;
		tree->state = VALIDATED_TREE;
	}
}

// priority of candidate -> length of expression
// TODO: What about occurrence count (list length)?
static pqueue_pri_t get_pri(void *a)
{
	return ((struct tree *)((struct list *)a)->data)->size;
}

static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr)
{
	return next < curr;
}

static void set_pos(void *a, size_t position)
{
	(void)a;
	(void)position;
}

struct tree *tree_merge_duplicates(struct term *term, void **all_trees)
{
	debug("building the merkle tree and deduplication set\n");

	// get the deduplication candidates
	void *set = 0;
	struct tree *built = build_tree(term, &set);
	if (!set) {
		debug("term not suitable for deduplication, emitting directly\n");
		return built;
	}

	// construct priority queue while deleting set
	// ~> sorts the candidates by get_pri
	debug("constructing priority queue\n");
	struct pqueue *prioritized =
		pqueue_init(2 << 15, cmp_pri, get_pri, set_pos);
	if (!prioritized)
		fatal("can't create pqueue\n");
	while (set) {
		struct hash_to_list *element = *(struct hash_to_list **)set;
		pqueue_insert(prioritized, element->list);
		tdelete(element, &set, hash_compare);
		free(element);
	}

	struct list *invalidated = list_end;

	// add longest (=> blueprint/structure of expression)
	struct list *longest = pqueue_pop(prioritized);
	struct hash_to_tree *element = malloc(sizeof(*element));
	element->hash = ((struct tree *)longest->data)->hash;
	element->tree = longest->data;
	tsearch(element, all_trees, hash_compare);

	debug("iterating priority queue, invalidating duplicates\n");
	struct list *iterator;
	while ((iterator = pqueue_pop(prioritized))) {
		// only consider merging if they occur >1 times
		if (iterator->val <= 1)
			continue;

		struct tree *head = iterator->data;

		// skip if invalidated and not duplicated enough
		if (head->state != VALIDATED_TREE &&
		    head->duplication_count >= iterator->val)
			continue;

		// clone root so it doesn't get replaced by a ref to itself
		struct tree *cloned_head = clone_tree_root(head);
		cloned_head->state = INVALIDATED_TREE;

		element = malloc(sizeof(*element));
		element->hash = cloned_head->hash;
		element->tree = cloned_head;
		struct hash_to_tree **handle =
			tsearch(element, all_trees, hash_compare);
		if (*handle != element)
			free(element); // already exists, not needed

		// invalidate all subtrees
		// invalidated trees will be replaced with a reference
		struct list *list = iterator;
		while (list) {
			invalidate_tree(list->data, list->val);

			// keep a ref for later replacement
			((struct tree *)list->data)->state = head->hash;

			invalidated = list_add(invalidated, list->data);
			list = list->next;
		}
	}

	// destroy invalidated list and replace reffed subtrees
	debug("replacing invalidated trees with references\n");
	iterator = invalidated;
	while (iterator) {
		ref_invalidated_tree(iterator->data);
		struct list *temp = iterator->next;
		free(iterator);
		iterator = temp;
	}

	// destroy prioritized list
	pqueue_free(prioritized);

	return longest->data;
}

void tree_destroy(struct list *table)
{
	return;
	debug("freeing %d tree elements\n", table->val);
	struct list *iterator = table;
	while (iterator) {
		free_tree(iterator->data, 0);
		struct list *temp = iterator->next;
		free(iterator);
		iterator = temp;
	}
}
