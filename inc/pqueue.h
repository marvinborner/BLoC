/*
 * Copyright (c) 2014, Volkan Yazıcı <volkan.yazici@gmail.com>
 * Copyright (c) 2014, Marvin Borner <dev@marvinborner.de>
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

/**
 * @file  pqueue.h
 * @brief Priority Queue function declarations
 *
 * @{
 */

#ifndef PQUEUE_H
#define PQUEUE_H

#include <stddef.h>
#include <stdio.h>

/** priority data type */
typedef unsigned long long pqueue_pri_t;

/** callback functions to get/set/compare the priority of an element */
typedef pqueue_pri_t (*pqueue_get_pri_f)(void *a);
typedef int (*pqueue_cmp_pri_f)(pqueue_pri_t next, pqueue_pri_t curr);

/** callback functions to get/set the position of an element */
typedef size_t (*pqueue_get_pos_f)(void *a);
typedef void (*pqueue_set_pos_f)(void *a, size_t pos);

/** debug callback function to print a entry */
typedef void (*pqueue_print_entry_f)(FILE *out, void *a);

/** the priority queue handle */
struct pqueue {
	size_t size; /**< number of elements in this queue */
	size_t avail; /**< slots available in this queue */
	size_t step; /**< growth stepping setting */
	pqueue_cmp_pri_f cmppri; /**< callback to compare nodes */
	pqueue_get_pri_f getpri; /**< callback to get priority of a node */
	pqueue_set_pos_f setpos; /**< callback to set position of a node */
	void **d; /**< The actualy queue in binary heap form */
};

/**
 * initialize the queue
 *
 * @param n the initial estimate of the number of queue items for which memory
 *     should be preallocated
 * @param cmppri The callback function to run to compare two elements
 *     This callback should return 0 for 'lower' and non-zero
 *     for 'higher', or vice versa if reverse priority is desired
 * @param getpri the callback function to run to set a score to an element
 *
 * @return the handle or NULL for insufficent memory
 */
struct pqueue *pqueue_init(size_t n, pqueue_cmp_pri_f cmppri,
			   pqueue_get_pri_f getpri, pqueue_set_pos_f set_pos);

/**
 * free all memory used by the queue
 * @param q the queue
 */
void pqueue_free(struct pqueue *q);

/**
 * return the size of the queue.
 * @param q the queue
 */
size_t pqueue_size(struct pqueue *q);

/**
 * insert an item into the queue.
 * @param q the queue
 * @param d the item
 * @return 0 on success
 */
int pqueue_insert(struct pqueue *q, void *d);

/**
 * pop the highest-ranking item from the queue.
 * @param q the queue
 * @return NULL on error, otherwise the entry
 */
void *pqueue_pop(struct pqueue *q);

#endif
