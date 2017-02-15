/*
 * img_morph.c --
 *
 * Implementation of simple morpho-math image routines.
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

#ifndef _IMG_MORPH_C
#define _IMG_MORPH_C 1

#include <stdlib.h>
#include <errno.h>

#include "img.h"


/* Definitions of public functions. */

extern int img_morph_lmin_lmax(int type, long width, long height,
                               const void *img, long img_pitch,
                               long r, long ws[],
                               void *lmin, long lmin_pitch,
                               void *lmax, long lmax_pitch);

extern int img_morph_erosion(int type, long width, long height,
                             const void *img, long img_pitch,
                             long r, long ws[],
                             void *lmin, long lmin_pitch);

extern int img_morph_dilation(int type, long width, long height,
                              const void *img, long img_pitch,
                              long r, long ws[],
                              void *lmax, long lmax_pitch);


/* Definitions that will be expanded by the template code. */

#define MORPH_LMIN_LMAX(TYPE) CPT_JOIN(img_morph_lmin_lmax_,CPT_ABBREV(TYPE))

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
 * @brief Compute local minima and/or maxima of an image.
 *
 * This function performs morpho-math erosion and/or morpho-math dilation of
 * an image.  The neighborhood of each pixel is defined by a structuring
 * element which is a disk of radius \a r centered at the pixel of interest.
 *
 * @param type        The type identifier of the input image \a img and
 *                    outputs \a lmin and \a lmax.
 * @param width       The image width.
 * @param height      The image height.
 * @param img         The input image.
 * @param img_pitch   The number of elements per row of \a img.
 * @param r           The radius of the neighborhood, must be non-negative.
 * @param ws          A workspace array of, at least, 2*R + 1 elements.
 * @param lmin        The address of array to store local minima or \c NULL.
 * @param lmin_pitch  The number of elements per row of \a lmin.
 * @param lmax        The address of array to store local maxima or \c NULL.
 * @param lmax_pitch  The number of elements per row of \a lmax.
 *
 * @return \c IMG_SUCCESS or \c IMG_FAILURE.
 *
 * @see img_morph_erosion(), img_morph_dilation().
 */

int img_morph_lmin_lmax(int type, long width, long height,
                        const void *img, long img_pitch,
                        long r, long ws[],
                        void *lmin, long lmin_pitch,
                        void *lmax, long lmax_pitch)
{
  if ((img == NULL) || (ws == NULL)) {
    errno = EFAULT;
    return IMG_FAILURE;
  }
  if ((r < 0) || (width <= 0) || (height <= 0) || (img_pitch < width)
      || ((lmin != NULL) && (lmin_pitch < width))
      || ((lmax != NULL) && (lmax_pitch < width))) {
    errno = EINVAL;
    return IMG_FAILURE;
  }

#define CASE(TYPE) case IMG_TYPE_##TYPE:                                  \
  MORPH_LMIN_LMAX(TYPE)(width, height, (const CPT_CTYPE(TYPE) *)img,      \
                        img_pitch, r, ws, (CPT_CTYPE(TYPE) *)lmin,        \
                        lmin_pitch, (CPT_CTYPE(TYPE) *)lmax, lmax_pitch); \
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
    return IMG_FAILURE;
  }

#undef CASE

  return IMG_SUCCESS;
}

/**
 * @brief Get the local minima of an image.
 *
 * This function performs morpho-math erosion of an image. The result is an
 * image set with the local minima in the neighborhood of every input pixels.
 * The neighborhood is defined by the structuring element which is a disk of
 * radius \a r centered at the pixel of interest.
 *
 * @param type        The type identifier of the input image \a img and
 *                    outputs \a lmin and \a lmax.
 * @param width       The image width.
 * @param height      The image height.
 * @param img         The input image.
 * @param img_pitch   The number of elements per row of \a img.
 * @param r           The radius of the neighborhood, must be non-negative.
 * @param ws          A workspace array of, at least, 2*R + 1 elements.
 * @param lmin        The address of array to store local minima or \c NULL.
 * @param lmin_pitch  The number of elements per row of \a lmin.
 *
 * @return \c IMG_SUCCESS or \c IMG_FAILURE.
 *
 * @see img_morph_dilation(), img_morph_lmin_lmax().
 */
int img_morph_erosion(int type, long width, long height,
                      const void *img, long img_pitch,
                      long r, long ws[],
                      void *lmin, long lmin_pitch)
{
  return img_morph_lmin_lmax(type, width, height, img, img_pitch,
                             r, ws, lmin, lmin_pitch, NULL, 0);
}

/**
 * @brief Get the local maxima of an image.
 *
 * This function performs morpho-math dialtion of an image. The result is an
 * image set with the local maxima in the neighborhood of every input pixels.
 * The neighborhood is defined by the structuring element which is a disk of
 * radius \a r centered at the pixel of interest.
 *
 * @param type        The type identifier of the input image \a img and
 *                    outputs \a lmin and \a lmax.
 * @param width       The image width.
 * @param height      The image height.
 * @param img         The input image.
 * @param img_pitch   The number of elements per row of \a img.
 * @param r           The radius of the neighborhood, must be non-negative.
 * @param ws          A workspace array of, at least, 2*R + 1 elements.
 * @param lmax        The address of array to store local maxima or \c NULL.
 * @param lmax_pitch  The number of elements per row of \a lmax.
 *
 * @return \c IMG_SUCCESS or \c IMG_FAILURE.
 *
 * @see img_morph_erosion(), img_morph_lmin_lmax().
 */

int img_morph_dilation(int type, long width, long height,
                       const void *img, long img_pitch,
                       long r, long ws[],
                       void *lmax, long lmax_pitch)
{
  return img_morph_lmin_lmax(type, width, height, img, img_pitch,
                             r, ws, NULL, 0, lmax, lmax_pitch);
}

/*---------------------------------------------------------------------------*/

#else /* _IMG_MORPH_C defined */

static void MORPH_LMIN_LMAX(TYPE)(const long width, const long height,
                                  const pixel_t img[], const long img_pitch,
                                  const long r, long ws[],
                                  pixel_t lmin[], const long lmin_pitch,
                                  pixel_t lmax[], const long lmax_pitch)
{
  pixel_t pval, pmin, pmax; /* pixel values */
  long dx, dy, dx0, dx1, dy0, dy1; /* offsets and bounds */
  long x, y; /* coordinates in source/destination image */
  long *off;

  /*
   * Fill the offset array with the range of DX for any DY.  To lie inside the
   * neighborhood, the condition is:
   *
   *    sqrt(dx*dx + dy*dy) < r + 0.5
   *
   * Taking the square and accounting for the fact that DX, DY and R are
   * integers yields:
   *
   *    dx*dx <= (r + 1)*r - dy*dy
   */
  off = ws + r;
  for (dy = 0; dy <= r; ++dy) {
    long t = (r + 1)*r - dy*dy;
    dx = r;
    while (dx*dx > t) {
      --dx;
    }
    off[ dy] = dx;
    off[-dy] = dx;
  }

  /*
   * The following macro set inclusive endpoints [START,STOP] of range
   * for position POS +/- SPAN along axis of lenght LEN.
   */
#define SET_RANGE(pos, len, span, start, stop) \
    start = ((pos) >= (span) ? -(span) : -(pos)); \
    stop = ((pos) + (span) < (len) ? (span) : (len) - 1 - (pos))

  /*
   * Perfom the morpho-math operation(s).
   */
  if (lmin != (pixel_t *)NULL && lmax != (pixel_t *)NULL) {
    for (y = 0; y < height; ++y) {
      SET_RANGE(y, height, r, dy0, dy1);
      for (x = 0; x < width; ++x) {
        const pixel_t *cur = &img[x + img_pitch*y];
        pmin = pmax = cur[0];
        for (dy = dy0; dy <= dy1; ++dy) {
          const pixel_t *fast = &cur[img_pitch*dy];
          long t = off[dy];
          SET_RANGE(x, width, t, dx0, dx1);
          for (dx = dx0; dx <= dx1; ++dx) {
            pval = fast[dx];
            if (pval < pmin) pmin = pval;
            if (pval > pmax) pmax = pval;
          }
        }
        lmin[x + lmin_pitch*y] = pmin;
        lmax[x + lmax_pitch*y] = pmax;
      }
    }
  } else if (lmin != (pixel_t *)NULL) {
    for (y = 0; y < height; ++y) {
      SET_RANGE(y, height, r, dy0, dy1);
      for (x = 0; x < width; ++x) {
        const pixel_t *cur = &img[x + img_pitch*y];
        pmin = cur[0];
        for (dy = dy0; dy <= dy1; ++dy) {
          const pixel_t *fast = &cur[img_pitch*dy];
          long t = off[dy];
          SET_RANGE(x, width, t, dx0, dx1);
          for (dx = dx0; dx <= dx1; ++dx) {
            pval = fast[dx];
            if (pval < pmin) pmin = pval;
          }
        }
        lmin[x + lmin_pitch*y] = pmin;
      }
    }
  } else if (lmax != (pixel_t *)NULL) {
    for (y = 0; y < height; ++y) {
      SET_RANGE(y, height, r, dy0, dy1);
      for (x = 0; x < width; ++x) {
        const pixel_t *cur = &img[x + img_pitch*y];
        pmax = cur[0];
        for (dy = dy0; dy <= dy1; ++dy) {
          const pixel_t *fast = &cur[img_pitch*dy];
          long t = off[dy];
          SET_RANGE(x, width, t, dx0, dx1);
          for (dx = dx0; dx <= dx1; ++dx) {
            pval = fast[dx];
            if (pval > pmax) pmax = pval;
          }
        }
        lmax[x + lmax_pitch*y] = pmax;
      }
    }
  }

  /*
   * Destroy "local" macro.
   */
#undef SET_RANGE
}

/* Undefine macro(s) that may be re-defined to avoid warnings. */
#undef TYPE

#endif /* _IMG_MORPH_C defined */
