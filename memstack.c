/*
 * memstack.c --
 *
 * Implementation of stack of dynamic memory blocks.
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
 * $Id$
 * $Log$
 *-----------------------------------------------------------------------------
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
