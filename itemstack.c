/*
 * itemstack.c --
 *
 * Implementation of stacks of items.
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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "itemstack.h"

struct _itemstack_stack {
  long count, size;
  itemstack_item_t *item;
};

#define OFFSET_OF(TYPE, MEMBER)      ((char *)&((TYPE *)0)->MEMBER - (char *)0)
#define NEW_OBJECT(TYPE)             ((TYPE *)malloc(sizeof(TYPE)))
#define NEW_OBJECT_ZERO(TYPE)        ((TYPE *)calloc(1, sizeof(TYPE)))
#define NEW_ARRAY(TYPE, NUMBER)      ((TYPE *)malloc((NUMBER)*sizeof(TYPE)))
#define NEW_ARRAY_ZERO(TYPE, NUMBER) ((TYPE *)calloc((NUMBER), sizeof(TYPE)))
#define ADDRESS(base, offset)        ((void *)(((char *)(base)) + (offset)))
#define ROUND_UP(a, b)               ((((a) + ((b) - 1))/(b))*(b))
#define MAX(a, b)                    ((a) >= (b) ? (a) : (b))
#define MIN(a, b)                    ((a) <= (b) ? (a) : (b))

/**
 * @brief Creates a new item stack.
 *
 * @param size The initial stack size.  To avoid realloc(), \a size should be
 *             a typical value of the expected stack size.  Whatever is the
 *             value of \a size, the stack is initially empty and will grow as
 *             needed.  Hence \a size = 0 is perfectly legitimate.
 *
 * @return The address of a new item stack object.  In case of error, \c NULL
 *         is returned and \c errno is set.
 *
 * @see itemstack_destroy().
 */
itemstack_t *itemstack_new(long size)
{
  itemstack_t *stack;

  stack = NEW_OBJECT(itemstack_t);
  if (stack != NULL) {
    stack->item = ((size > 0) ? NEW_ARRAY(itemstack_item_t, size) : NULL);
    stack->size = ((stack->item != NULL) ? size : 0);
    stack->count = 0;
  }
  return stack;
}

/**
 * @brief Destroy an item stack.
 *
 * This function frees all the items of the stack (according to their
 * "destroy" method) and then frees the memory used by the stack.
 *
 * @param stack   The address of the item stack.
 */
void itemstack_destroy(itemstack_t *stack)
{
  if (stack != NULL) {
    if (stack->item != NULL) {
      while (stack->count > 0) {
        itemstack_item_t *item = &stack->item[--stack->count];
        if (item->destroy != NULL) {
          item->destroy(item->data);
        }
      }
      free((void *)stack->item);
    }
    free((void *)stack);
  }
}

/**
 * @brief Query the number of items stored in a stack.
 *
 * @param stack The address of the item stack.
 *
 * @return The number of items stored in a stack.  In case of error, \c 0 is
 *         returned and \c errno is set.
 */
long itemstack_get_number(const itemstack_t *stack)
{
  if (stack == NULL) {
    errno = EFAULT;
    return 0;
  }
  return stack->count;
}

/**
 * @brief Query the size of a stack.
 *
 * This function yields the maximum number of items that can be stored by
 * \a stack prior to re-allocating the internal buffer of the stack.
 *
 * @param stack The address of the item stack.
 *
 * @return The size of a stack.  In case of error, \c 0 is returned and
 *         \c errno is set.
 */
long itemstack_get_size(const itemstack_t *stack)
{
  if (stack == NULL) {
    errno = EFAULT;
    return 0;
  }
  return stack->size;
}

/**
 * @brief Push an item on top of an item stack.
 *
 * @param stack   The address of the item stack.
 * @param data    The item address or client data.
 * @param destroy The function to delete the item (can be \c NULL to
 *                do nothing).
 *
 * @return \c ITEMSTACK_SUCCESS on success or \c ITEMSTACK_FAILURE if there is
 *         not enough memory or if \a stack is invalid.
 */
int itemstack_push(itemstack_t *stack, void *data, void (*destroy)(void *))
{
  itemstack_item_t *item;

  if (stack == NULL) {
    errno = EFAULT;
    return ITEMSTACK_FAILURE;
  }
  if (stack->count >= stack->size) {
    /* Realloc the stack buffer.  The new size is ~ 1.4 times the old one (hence
       approximately doubled every two re-allocations) but not less than 4
       elements. */
    long size;
    size_t nbytes;
    size = (7*stack->count + 20)/5;
    nbytes = size*sizeof(itemstack_item_t);
    item = stack->item; /* save the initial address */
    if (item == NULL) {
      stack->item = (itemstack_item_t *)malloc(nbytes);
    } else {
      stack->item = (itemstack_item_t *)realloc(item, nbytes);
    }
    if (stack->item == NULL) {
      stack->item = item; /* restore the initial address */
      return ITEMSTACK_FAILURE;
    }
    stack->size = size;
  }
  item = &stack->item[stack->count];
  item->data = data;
  item->destroy = destroy;
  ++stack->count;
  return ITEMSTACK_SUCCESS;
}

/**
 * @brief Allocate and push a new piece of dynamic memory on top of a stack.
 *
 * This function allocates and push a new piece of dynamic memory on top of
 * \a stack.  Do not use the standard free() function to release the memory
 * but itemstack_drop() or itemstack_destroy().
 *
 * @param stack   The address of the item stack.
 * @param nbytes  The number of bytes to allocate, must be at least one.
 * @param clear   Non-zeor to clear the allocated memory (fill it with zeroes).
 *
 * @return The address of a buffer with \a size bytes, or \c NULL in case of
 *         error (invalid argument(s) or insufficient memory and \c errno set
 *         accordingly).
 *
 * @see itemstack_new, itemstack_push, itemstack_drop, itemstack_destroy.
 */
void *itemstack_push_dynamic(itemstack_t *stack, size_t nbytes, int clear)
{
  void *ptr;

  if (nbytes <= 0) {
    errno = EINVAL;
    return NULL;
  }
  ptr = malloc(nbytes);
  if (ptr == NULL) {
    return NULL;
  }
  if (clear) {
    memset(ptr, 0, nbytes);
  }
  if (itemstack_push(stack, ptr, free) != ITEMSTACK_SUCCESS) {
    free(ptr);
    return NULL;
  }
  return ptr;
}

/**
 * @brief Drop the topmost element(s) of an item stack.
 *
 * This function frees the last item(s) that has been pushed on a stack.  The
 * "destroy" method of the items are used if non-\c NULL.
 *
 * @param stack   The address of the item stack.
 * @param n       The number of elements to drop, \c -1L to drop all elements
 *                (clear the stack).  Whatever is the value of \a n, no more
 *                than the actual number of elements stored into the stack get
 *                dropped.
 */
void itemstack_drop(itemstack_t *stack, long n)
{
  if (stack != NULL) {
    long final_count;
    if ((n == -1) || (n >= stack->count)) {
      final_count = 0;
    } else {
      final_count = stack->count - n;
    }
    while (stack->count > final_count) {
      itemstack_item_t *item = &stack->item[--stack->count];
      if (item->destroy != NULL) {
        item->destroy(item->data);
      }
    }
  }
}

/**
 * @brief Steal the topmost item from the stack.
 *
 * This function steals the topmost item stored in \a stack.  The retrieved
 * item is no longer managed by the stack (which has therefore one less item);
 * in particular it is the caller's responsability to eventually destroy the
 * item contents.
 *
 * @param stack The address of the item stack.
 * @param item  The address where to store the contents of the topmost item
 *              of \a stack.
 *
 * @return A standard result: \c ITEMSTACK_SUCCESS or \c ITEMSTACK_FAILURE
 *         (with \c errno set to \c EFAULT if one of the addresses is invalid,
 *         or to \c EINVAL if the stack is empty; in this latter case,
 *         the members of \a item are set to \c NULL).
 *
 * @see itemstack_drop(), itemstack_peek().
 */
int itemstack_pop(itemstack_t *stack, itemstack_item_t *item)
{
  itemstack_item_t *src;

  if ((stack == NULL) || (item == NULL)) {
    errno = EFAULT;
    return ITEMSTACK_FAILURE;
  }
  if (stack->count < 1) {
    errno = EINVAL;
    item->data = NULL;
    item->destroy = NULL;
    return ITEMSTACK_FAILURE;
  }
  src = &stack->item[--stack->count];
  item->data = src->data;
  item->destroy = src->destroy;
  return ITEMSTACK_SUCCESS;
}

/**
 * @brief Get an item from the stack for examination.
 *
 * This function retrieves an item stored in \a stack.  The index \a j is
 * relative to the top of the stack: 0 is the topmost item, 1 is the previous
 * one, etc.  Beware that the retrieved item is still managed by the stack (in
 * particular for what concerns the destruction of the item).  If you want to
 * take over control of the item, use itemstack_swap() to move it to the top
 * of the stack and then itemstack_pop() to steal it.
 *
 * @param stack The address of the item stack.
 * @param j     The index of the item relative to the top of the stack.
 * @param item  The address where to store the contents of the \a j-th item
 *              from the top of \a stack.
 *
 * @return A standard result: \c ITEMSTACK_SUCCESS or \c ITEMSTACK_FAILURE
 *         (with \c errno set to \c EFAULT if one of the addresses is invalid,
 *         or to \c EINVAL if the index is out of range; in this latter case,
 *         the members of \a item are set to \c NULL).
 *
 * @see itemstack_drop(), itemstack_pop().
 */
int itemstack_peek(const itemstack_t *stack, long j,
                   itemstack_item_t *item)
{
  itemstack_item_t *src;
  long k, top;

  if ((stack == NULL) || (item == NULL)) {
    errno = EFAULT;
    return ITEMSTACK_FAILURE;
  }
  top = stack->count - 1;
  k = top - j;
  if ((j < 0) || (k < 0)) {
    errno = EINVAL;
    item->data = NULL;
    item->destroy = NULL;
    return ITEMSTACK_FAILURE;
  }
  src = &stack->item[k];
  item->data = src->data;
  item->destroy = src->destroy;
  return ITEMSTACK_SUCCESS;
}

/**
 * @brief Swap two items of a stack.
 *
 * This function exchanges the \a j1-th item and the \a j2-th item of
 * \a stack.  Indices are relative to the top of the stack: 0 is the
 * topmost item, 1 is the previous one, etc.
 *
 * @param stack   The address of the item stack.
 * @param j1      The index of the first item to swap relative to the
 *                top of the stack.
 * @param j2      The index of the second item to swap relative to the
 *                top of the stack.
 *
 * @return A standard result: \c ITEMSTACK_SUCCESS or \c ITEMSTACK_FAILURE
 *         (with \c errno set to \c EFAULT if \a stack is an invalid address,
 *         or to \c EINVAL if one of the indices is out of range).
 */
int itemstack_swap(itemstack_t *stack, long j1, long j2)
{
  long k1, k2, top;

  if (stack == NULL) {
    errno = EINVAL;
    return ITEMSTACK_FAILURE;
  }
  top = stack->count - 1;
  k1 = top - j1;
  k2 = top - j2;
  if ((j1 < 0) || (k1 < 0) || (j2 < 0) || (k2 < 0)) {
    errno = EINVAL;
    return ITEMSTACK_FAILURE;
  }
  if (j1 != j2) {
    /* The code below is pathetically intricated to achieve some robustness
       with respect to interrupts. */
    volatile itemstack_item_t *item1 = &stack->item[k1];
    void (*destroy1)(void *) =  item1->destroy;
    void *data1 = item1->data;
    volatile itemstack_item_t *item2 = &stack->item[k2];
    void (*destroy2)(void *) =  item2->destroy;
    void *data2 = item2->data;
    if (destroy1 != destroy2) {
      item1->destroy = NULL;
      item2->destroy = NULL;
    }
    item1->data = data2;
    item2->data = data1;
    if (destroy1 != destroy2) {
      item1->destroy = destroy2;
      item2->destroy = destroy1;
    }
  }
  return ITEMSTACK_SUCCESS;
}

/**
 * @brief Printout the top part of an item stack.
 *
 * @param stack   The address of the item stack.
 * @param n       The number of items to dump.
 * @param file    The output stream.
 */
void itemstack_dump(const itemstack_t *stack, long n, FILE *file)
{
  char data[32], destroy[32];
  itemstack_item_t *item;
  long j, k, top;

  if (stack == NULL) {
    return;
  }
  if (file == NULL) {
    file = stdout;
  }
  if ((n == -1) || (n > stack->count) ) {
    n = stack->count;
  } else if (n <= 0) {
    return;
  }
  top = stack->count - 1;
  for (j = 0; j < n; ++j) {
    k = top - j;
    item = &stack->item[k];
    if (item->data == NULL) {
      strcpy(data, "NULL");
    } else {
      sprintf(data, "0x%lx", (unsigned long)item->data);
    }
    if (item->destroy == NULL) {
      strcpy(destroy, "NULL");
    } else if (item->destroy == free) {
      strcpy(destroy, "free");
    } else {
      sprintf(destroy, "0x%lx", (unsigned long)item->destroy);
    }
    fprintf(file, " stack(%ld) = { data = %s, destroy = %s }\n",
            j, data, destroy);
  }
}
