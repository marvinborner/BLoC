/*
 * Copyright (c) 2014, Volkan Yazıcı <volkan.yazici@gmail.com>
 * Copyright (c) 2023, Marvin Borner <dev@marvinborner.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pqueue.h>

#define left(i) ((i) << 1)
#define right(i) (((i) << 1) + 1)
#define parent(i) ((i) >> 1)

struct pqueue *pqueue_init(size_t n, pqueue_cmp_pri_f cmppri,
			   pqueue_get_pri_f getpri, pqueue_set_pos_f setpos)
{
	struct pqueue *q;

	if (!(q = malloc(sizeof(struct pqueue))))
		return NULL;

	/* Need to allocate n+1 elements since element 0 isn't used. */
	if (!(q->d = malloc((n + 1) * sizeof(void *)))) {
		free(q);
		return NULL;
	}

	q->size = 1;
	q->avail = q->step = (n + 1); /* see comment above about n+1 */
	q->cmppri = cmppri;
	q->getpri = getpri;
	q->setpos = setpos;

	return q;
}

void pqueue_free(struct pqueue *q)
{
	free(q->d);
	free(q);
}

size_t pqueue_size(struct pqueue *q)
{
	/* queue element 0 exists but doesn't count since it isn't used. */
	return (q->size - 1);
}

static void bubble_up(struct pqueue *q, size_t i)
{
	size_t parent_node;
	void *moving_node = q->d[i];
	pqueue_pri_t moving_pri = q->getpri(moving_node);

	for (parent_node = parent(i);
	     ((i > 1) && q->cmppri(q->getpri(q->d[parent_node]), moving_pri));
	     i = parent_node, parent_node = parent(i)) {
		q->d[i] = q->d[parent_node];
		q->setpos(q->d[i], i);
	}

	q->d[i] = moving_node;
	q->setpos(moving_node, i);
}

static size_t maxchild(struct pqueue *q, size_t i)
{
	size_t child_node = left(i);

	if (child_node >= q->size)
		return 0;

	if ((child_node + 1) < q->size &&
	    q->cmppri(q->getpri(q->d[child_node]),
		      q->getpri(q->d[child_node + 1])))
		child_node++; /* use right child instead of left */

	return child_node;
}

static void percolate_down(struct pqueue *q, size_t i)
{
	size_t child_node;
	void *moving_node = q->d[i];
	pqueue_pri_t moving_pri = q->getpri(moving_node);

	while ((child_node = maxchild(q, i)) &&
	       q->cmppri(moving_pri, q->getpri(q->d[child_node]))) {
		q->d[i] = q->d[child_node];
		q->setpos(q->d[i], i);
		i = child_node;
	}

	q->d[i] = moving_node;
	q->setpos(moving_node, i);
}

int pqueue_insert(struct pqueue *q, void *d)
{
	void *tmp;
	size_t i;
	size_t newsize;

	if (!q)
		return 1;

	/* allocate more memory if necessary */
	if (q->size >= q->avail) {
		newsize = q->size + q->step;
		if (!(tmp = realloc(q->d, sizeof(void *) * newsize)))
			return 1;
		q->d = tmp;
		q->avail = newsize;
	}

	/* insert item */
	i = q->size++;
	q->d[i] = d;
	bubble_up(q, i);

	return 0;
}

void *pqueue_pop(struct pqueue *q)
{
	void *head;

	if (!q || q->size == 1)
		return NULL;

	head = q->d[1];
	q->d[1] = q->d[--q->size];
	percolate_down(q, 1);

	return head;
}
