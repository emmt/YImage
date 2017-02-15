/*
 * itempool.h --
 *
 * Definitions for pools of small items of same size.
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

#ifndef _ITEMPOOL_H
#define _ITEMPOOL_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Opaque structure. */
typedef struct _itempool itempool_t;

/* Public functions. */
extern itempool_t *itempool_new(size_t size, size_t number);
extern void        itempool_destroy(itempool_t *pool);
extern size_t      itempool_get_size(const itempool_t *pool);
extern size_t      itempool_get_number(const itempool_t *pool);
extern void        itempool_set_number(itempool_t *pool, size_t number);
extern void       *itempool_new_item(itempool_t *pool);
extern void        itempool_free_item(itempool_t *pool, void *item);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ITEMPOOL_H */

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
