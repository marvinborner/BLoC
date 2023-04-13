// Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
// SPDX-License-Identifier: MIT

// We need to find the longest repeating subexpressions.
// We do this by creating a kind-of merkle tree out of the expressions
// and finding the largest repeating subtrees.

#include <stdio.h>
#include <search.h>
#include <string.h>
#include <stdlib.h>

#include <tree.h>

static inline uint32_t murmur_32_scramble(uint32_t k)
{
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

// TODO: I'm really unsure whether murmur3 is appropriate for this.
static uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
	uint32_t k;
	for (size_t i = len >> 2; i; i--) {
		memcpy(&k, key, sizeof(uint32_t));
		key += sizeof(uint32_t);
		h ^= murmur_32_scramble(k);
		h = (h << 13) | (h >> 19);
		h = h * 5 + 0xe6546b64;
	}
	k = 0;
	for (size_t i = len & 3; i; i--) {
		k <<= 8;
		k |= key[i - 1];
	}
	h ^= murmur_32_scramble(k);
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

static struct list *list_end = 0;
static struct list *list_add(struct list *list, void *data)
{
	struct list *new = malloc(sizeof(*new));
	new->next = list;
	new->data = data;
	new->val = list ? list->val + 1 : 1; // amount of trees in list
	return new;
}

static struct list *list_add_prioritized(struct list *list, struct list *sub)
{
	struct list *new = malloc(sizeof(*new));
	/* new->val = ((struct tree *)sub->data)->size * sub->val; */
	new->val = ((struct tree *)sub->data)->size;
	new->data = sub;

	if (!list) {
		new->next = list;
		return new;
	}

	if (new->val >= list->val) { // insert at head
		new->next = list;
		return new;
	}

	struct list *iterator = list;
	while (iterator->next && new->val < iterator->next->val) {
		iterator = iterator->next;
	}

	new->next = iterator->next;
	iterator->next = new;

	return list;
}

// element of the tsearch tree
struct set_element {
	uint32_t hash;
	struct list *list;
};

// comparison_fn_t for tsearch
static int hash_compare(const void *_a, const void *_b)
{
	const struct set_element *a = _a;
	const struct set_element *b = _b;

	if (a->hash < b->hash)
		return -1;
	if (a->hash > b->hash)
		return 1;
	return 0;
}

// applies the hash function to the tree's elements (similar to merkle trees)
// TODO: as above: rethink hash choice
static struct tree *build_tree(struct term *term, void **set)
{
	struct tree *tree = malloc(sizeof(*tree));
	tree->type = term->type;
	tree->state = VALIDATED_TREE;

	switch (term->type) {
	case ABS:
		tree->u.abs.term = build_tree(term->u.abs.term, set);
		tree->hash =
			murmur3_32((const uint8_t *)&tree->type,
				   sizeof(tree->type), tree->u.abs.term->hash);
		tree->size = tree->u.abs.term->size + 2;
		break;
	case APP:
		tree->u.app.lhs = build_tree(term->u.app.lhs, set);
		tree->u.app.rhs = build_tree(term->u.app.rhs, set);
		tree->hash =
			murmur3_32((const uint8_t *)&tree->type,
				   sizeof(tree->type), tree->u.app.lhs->hash);
		tree->hash =
			murmur3_32((const uint8_t *)&tree->hash,
				   sizeof(tree->hash), tree->u.app.rhs->hash);
		tree->size = tree->u.app.lhs->size + tree->u.app.rhs->size + 3;
		break;
	case VAR:
		tree->u.var.index = term->u.var.index;
		tree->hash = murmur3_32((const uint8_t *)&tree->type,
					sizeof(tree->type), tree->u.var.index);
		tree->size = term->u.var.index;
		break;
	default:
		fprintf(stderr, "invalid type %d\n", term->type);
		return 0;
	}

	if (tree->size < 10) // not suitable for deduplication
		return tree;

	struct set_element *element = malloc(sizeof(*element));
	element->hash = tree->hash;

	struct set_element **handle = tsearch(element, set, hash_compare);
	if (*handle == element) { // first of its kind
		element->list = list_add(list_end, tree);
		return tree;
	}

	free(element); // already exists, not needed
	(*handle)->list = list_add((*handle)->list, tree);

	fprintf(stderr, "found suitable duplicate! %x %d\n", tree->hash,
		tree->size);
	return tree;
}

static struct term *tree_to_term(struct tree *tree)
{
	struct term *term = new_term(tree->type);

	switch (tree->type) {
	case ABS:
		term->u.abs.term = tree_to_term(tree->u.abs.term);
		break;
	case APP:
		term->u.app.lhs = tree_to_term(tree->u.app.lhs);
		term->u.app.rhs = tree_to_term(tree->u.app.rhs);
		break;
	case VAR:
		term->u.var.index = tree->u.var.index;
		break;
	case REF:
		term->u.ref.index = tree->u.ref.index;
		break;
	default:
		fprintf(stderr, "invalid type %d\n", term->type);
	}
	return term;
}

static struct tree *clone_tree_root(struct tree *tree)
{
	struct tree *new = malloc(sizeof(*new));
	new->type = tree->type;
	new->hash = tree->hash;

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
		fprintf(stderr, "invalid type %d\n", tree->type);
		free(new);
		return 0;
	}

	return new;
}

static void invalidate_tree(struct tree *tree)
{
	tree->state = INVALIDATED_TREE;
	switch (tree->type) {
	case ABS:
		invalidate_tree(tree->u.abs.term);
		break;
	case APP:
		invalidate_tree(tree->u.app.lhs);
		invalidate_tree(tree->u.app.rhs);
		break;
	case VAR:
		break;
	default:
		fprintf(stderr, "invalid type %d\n", tree->type);
	}
}

static void free_tree(struct tree *tree)
{
	switch (tree->type) {
	case ABS:
		free_tree(tree->u.abs.term);
		break;
	case APP:
		free_tree(tree->u.app.lhs);
		free_tree(tree->u.app.rhs);
		break;
	case VAR:
		break;
	case REF:
		break;
	default:
		fprintf(stderr, "invalid type %d\n", tree->type);
		return;
	}

	if (FREEABLE_TREE(tree))
		free(tree);
}

static void ref_invalidated_tree(struct tree *tree)
{
	switch (tree->type) {
	case ABS:
		free_tree(tree->u.abs.term);
		break;
	case APP:
		free_tree(tree->u.app.lhs);
		free_tree(tree->u.app.rhs);
		break;
	case VAR:
		break;
	default:
		fprintf(stderr, "invalid type %d\n", tree->type);
	}
	if (tree->state != INVALIDATED_TREE &&
	    tree->state != VALIDATED_TREE) { // is reffed
		tree->type = REF;
		tree->u.ref.index = tree->state - 1;
		tree->state = VALIDATED_TREE;
	}
}

struct list *tree_merge_duplicates(struct term *term)
{
	void *set = 0;
	build_tree(term, &set);
	if (!set)
		return 0;

	// construct priority list while deleting set
	struct list *prioritized = list_end;
	while (set) {
		struct set_element *element = *(struct set_element **)set;
		prioritized = list_add_prioritized(prioritized, element->list);
		tdelete(element, &set, hash_compare);
		free(element);
	}

	struct list *final = list_end;
	struct list *invalidated = list_end;

	struct list *iterator = prioritized;

	// add longest (=> blueprint/structure of expression)
	final = list_add(final, ((struct list *)iterator->data)->data);
	iterator = iterator->next;

	while (iterator) {
		struct list *list = iterator->data;
		struct tree *head = list->data;

		print_bruijn(tree_to_term(head));
		printf("\n%x: %d\n\n", head->hash, list->val);

		if (head->state != VALIDATED_TREE) {
			printf("skipping invalidated: %x\n", head->hash);
			iterator = iterator->next;
			continue;
		}

		if (list->val > 1) { // needs merging
			// clone root so it doesn't get replaced by a ref to itself
			struct tree *cloned_head = clone_tree_root(head);
			cloned_head->state = INVALIDATED_TREE;
			final = list_add(final, cloned_head);

			// invalidate all subtrees
			while (list) {
				invalidate_tree(list->data);
				((struct tree *)list->data)->state =
					final->val - 1;

				invalidated = list_add(invalidated, list->data);
				list = list->next;
			}
		}

		iterator = iterator->next;
	}

	printf("\n---\n");

	iterator = invalidated;
	while (iterator) {
		struct tree *huh = iterator->data;
		printf("%x %d\n", huh->hash, huh->state);
		print_bruijn(tree_to_term(huh));
		printf("\n");
		ref_invalidated_tree(iterator->data);
		struct list *temp = iterator->next;
		free(iterator);
		iterator = temp;
	}

	printf("\n---\n");

	iterator = final;
	while (iterator) {
		struct tree *huh = iterator->data;
		struct term *foo = tree_to_term(huh);
		print_bruijn(foo);
		printf("\n%x\n\n", huh->hash);
		iterator = iterator->next;
	}

	return final;
}
