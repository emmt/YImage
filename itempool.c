/*
 * itempool.c --
 *
 * Implementation of pools of items to allocate lots of small items by chunk
 * of larger blocks of memory.  All the items managed by a given pool have the
 * same size.
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
#include <stdlib.h>

#include "itempool.h"

#define ADDRESS(base, offset) ((void *)(((char *)(base)) + (offset)))
#define ROUND_UP(a, b)        ((((a) + ((b) - 1))/(b))*(b))
#define MAX(a, b)             ((a) >= (b) ? (a) : (b))

static const size_t ALIGN = MAX(sizeof(double), sizeof(void *));

struct _itempool {
  size_t number; /* number of items per block of memory */
  size_t size;   /* size of an item */
  void *block;   /* last fragment of memory allocated */
  void *item;    /* first unused item, NULL if none */
};

static void insert_fragment(itempool_t *pool, void *block,
                            size_t offset, size_t size, size_t stride);

/**
 * @brief Create a new item-pool.
 *
 * @param size     The size of the items that will be managed by the
 *                 item-pool.
 * @param number   The number of items per fragment of memory allocated by
 *                 the item-pool.
 *
 * @return The address of the new item-pool; \c NULL in case of error (invalid
 *         argument(s) or insufficient memory and \c errno set accordingly).
 */
itempool_t *itempool_new(size_t size, size_t number)
{
  size_t stride, block_size, offset;
  itempool_t *pool;

  if (size < 1 || number < 1) {
    errno = EINVAL;
    return NULL;
  }

  /* To avoid memory fragmentation, we allocate a memory fragment large
     enough to store the pool itself and the first block of items. */
  stride = ROUND_UP(size, ALIGN);
  offset = ROUND_UP(sizeof(itempool_t), ALIGN);
  block_size = offset + stride*number;
  pool = (itempool_t *)malloc(block_size);
  if (pool != NULL) {
    pool->number = number;
    pool->size = size;
    pool->block = NULL;
    pool->item = NULL;
    insert_fragment(pool, (void *)pool, offset, block_size, stride);
  }
  return pool;
}

/**
 * @brief Insert a fragment of memory in the item pool.
 *
 * The \a block can be the address of \a pool for the first block.
 *
 * @param pool     The address of the item pool.
 * @param block    The block of memory to insert.
 * @param offset   The offset (in bytes) of the first item in the block.
 * @param size     The size (in bytes) of an item.
 * @param stride   The spacing (in bytes) between items.
 */
static void insert_fragment(itempool_t *pool, void *block,
                            size_t offset, size_t size, size_t stride)
{
  void *first, *item;

  /* Initialize the chain of unused items. */
  first = ADDRESS(block, offset);
  item = first;
  while ((offset += stride) < size) {
    void *next = ADDRESS(block, offset);
    *(void **)item = next;
    item = next;
  }
  *(void **)item = pool->item; /* mark last one */
  pool->item = first;

  /* Attach memory fragment and unused items to the pool. */
  if (block != (void *)pool) {
    /* Register the fragment into the pool, remembering previous one. */
    *(void **)block = pool->block;
    pool->block = block;
  }
}

/**
 * @brief Destroy an item-pool.
 *
 * This function releases all ressources allocated for an item-pool.  All
 * items managed by the item-pool and the item-pool itself must not be used
 * after calling this function.
 *
 * @param pool The address of the item-pool (can be \c NULL in which case
 *             nothing happens).
 */
void itempool_destroy(itempool_t *pool)
{
  if (pool != NULL) {
    /* Free all fragments of memory allocated by the pool
       and, then, the pool itself. */
    void *block = pool->block;
    while (block != NULL) {
      void *next = *(void **)block;
      free(block);
      block = next;
    }
    free((void *)pool);
  }
}

/**
 * @brief Get the size of a single item in an item-pool.
 *
 * @param pool   The address of the item-pool.
 *
 * @return The item size; 0 in case of error.
 */
size_t itempool_get_size(const itempool_t *pool)
{
  return (pool != NULL ? pool->size : 0);
}

/**
 * @brief Get the number of items per fragment of memory in an item-pool.
 *
 * @param pool   The address of the item-pool.
 *
 * @return The number of items allocated per fragment of memory; 0 in case of
 *         error.
 */
size_t itempool_get_number(const itempool_t *pool)
{
  return (pool != NULL ? pool->number : 0);
}

/**
 * @brief Set the number of items per fragment of memory in an item-pool.
 *
 * This function changes the number of items per fragment of memory that will
 * be allocated by the item-pool.  The change will be effective the next time
 * a new fragment of memry is required.  Nothing is done if the arguments are
 * invalid.
 *
 * @param pool   The address of the item-pool.
 * @param size   The new number of items per fragment of memory.
 */
void itempool_set_number(itempool_t *pool, size_t number)
{
  if (pool != NULL && number >= 1) {
    pool->number = number;
  }
}

/**
 * @brief Get a new item from a pool.
 *
 * This function returns the address of the first unused item in the pool.
 * If there is no unused items, a new fragment of memory is allocated for
 * the pool.
 *
 * @param pool The address of the item-pool.
 *
 * @return The address of a new item; \c NULL in case of error (invalid
 *         argument(s) or insufficient memory and \c errno set accordingly).
 */
void *itempool_new_item(itempool_t *pool)
{
  void *item;

  if (pool == NULL) {
    errno = EFAULT;
    return NULL;
  }
  if (pool->item == NULL) {
    /* Allocate a new fragment of memory. */
    size_t number, stride, offset, size;
    void *block;
    number = pool->number;
    stride = ROUND_UP(pool->size, ALIGN);
    offset = ALIGN;
    size = offset + number*stride;
    block = malloc(size);
    if (block == NULL) {
      return NULL;
    }
    insert_fragment(pool, block, offset, size, stride);
  }

  /* Remove next unused item and return its address. */
  item = pool->item;
  pool->item = *(void **)item;
  return item;
}

/**
 * @brief Return an item to its pool.
 *
 * This function returns an item to its pool.  The item is inserted in the
 * list of free items in the pool.  The item must not be used after calling
 * this function.  The item must belongs to the pool.  Nothing is done if one
 * or both of the arguments are \c NULL.
 *
 * @param pool   The address of the item-pool.
 * @param item   The address of the item to return to the pool.
 */
void itempool_free_item(itempool_t *pool, void *item)
{
  if ((item != NULL) && (pool != NULL)) {
    *(void **)item = pool->item;
    pool->item = item;
  }
}
