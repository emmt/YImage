/*
 * itemstack.h --
 *
 * Definitions for stacks of items.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2009-2017 Éric Thiébaut <eric.thiebaut@univ-lyon1.fr>
 *
 * This file is part of YImage.
 *
 * YImage is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * YImage is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * YImage.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ITEMSTACK_H
#define _ITEMSTACK_H 1

#include <stdlib.h>

#define ITEMSTACK_FAILURE (-1)
#define ITEMSTACK_SUCCESS  (0)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exposed structure used to query a stack item. */
typedef struct _itemstack_item itemstack_item_t;
struct _itemstack_item {
  void *data;
  void (*destroy)(void *);
};

/* Opaque structure. */
typedef struct _itemstack_stack itemstack_t;

/* Public functions. */
extern itemstack_t *itemstack_new(long size);
extern void         itemstack_destroy(itemstack_t *stack);
extern int          itemstack_push(itemstack_t *stack, void *data,
                                   void (*destroy)(void *));
extern void        *itemstack_push_dynamic(itemstack_t *stack,
                                           size_t nbytes, int clear);
extern void         itemstack_drop(itemstack_t *stack, long n);
extern int          itemstack_pop(itemstack_t *stack, itemstack_item_t *item);
extern int          itemstack_peek(const itemstack_t *stack, long j,
                                   itemstack_item_t *item);
extern int          itemstack_swap(itemstack_t *stack, long j1, long j2);
extern long         itemstack_get_number(const itemstack_t *stack);
extern long         itemstack_get_size(const itemstack_t *stack);
extern void         itemstack_dump(const itemstack_t *stack, long n,
                                   FILE *file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ITEMSTACK_H */
