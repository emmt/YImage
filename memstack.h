/*
 * memstack.h --
 *
 * Definitions for stacks of dynamic memory.
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

#ifndef _MEMSTACK_H
#define _MEMSTACK_H 1

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Public functions. */
extern void  memstack_init(void **stack);
extern void *memstack_push(void **stack, size_t size);
extern void *memstack_push_zero(void **stack, size_t size);
extern void  memstack_drop(void **stack);
extern void  memstack_clear(void **stack);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MEMSTACK_H */

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
