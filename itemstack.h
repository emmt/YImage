/*
 * itemstack.h --
 *
 * Definitions for stacks of items.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 Eric Thiébaut <thiebaut@obs.univ-lyon1.fr>
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can use, modify
 * and/ or redistribute the software under the terms of the CeCILL-C license
 * as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited liability.
 *
 * In this respect, the user's attention is drawn to the risks associated with
 * loading, using, modifying and/or developing or reproducing the software by
 * the user in light of its specific status of free software, that may mean
 * that it is complicated to manipulate, and that also therefore means that it
 * is reserved for developers and experienced professionals having in-depth
 * computer knowledge. Users are therefore encouraged to load and test the
 * software's suitability as regards their requirements in conditions enabling
 * the security of their systems and/or data to be ensured and, more
 * generally, to use and operate it in the same conditions as regards
 * security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 *
 *-----------------------------------------------------------------------------
 *
 * $Id: itemstack.h,v 1.1 2009/12/10 08:53:26 eric Exp $
 * $Log: itemstack.h,v $
 * Revision 1.1  2009/12/10 08:53:26  eric
 * Initial revision
 *
 *-----------------------------------------------------------------------------
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

/*
 * Local Variables:
 * coding: utf-8
 * mode: C
 * tab-width: 8
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * fill-column: 78
 * End:
 */
