/*
 * img_watershed.c --
 *
 * Implement watershed segmentation.
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

#ifndef _WATERSHED_C
#define _WATERSHED_C 1

#include <string.h>
#include <stdlib.h>
#include <yapi.h>

#define IGNORE   -1
#define UNKNOWN   0

#define _JOIN(a, b)   a##b
#define JOIN(a, b)    _JOIN(a, b)

#define TYPE unsigned char
#define SFX  c
#include __FILE__

#define TYPE short
#define SFX  s
#include __FILE__

#define TYPE int
#define SFX  i
#include __FILE__

#define TYPE long
#define SFX  l
#include __FILE__

#define TYPE float
#define SFX  f
#include __FILE__

#define TYPE double
#define SFX  d
#include __FILE__

void Y_img_watershed(int argc)
{
  long dstDims[Y_DIMSIZE];
  long srcDims[Y_DIMSIZE];
  void* src;
  void* dst;
  long* idx;
  long i, rank, srcNumber, dstNumber;
  int srcType, dstType;

  if (argc != 2) {
    y_error("expecting exactly 2 arguments");
    return;
  }
  src = ygeta_any(0, &srcNumber, srcDims, &srcType);
  if ((rank = srcDims[0]) != 2 || srcType > Y_DOUBLE) {
    y_error("expecting a 2D real array");
    return;
  }
  dst = ygeta_any(1, &dstNumber, dstDims, &dstType);
  if (dstType != Y_LONG) {
    y_error("destination must be of type long");
    return;
  }
  for (i = 0; i <= rank; ++i) {
    if (dstDims[i] != srcDims[i]) {
      y_error("destination must have same dimensions as source");
      return;
    }
  }
  if (! yarg_subroutine() && ! yarg_scratch(1)) {
    /* When called as a function, make sure the label array can be re-used.
       Otherwise, make a copy. */
    void* tmp = ypush_l(dstDims);
    memcpy(tmp, dst, dstNumber*sizeof(long));
    yarg_swap(0, 2);
    yarg_drop(1);
    dst = tmp;
  }
  idx = ypush_l(srcDims);

  if (srcType == Y_CHAR) {
    watershed_c(dst, src, srcDims[1], srcDims[2], idx);
  } else if (srcType == Y_SHORT) {
    watershed_s(dst, src, srcDims[1], srcDims[2], idx);
  } else if (srcType == Y_INT) {
    watershed_i(dst, src, srcDims[1], srcDims[2], idx);
  } else if (srcType == Y_LONG) {
    watershed_l(dst, src, srcDims[1], srcDims[2], idx);
  } else if (srcType == Y_FLOAT) {
    watershed_f(dst, src, srcDims[1], srcDims[2], idx);
  } else if (srcType == Y_DOUBLE) {
    watershed_d(dst, src, srcDims[1], srcDims[2], idx);
  } else {
    y_error("expecting a 2D real array");
    return;
  }

  /* Left the result on top of the stack. */
  yarg_drop(2);
}

#else /* _WATERSHED_C defined */

#define HEAPSORT   JOIN(heapsort_,SFX)
#define PROPAGATE  JOIN(propagate_,SFX)
#define WATERSHED  JOIN(watershed_,SFX)

/* HEAPSORT - indirect sorting of an array, with C-indexing (starting at 0) */
static void
HEAPSORT(long index[], const TYPE a[], const long n)
{
  long i, j, k, l, isave;
  TYPE asave;

  for (i = 0; i < n; ++i) {
    index[i] = i;
  }
  if (n < 2) {
    return;
  }
  k = n/2;
  l = n - 1;
  for (;;) {
    if (k > 0) {
      isave = index[--k];
    } else {
      isave = index[l];
      index[l] = index[0];
      if (--l == 0) {
        index[0] = isave;
        return;
      }
    }
    asave = a[isave];
    i = k;
    while ((j = 2*i + 1) <= l) {
      if (j < l && a[index[j]] < a[index[j + 1]]) ++j;
      if (a[index[j]] <= asave) break;
      index[i] = index[j];
      i = j;
    }
    index[i] = isave;
  }
}

static void
PROPAGATE(long lab[], const TYPE arr[], TYPE lvl,
          long i, long i1, long i2, long k, long n1, long n2)
{
  lab[i] = k;
#define CHECK(test, off, i1, i2)                        \
  if (test) {                                           \
    long j = off;                                       \
    if (lab[j] == UNKNOWN && arr[j] <= lvl) {           \
      PROPAGATE(lab, arr, lvl, j, i1, i2, k, n1, n2);   \
    }                                                   \
  }
  CHECK( i2 > 0    , i-n1 , i1   , i2-1 );
  CHECK( i1 > 0    , i-1  , i1-1 , i2   );
  CHECK( i1 < n1-1 , i+1  , i1+1 , i2   );
  CHECK( i2 < n2-1 , i+n1 , i1   , i2+1 );
#undef CHECK
}

static void
WATERSHED(long lab[], const TYPE arr[], long n1, long n2, long idx[])
{
  long i1, i2, i, j, k, kp, n;

  n = n1*n2;
  HEAPSORT(idx, arr, n);
  for (j = 0; j < n; ++j) {
    i = idx[j];
    if (lab[i] != UNKNOWN) {
      continue;
    }
    i2 = i/n1;
    i1 = i - i2*n1;
    k = UNKNOWN;
#define UPDATE(test, ip)					\
    if ((test) && (kp = lab[ip]) > UNKNOWN && kp != k) {	\
      if (k == UNKNOWN) {					\
	k = kp;							\
      } else {							\
	lab[i] = IGNORE;					\
	continue;						\
      }								\
    }
    UPDATE( i2 > 0    , i-n1 );
    UPDATE( i1 > 0    , i-1  );
    UPDATE( i1 < n1-1 , i+1  );
    UPDATE( i2 < n2-1 , i+n1 );
#undef UPDATE
    if (k > UNKNOWN) {
#if 0
      lab[i] = k;
#else
      PROPAGATE(lab, arr, arr[i], i, i1, i2, k, n1, n2);
#endif
    }
  }
}

#undef TYPE
#undef WATERSHED
#undef PROPAGATE
#undef HEAPSORT
#undef SFX

#endif /* _WATERSHED_C */
