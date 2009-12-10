/*
 * itempool.h --
 *
 * Definitions for pools of small items of same size.
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
