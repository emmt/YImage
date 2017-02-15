/*
 * memstack.c --
 *
 * Implementation of stack of dynamic memory blocks.
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
#ifndef NDEBUG
# include <stdio.h>
#endif

#include "memstack.h"

#define ADDRESS(base, offset) ((void *)(((char *)(base)) + (offset)))
#define ROUND_UP(a, b)        ((((a) + ((b) - 1))/(b))*(b))
#define MAX(a, b)             ((a) >= (b) ? (a) : (b))

static const size_t ALIGN = MAX(sizeof(double), sizeof(void *));

#define STACK_MAGIC 0xDEADC0DE

/**
 * @brief Initialize a memory stack.
 *
 * This function set the value pointed by \a stack to be \c NULL.
 *
 * @param stack   The address of a persistent pointer to store the stack.
 */
void memstack_init(void **stack)
{
  if (stack != NULL) {
    *stack = NULL;
  }
}

/**
 * @brief Allocate a new fragment of memory in a stack.
 *
 * This function push a new fragment of memory on a stack.  Do not use the
 * standard \c free() function to release the memory but \c memstack_drop() or
 * \c memstack_destroy().
 *
 * @param stack   The address of a persistent pointer to store the stack.
 * @param size    The number of bytes to allocate.
 *
 * @return The address of a buffer with \a size bytes, or \c NULL in case of
 *         error (invalid argument(s) or insufficient memory and \c errno set
 *         accordingly).
 *
 * @see memstack_push_zero, memstack_drop, memstack_destroy.
 */
void *memstack_push(void **stack, size_t size)
{
  size_t offset, nbytes;
  void *ptr;

  if (stack == NULL) {
    errno = EFAULT;
    return NULL;
  }
  if (size <= 0) {
    errno = EINVAL;
    return NULL;
  }
  offset = ROUND_UP(2*sizeof(void *), ALIGN);
  nbytes = offset + ROUND_UP(size, ALIGN);
  ptr = malloc(nbytes);
  if (ptr == NULL) {
    return NULL;
  }
  ((void **)ptr)[0] = stack;
  ((void **)ptr)[1] = (void *)STACK_MAGIC;
  *stack = ptr;
  return ADDRESS(ptr, offset);
}

/**
 * @brief Allocate a new fragment of memory in a stack and fill it with zeroes.
 *
 * This function push a new fragment of memory on a stack and fill it with
 * zeroes.  Do not use the standard \c free() function to release the memory
 * but \c memstack_drop() or \c memstack_destroy().
 *
 * @param stack   The address of a persistent pointer to store the stack.
 * @param size    The number of bytes to allocate.
 *
 * @return The address of a buffer with \a size bytes, or \c NULL in case of
 *         error (invalid argument(s) or insufficient memory and \c errno set
 *         accordingly).
 *
 * @see memstack_push, memstack_drop, memstack_destroy.
 */
void *memstack_push_zero(void **stack, size_t size)
{
  void *ptr;

  ptr = memstack_push(stack, size);
  if (ptr != NULL) {
    memset(ptr, 0, size);
  }
  return ptr;
}

/**
 * @brief Drop the topmost element of a memory stack.
 *
 * This function frees the last fragment of memory that has been pushed on a
 * stack.
 *
 * @param stack   The address of a persistent pointer to store the stack.
 */
void memstack_drop(void **stack)
{
  void *ptr;

  if (stack != NULL) {
    if ((ptr = *stack) != NULL) {
#ifndef NDEBUG
      if (((void **)ptr)[1] != (void *)STACK_MAGIC) {
        fprintf(stderr, "corrupted memstack (%s, %d)\n",
                __FILE__, __LINE__);
        return;
      }
#endif
      *stack = ((void **)ptr)[0];
      free(ptr);
    }
  }
}

/**
 * @brief Free all memory allocated on a stack.
 *
 * This function frees all the fragment of memory that have been pushed on a
 * stack.
 *
 * @param stack   The address of a persistent pointer to store the stack.
 */
void memstack_clear(void **stack)
{
  void *ptr;

  if (stack != NULL) {
    while ((ptr = *stack) != NULL) {
#ifndef NDEBUG
      if (((void **)ptr)[1] != (void *)STACK_MAGIC) {
        fprintf(stderr, "corrupted memstack (%s, %d)\n",
                __FILE__, __LINE__);
        return;
      }
#endif
      *stack = ((void **)ptr)[0];
      free(ptr);
    }
  }
}
