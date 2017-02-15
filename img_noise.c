/*
 * img_noise.c --
 *
 * Estimation of the noise level in an image.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2009-2013 Éric Thiébaut <eric.thiebaut@univ-lyon1.fr>
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

#ifndef _IMG_NOISE_C
#define _IMG_NOISE_C 1

#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "img.h"


/* Definitions of public functions. */

extern double img_estimate_noise(const int type,
                                 const void *img,
                                 const long offset,
                                 const long width,
                                 const long height,
                                 const long stride,
                                 const int method);


/* Definitions that will be expanded by the template code. */

#define ESTIMATE_NOISE(TYPE)  CPT_JOIN(img_estimate_noise_,CPT_ABBREV(TYPE))

#define pixel_t               CPT_CTYPE(TYPE)


/* Manage to include this file with a different data type each time. */

#ifdef IMG_TYPE_INT8
# define TYPE INT8
# include __FILE__
#endif

#ifdef IMG_TYPE_UINT8
# define TYPE UINT8
# include __FILE__
#endif

#ifdef IMG_TYPE_INT16
# define TYPE INT16
# include __FILE__
#endif

#ifdef IMG_TYPE_UINT16
# define TYPE UINT16
# include __FILE__
#endif

#ifdef IMG_TYPE_INT32
# define TYPE INT32
# include __FILE__
#endif

#ifdef IMG_TYPE_UINT32
# define TYPE UINT32
# include __FILE__
#endif

#ifdef IMG_TYPE_INT64
# define TYPE INT64
# include __FILE__
#endif

#ifdef IMG_TYPE_UINT64
# define TYPE UINT64
# include __FILE__
#endif

#ifdef IMG_TYPE_FLOAT
# define TYPE FLOAT
# include __FILE__
#endif

#ifdef IMG_TYPE_DOUBLE
# define TYPE DOUBLE
# include __FILE__
#endif

/*
#ifdef IMG_TYPE_SCOMPLEX
# define TYPE SCOMPLEX
# include __FILE__
#endif

#ifdef IMG_TYPE_DCOMPLEX
# define TYPE DCOMPLEX
# include __FILE__
#endif

#ifdef IMG_TYPE_RGB
# define TYPE RGB
# include __FILE__
#endif

#ifdef IMG_TYPE_RGBA
# define TYPE RGBA
# include __FILE__
#endif
*/

/**
 * @brief Estimate the noise level in a sub-image.
 *
 * This function estimates the noise level in a rectangular ROI (region of
 * interest) of image \a img.
 *
 * @param type        The type identifier of the input image \a img.
 * @param img         The input image.
 * @param offset      The offset (in pixels w.r.t. \a img) of the first pixel
 *                    of the ROI.
 * @param width       The width of the ROI.
 * @param height      The height of the ROI.
 * @param pitch       The number of elements per row of \a img.
 * @param method      The method used to estimate the noise level (irrelevant
 *                    for the moment).
 *
 * @return The estimated noise level; -1.0 on error.
 */
extern double img_estimate_noise(const int type, const void *img,
                                 const long offset, const long width,
                                 const long height, const long stride,
                                 const int method)
{
  double result;

  if (img == NULL) {
    errno = EFAULT;
    return -1.0;
  }
  if ((width < 1) || (height < 1) || (stride < width)) {
    errno = EINVAL;
    return -1.0;
  }

#define CASE(TYPE) case IMG_TYPE_##TYPE:                                 \
    result = ESTIMATE_NOISE(TYPE)((const CPT_CTYPE(TYPE) *)img, offset, \
                                  width, height, stride, method);        \
  break

  switch (type) {
#ifdef IMG_TYPE_INT8
    CASE(INT8);
#endif
#ifdef IMG_TYPE_UINT8
    CASE(UINT8);
#endif
#ifdef IMG_TYPE_INT16
    CASE(INT16);
#endif
#ifdef IMG_TYPE_UINT16
    CASE(UINT16);
#endif
#ifdef IMG_TYPE_INT32
    CASE(INT32);
#endif
#ifdef IMG_TYPE_UINT32
    CASE(UINT32);
#endif
#ifdef IMG_TYPE_INT64
    CASE(INT64);
#endif
#ifdef IMG_TYPE_UINT64
    CASE(UINT64);
#endif
#ifdef IMG_TYPE_FLOAT
    CASE(FLOAT);
#endif
#ifdef IMG_TYPE_DOUBLE
    CASE(DOUBLE);
#endif
    /*
#ifdef IMG_TYPE_SCOMPLEX
    CASE(SCOMPLEX);
#endif
#ifdef IMG_TYPE_DCOMPLEX
    CASE(DCOMPLEX);
#endif
#ifdef IMG_TYPE_RGB
    CASE(RGB);
#endif
#ifdef IMG_TYPE_RGBA
    CASE(RGBA);
#endif
    */
  default:
    /* Bad pixel type. */
    errno = EINVAL;
    result = -1.0;
  }

#undef CASE

  return result;
}

#else /* _IMG_NOISE_C ********************************************************/

static double ESTIMATE_NOISE(TYPE)(const pixel_t *img,
                                   const long offset,
                                   const long width,
                                   const long height,
                                   const long stride,
                                   const int method)
{
  const pixel_t *row0, *row1;
  double a00, a01, a10, a11, r, s;
  long x, y;

  s = 0.0;
  row1 = img + offset;
  for (y = 1; y < height; ++y) {
    row0 = row1;
    row1 += stride;
    a01 = row0[0];
    a11 = row1[0];
    for (x = 1; x < width; ++x) {
      a00 = a01;
      a01 = row0[x];
      a10 = a11;
      a11 = row1[x];
      r = a00 - a01 - a10 + a11;
      s += r*r;
    }
  }
  return sqrt(s/(4.0*width*height));
}

/* Undefine macro(s) that may be re-defined to avoid warnings. */
#undef TYPE

#endif /* _IMG_NOISE_C defined ***********************************************/

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * fill-column: 78
 * coding: utf-8
 * End:
 */
