/*
 * heapsort.h --
 *
 * Implements in-place sorting of an array of values or objects by heap-sort
 * algorithm.  Adapted from "Numerical Recipes in C" (W.H. Press,
 * B.P. Flannery, S.A. Teukolsky and W.T. Vetterling, Cambridge University
 * Press).
 *
 * Two different kinds of routines are obtained by defining some macros prior
 * to include this file.  These macros are:
 *
 *   HEAPSORT_SCOPE        - scope of the heap-sort routine (optional,
 *                           "static" by default);
 *   HEAPSORT_FUNCTION     - name of the heap-sort routine (must be defined);
 *   HEAPSORT_OBJ_TYPE     - data type of objects to sort (must be defined);
 *   HEAPSORT_KEY_TYPE     - data type of sort key (optional, see below);
 *   HEAPSORT_GET_KEY(obj) - macro to get the sorting key of object OBJ
 *                           (optional, see below).
 *
 * The prototype of the function will be:
 *
 *   HEAPSORT_SCOPE void HEAPSORT_FUNCTION(HEAPSORT_OBJ_TYPE obj[], size_t n);
 *
 * To sort an array of values, only HEAPSORT_FUNCTION and HEAPSORT_OBJ_TYPE
 * have to be defined.  For instance, the following preprocessor instructions
 * will insert the code of a private (static) routine "sort_long" which
 * performs in-place sorting of an array of long integers:
 *
 *   #define HEAPSORT_SCOPE      static
 *   #define HEAPSORT_FUNCTION   sort_long
 *   #define HEAPSORT_OBJ_TYPE   long
 *   #include "heapsort.h"
 *
 * To sort an array of arbitrarily complex objects, the macros
 * HEAPSORT_KEY_TYPE and HEAPSORT_GET_KEY must be also defined.  For instance,
 * the following preprocessor instructions will insert the code of a public
 * routine "sort_foo" which performs in-place sorting of an array of objects
 * of type foo_t according to the value of their "weight" member:
 *
 *   #define HEAPSORT_SCOPE         extern "C"
 *   #define HEAPSORT_FUNCTION      sort_foo
 *   #define HEAPSORT_OBJ_TYPE      foo_t *
 *   #define HEAPSORT_KEY_TYPE      double
 *   #define HEAPSORT_GET_KEY(obj)  (obj)->weight
 *   #include "heapsort.h"
 *
 * The following example is for sorting an array of pointer to objects of type
 * foo_t (note the differences with the previous example):
 *
 *   #define HEAPSORT_SCOPE         extern "C"
 *   #define HEAPSORT_FUNCTION      sort_foo
 *   #define HEAPSORT_OBJ_TYPE      foo_t
 *   #define HEAPSORT_KEY_TYPE      double
 *   #define HEAPSORT_GET_KEY(obj)  (obj).weight
 *   #include "heapsort.h"
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

#ifndef HEAPSORT_SCOPE
# define HEAPSORT_SCOPE static
#endif

/*
#ifndef HEAPSORT_INDEX_TYPE
# define HEAPSORT_INDEX_TYPE size_t
#endif
*/

#ifndef HEAPSORT_FUNCTION
# error Macro HEAPSORT_FUNCTION must be defined.
#endif

#ifndef HEAPSORT_OBJ_TYPE
# error Macro HEAPSORT_OBJ_TYPE must be defined.
#endif

#if defined(HEAPSORT_KEY_TYPE) && defined(HEAPSORT_GET_KEY)
# define _HEAPSORT_KEY(a)   HEAPSORT_GET_KEY(obj[a])
#elif defined(HEAPSORT_KEY_TYPE) || defined(HEAPSORT_GET_KEY)
# error Macros HEAPSORT_GET_KEY and HEAPSORT_KEY_TYPE must be defined together.
#else
# define _HEAPSORT_KEY(a)   (obj[a])
#endif

HEAPSORT_SCOPE void HEAPSORT_FUNCTION(HEAPSORT_OBJ_TYPE obj[], size_t n);

HEAPSORT_SCOPE void HEAPSORT_FUNCTION(HEAPSORT_OBJ_TYPE obj[], size_t n)
{
  size_t i, j, k, l;
  HEAPSORT_OBJ_TYPE tmp;
#ifdef HEAPSORT_GET_KEY
  HEAPSORT_KEY_TYPE key;
#endif

  if (n < 2) return;
  k = n/2;
  l = n - 1;
  for (;;) {
    if (k > 0) {
      tmp = obj[--k];
    } else {
      tmp = obj[l];
      obj[l] = obj[0];
      if (--l == 0) {
        obj[0] = tmp;
        return;
      }
    }
#ifdef HEAPSORT_GET_KEY
    key = HEAPSORT_GET_KEY(tmp);
#endif
    i = k;
    while ((j = 2*i + 1) <= l) {
      if (j < l && _HEAPSORT_KEY(j) < _HEAPSORT_KEY(j + 1)) ++j;
#ifdef HEAPSORT_GET_KEY
      if (_HEAPSORT_KEY(j) <= key) break;
#else
      if (_HEAPSORT_KEY(j) <= tmp) break;
#endif
      obj[i] = obj[j];
      i = j;
    }
    obj[i] = tmp;
  }
}

/* Undefined all specific macros so that this file can be included multiple
   times. */
#undef _HEAPSORT_KEY
#undef  HEAPSORT_KEY_TYPE
#undef  HEAPSORT_OBJ_TYPE
#undef  HEAPSORT_FUNCTION
#undef  HEAPSORT_SCOPE
#undef  HEAPSORT_GET_KEY
