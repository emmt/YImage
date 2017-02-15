/*
 * img_cost.c --
 *
 * Implementation of image cost functions.
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
 */

#ifndef _IMG_COST_C
#define _IMG_COST_C 1

#include <stdlib.h>
#include <errno.h>

#include "img.h"

#ifndef NULL
# define NULL ((void*)0L)
#endif


/* Definitions that will be expanded by the template code. */

#define COST_L2(TYPE)  CPT_JOIN(img_cost_l2_,CPT_ABBREV(TYPE))

#define pixel_t        CPT_CTYPE(TYPE)


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
 * @brief Compute quadratic difference between two sub-images.
 *
 * This function computes the total quadratic difference between two
 * sub-images: a "raw" one and a "reference" one.  The two sub-images may be
 * parts of larger images (or arrays).
 *
 * @param type        The type identifier of the images \a raw and \a ref.
 * @param raw_image   Base address of the raw image.
 * @param raw_offset  Offset (in number of pixels with respect to \a raw_image)
 *                    of the first pixel of the raw sub-image.
 * @param raw_width   Width of raw sub-image.
 * @param raw_height  Height of raw sub-image.
 * @param raw_stride  Elements per row of \a ref_image.
 * @param ref_image   Base address of the reference image.
 * @param ref_offset  Offset (in number of pixels with respect to \a ref_image)
 *                    of the first pixel of the reference sub-image.
 * @param ref_width   Width of reference sub-image.
 * @param ref_height  Height of reference sub-image.
 * @param ref_stride  Elements per row of \a ref_image.
 * @param dx          X-position of reference sub-image with respect
 *                    to raw sub-image.
 * @param dy          Y-position of reference sub-image with respect
 *                    to raw sub-image.
 * @param scale       Scale factor, if \a scale = 0, the error is normalized
 *                    by the total number of pixels in the overlapping region
 *                    *and* non-overlapping regions.
 * @param bg          Background level for pixels outside the overlapping
 *                    region.
 *
 * @return The cost, -1 in case of error.
 */

double img_cost_l2(const int type,
                   const void *raw_image,
                   const long raw_offset,
                   const long raw_width,
                   const long raw_height,
                   const long raw_stride,
                   const void *ref_image,
                   const long ref_offset,
                   const long ref_width,
                   const long ref_height,
                   const long ref_stride,
                   const long dx,
                   const long dy,
                   const double bg,
                   double scale)
{
  double result;
  if ((raw_image == NULL) || (ref_image == NULL)) {
    errno = EFAULT;
    return -1.0;
  }
  if ((raw_width < 1) || (raw_height < 1) || (raw_stride < raw_width) ||
      (ref_width < 1) || (ref_height < 1) || (ref_stride < ref_width)) {
    errno = EINVAL;
    return -1.0;
  }

#define CASE(TYPE) case IMG_TYPE_##TYPE:                                \
    result = COST_L2(TYPE)((const CPT_CTYPE(TYPE) *)raw_image,          \
                           raw_offset, raw_width,                       \
                           raw_height, raw_stride,                      \
                           (const CPT_CTYPE(TYPE) *)ref_image,          \
                           ref_offset, ref_width,                       \
                           ref_height, ref_stride,                      \
                           dx, dy, bg, scale);                          \
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

/*---------------------------------------------------------------------------*/

#else /* _IMG_COST_C defined */

double COST_L2(TYPE)(const pixel_t *raw_image,
                     const long raw_offset,
                     const long raw_width,
                     const long raw_height,
                     const long raw_stride,
                     const pixel_t *ref_image,
                     const long ref_offset,
                     const long ref_width,
                     const long ref_height,
                     const long ref_stride,
                     const long dx,
                     const long dy,
                     const double bg,
                     double scale)
{
  double s;
  long raw_x0, raw_x1, raw_y0, raw_y1;
  long ref_x0, ref_x1, ref_y0, ref_y1;
  long x, y, offset, temp;

  raw_image += raw_offset;
  ref_image += ref_offset;
  s = 0;

  /* Compute the bounding box coordinates of the overlapping region in the
     reference and raw images.  The limits are X0 <= X < X1 and Y0 <= Y < Y1
     that is (X0,Y0 inclusive and (X1,Y1) exclusive. */
  if (dx >= 0) {
    if ((raw_x0 = dx) >= raw_width) {
      goto no_overlap;
    }
    ref_x0 = 0;
  } else {
    if ((ref_x0 = -dx) >= ref_width) {
      goto no_overlap;
    }
    raw_x0 = 0;
  }
  if (dy >= 0) {
    if ((raw_y0 = dy) >= raw_height) {
      goto no_overlap;
    }
    ref_y0 = 0;
  } else {
    if ((ref_y0 = -dy) >= ref_height) {
      goto no_overlap;
    }
    raw_y0 = 0;
  }
  if ((temp = ref_width + dx) <= raw_width) {
    raw_x1 = temp;
    ref_x1 = ref_width;
  } else {
    raw_x1 = raw_width;
    ref_x1 = raw_width - dx;
  }
  if ((temp = ref_height + dy) <= raw_height) {
    raw_y1 = temp;
    ref_y1 = ref_height;
  } else {
    raw_y1 = raw_height;
    ref_y1 = raw_height - dy;
  }

  /* Integrate the cost in the overlapping region. */
  offset = (ref_x0 - raw_x0) + (ref_y0 - raw_y0)*ref_stride;
  for (y = raw_y0; y < raw_y1; ++y) {
    const pixel_t *raw = raw_image + y*raw_stride;
    const pixel_t *ref = ref_image + offset + y*ref_stride;
    for (x = raw_x0; x < raw_x1; ++x) {
      double a = (double)ref[x] - (double)raw[x];
      s += a*a;
    }
  }

  /* Integrate the cost in the non-overlapping regions of the raw
     sub-image. */
  for (y = 0; y < raw_y0; ++y) {
    const pixel_t *raw = raw_image + y*raw_stride;
    for (x = 0; x < raw_width; ++x) {
      double a = raw[x] - bg;
      s += a*a;
    }
  }
  if (raw_x0 > 0 || raw_x1 < raw_width) {
    for (y = raw_y0; y < raw_y1; ++y) {
      const pixel_t *raw = raw_image + y*raw_stride;
      for (x = 0; x < raw_x0; ++x) {
	double a = raw[x] - bg;
	s += a*a;
      }
      for (x = raw_x1; x < raw_width; ++x) {
	double a = raw[x] - bg;
	s += a*a;
      }
    }
  }
  for (y = raw_y1; y < raw_height; ++y) {
    const pixel_t *raw = raw_image + y*raw_stride;
    for (x = 0; x < raw_width; ++x) {
      double a = raw[x] - bg;
      s += a*a;
    }
  }

  /* Integrate the cost in the non-overlapping regions of the reference
     sub-image. */
  for (y = 0; y < ref_y0; ++y) {
    const pixel_t *ref = ref_image + y*ref_stride;
    for (x = 0; x < ref_width; ++x) {
      double a = ref[x] - bg;
      s += a*a;
    }
  }
  if (ref_x0 > 0 || ref_x1 < ref_width) {
    for (y = ref_y0; y < ref_y1; ++y) {
      const pixel_t *ref = ref_image + y*ref_stride;
      for (x = 0; x < ref_x0; ++x) {
	double a = ref[x] - bg;
	s += a*a;
      }
      for (x = ref_x1; x < ref_width; ++x) {
	double a = ref[x] - bg;
	s += a*a;
      }
    }
  }
  for (y = ref_y1; y < ref_height; ++y) {
    const pixel_t *ref = ref_image + y*ref_stride;
    for (x = 0; x < ref_width; ++x) {
      double a = ref[x] - bg;
      s += a*a;
    }
  }

  if (scale == 0.0) {
    scale = 1.0/(double)(raw_width*raw_height
			 + ref_width*(ref_height - ref_y1 + ref_y0)
			 + (ref_width - ref_x1 + ref_x0)*(ref_y1 - ref_y0));
  }
  return scale*s;

  /* The two sub-images are not overlapping. */
 no_overlap:
  for (y = 0; y < raw_height; ++y) {
    const pixel_t *pixel = raw_image + y*raw_stride;
    for (x = 0; x < raw_width; ++x) {
      double a = pixel[x] - bg;
      s += a*a;
    }
  }
  for (y = 0; y < ref_height; ++y) {
    const pixel_t *pixel = ref_image + y*ref_stride;
    for (x = 0; x < ref_width; ++x) {
      double a = pixel[x] - bg;
      s += a*a;
    }
  }
  if (scale == 0.0) {
    scale = 1.0/(double)(raw_width*raw_height + ref_width*ref_height);
  }
  return scale*s;
}

/* Undefine macro(s) that may be re-defined to avoid warnings. */
#undef TYPE

#endif /* _IMG_COST_C defined */

/*---------------------------------------------------------------------------*/

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
