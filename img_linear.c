/*
 * img_linear.c --
 *
 * Linear transform of an image.
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

#ifndef _IMG_LINEAR_C
#define _IMG_LINEAR_C 1

#include <errno.h>
#include <math.h>

#include "img.h"

#ifndef NULL
# define NULL ((void *)0)
#endif

/* Definitions that will be expanded by the template code. */

#define STATIC_FUNC(name,type)  CPT_JOIN3(name, _, CPT_ABBREV(type))
#define PRIVATE_FUNC(name,type) CPT_JOIN4(_img_, name, _, CPT_ABBREV(type))
#define PUBLIC_FUNC(name,type)  CPT_JOIN4(img_, name, _, CPT_ABBREV(type))

#define pixel_t    CPT_CTYPE(TYPE)

/* Manage to include this file with a different data type each time.  The
   handling of "int" is special as "int" can be the same as "short" or "long"
   depending on the compiler. */

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
 * @brief Extract a rectangular region from an image with coordinate transform.
 *
 * This function extracts a rectangular region of size \a dst_width by
 * \a dst_height with a linear change of coordinates by means of bi-linear
 * interpolation.  The coordinate transform is given by the coefficients
 * \a a.  If \a inverse is false, the coefficients are those of the direct
 * transform:
 * @code
 * xp = a[0] + a[1]*x + a[2]*y;
 * yp = a[3] + a[4]*x + a[5]*y;
 * @endcode
 * with (x,y) and (xp,yp) the coordinates in the source and destination image
 * respectively.  If \a inverse is true, then the coefficients are those of
 * the inverse transform:
 * @code
 * x = a[0] + a[1]*xp + a[2]*yp;
 * y = a[3] + a[4]*xp + a[5]*yp;
 * @endcode
 * The first pixel has coordinates (0,0) in the source and destination images.
 *
 * @param src         The base address of the source image.
 * @param src_type    The data type of the source image, must be the same
 *                    as \a dst_type.
 * @param src_offset  The offset, in pixels relative to the base address, of
 *                    the first pixel of the source image.
 * @param src_width   The width of the source image.
 * @param src_height  The height of the source image.
 * @param src_pitch   The number of pixels between two successive rows of the
 *                    source image.
 * @param dst         The base address of the destination image.
 * @param dst_type    The data type of the destination image, must be the same
 *                    as \a src_type.
 * @param dst_offset  The offset, in pixels relative to the base address, of
 *                    the first pixel of the destination image.
 * @param dst_width   The width of the destination image.
 * @param dst_height  The height of the destination image.
 * @param dst_pitch   The number of pixels between two successive rows of the
 *                    destination image.
 * @param a           A 6-element array of coefficients of the coordinate
 *                    transform to apply.
 * @param inverse     True if \a a gives the coefficients of the inverse
 *                    coordinate transform.
 *
 * @return Normally \c IMG_SUCCESS; \c IMG_FAILURE in case of error and
 *         \c errno set to \c EFAULT if one of the addresses is invalid,
 *         to \c EINVAL if one of the other arguments is invalid or if
 *         the linear transform is singular.
 *
 * @see img_inverse_linear_transform(), img_copy().
 */
int img_extract_rectangle(const void *src,
                          const int src_type,
                          const long src_offset,
                          const long src_width,
                          const long src_height,
                          const long src_pitch,
                          void *dst,
                          const int dst_type,
                          const long dst_offset,
                          const long dst_width,
                          const long dst_height,
                          const long dst_pitch,
                          const double a[6],
                          int inverse)
{
  double b[6];

  if ((src == NULL) || (dst == NULL) || (a == NULL)) {
    errno = EFAULT;
    return IMG_FAILURE;
  }
  if ((src_width <= 0) || (src_height <= 0) || (src_pitch < src_width) ||
      (dst_width <= 0) || (dst_height <= 0) || (dst_pitch < dst_width) ||
      (dst_type != src_type)) {
    errno = EINVAL;
    return IMG_FAILURE;
  }
  if (inverse) {
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    b[3] = a[3];
    b[4] = a[4];
    b[5] = a[5];
  } else if (img_inverse_linear_transform(a, 6, b) != IMG_SUCCESS) {
    return IMG_FAILURE;
  }

#define CASE(TYPE) case IMG_TYPE_##TYPE:                            \
    STATIC_FUNC(extract_rectangle,TYPE)(src, src_offset, src_width, \
                                        src_height, src_pitch,      \
                                        dst, dst_offset, dst_width, \
                                        dst_height, dst_pitch, b);  \
    break

  switch (src_type) {
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
    errno = EINVAL;
    return IMG_FAILURE;
  }

#undef CASE

  return IMG_SUCCESS;
}

/**
 * @brief Compute the coefficients of an inverse 2-D linear transform.
 *
 * This function computes the coefficients of the inverse linear transform
 * defined by \f$ (x,y) \mapsto (xp,yp) \f$ as:
 * @code
 * xp = a[0]*x + a[1]*y;
 * yp = a[2]*x + a[3]*y;
 * @endcode
 * or as:
 * @code
 * xp = a[0] + a[1]*x + a[2]*y;
 * yp = a[3] + a[4]*x + a[5]*y;
 * @endcode
 * depending whether \a ncoefs is 4 or 6 respectively.

 * This function takes care of overflows and allows to have the same input and
 * output (\a a and \a b can be the same array).
 *
 * @param a        The input coefficients of the direct transform.
 * @param ncoefs   The number of coefficients in \a a and \a b, must
 *                 be 4 or 6.
 * @param b        The output coefficients of the inverse transform.
 *
 * @return \c IMG_FAILURE if the input transform is singular; \c IMG_SUCCESS
 *         otherwise.
 */
int img_inverse_linear_transform(const double a[], long ncoefs, double b[])
{
  /* The following constant is to keep at least 3 significant digits in the
     inversion. */
  const double eps = 1E3*DBL_EPSILON;
  double amax, axx, axy, ayx, ayy, q, r, s, t;

  /* Copy values from A (in case A and B are the same). */
  if (ncoefs == 4) {
    axx = a[0];
    axy = a[1];
    ayx = a[2];
    ayy = a[3];
  } else if (ncoefs == 6) {
    axx = a[1];
    axy = a[2];
    ayx = a[4];
    ayy = a[5];
  } else {
    errno = EINVAL;
    return IMG_FAILURE;
  }

  /* Get maximum absolute value of input coefficients. */
  amax = fabs(axx);
  if ((t = fabs(axy)) > amax) amax = t;
  if ((t = fabs(ayx)) > amax) amax = t;
  if ((t = fabs(ayy)) > amax) amax = t;

  /* Normalize the coefficients to avoid overflows. */
  if (amax != 1.0) {
    if (amax <= 0.0) goto singular;
    t = 1.0/amax;
    axx *= t;
    axy *= t;
    ayx *= t;
    ayy *= t;
  }

  /* Compute the determinant. */
  r = axx*ayy;
  s = axy*ayx;
  q = fabs(r);
  if ((t = fabs(s)) > q) q = t;
  t = r - s;
  if (fabs(t) <= eps*q) goto singular;

  /* Compute the inverse. */
  t = 1.0/(amax*t);
  axx *= t;
  axy *= t;
  ayx *= t;
  ayy *= t;
  if (ncoefs == 4) {
    b[0] =  ayy;
    b[1] = -axy;
    b[2] = -ayx;
    b[3] =  axx;
  } else {
    double cx = a[0], cy = a[3];
    b[0] =  axy*cy - ayy*cx;
    b[1] =  ayy;
    b[2] = -axy;
    b[3] =  ayx*cx - axx*cy;
    b[4] = -ayx;
    b[5] =  axx;
  }
  return IMG_SUCCESS;

 singular:
  errno = ERANGE;
  return IMG_FAILURE;
}

#else /* _IMG_LINEAR_C *******************************************************/

/* The CONVERT macro is used to convert an interpolated value to the current
   data type.  For floating-point types, no extra work is needed.  For
   integers, rounding to nearest integer is sufficient because no overflow
   should occurs because interpolation coefficients are all in the range [0,1]
   and their sum is equal to 1.  For unsigned integer, it is sufficient to
   add 0.5 to the value. */
#if CPT_IS_INTEGER(TYPE)
# if CPT_IS_UNSIGNED(TYPE)
#  define CONVERT(expr) ((expr) + half)
# else
#  define CONVERT(expr) floor((expr) + half)
# endif
#else
#  define CONVERT(expr) (expr)
#endif

/* FIXME: To simplify the code, it is assumed that DST and SRC are separate
   images which do not share the same memory part.  Some optimizations are
   also possible by using single precision floating point (though taking care
   of rounding errors) and SIMD instructions. */

static void STATIC_FUNC(extract_rectangle,TYPE)(const pixel_t *src,
                                                const long src_offset,
                                                const long src_width,
                                                const long src_height,
                                                const long src_pitch,
                                                pixel_t *dst,
                                                const long dst_offset,
                                                const long dst_width,
                                                const long dst_height,
                                                const long dst_pitch,
                                                const double a[6])
{
  const double zero = 0.0;
  const double one = 1.0;
#if CPT_IS_INTEGER(TYPE)
  const double half = 0.5;
#endif
  double x, x_max, y, y_max, u0, u1, v0, v1, tx, ty;
  double axx, axy, ayx, ayy, bx, by, cx, cy;
  long xp, yp, x0, y0, x1, y1;

  src += src_offset;
  dst += dst_offset;
  x_max = src_width - one;
  y_max = src_height - one;

  /* Get coefficients of linear transform. */
  cx  = a[0];
  axx = a[1];
  axy = a[2];
  cy  = a[3];
  ayx = a[4];
  ayy = a[5];

  /* Interpolate image (bilinear interpolation). */
  for (yp = 0; yp < dst_height; ++yp, dst += dst_pitch) {
    ty = (double)yp;
    bx = axy*ty + cx;
    by = ayy*ty + cy;
    for (xp = 0; xp < dst_width; ++xp) {
      tx = (double)xp;
      x = axx*tx + bx;
      y = ayx*tx + by;
      if (x < zero) {
        x1 = x0 = 0;
        u1 = one;
        u0 = zero;
      } else if (x >= x_max) {
        x1 = x0 = src_width - 1;
        u1 = one;
        u0 = zero;
      } else {
        /* To compute X0, no needs to use floor(X) because X >= 0, nor to
           check for integer overflow because X < X_MAX. */
        x0 = (long)x;
        x1 = x0 + 1;
        u1 = x - x0;
        u0 = one - u1;
      }
      if (y < zero) {
        y1 = y0 = 0;
        v1 = one;
        v0 = zero;
      } else if (y >= y_max) {
        y1 = y0 = src_height - 1;
        v1 = one;
        v0 = zero;
      } else {
        /* To compute Y0, no needs to use floor(Y) because Y >= 0, nor to
           check for integer overflow because Y < Y_MAX. */
        y0 = (long)y;
        y1 = y0 + 1;
        v1 = y - y0;
        v0 = one - v1;
      }
      dst[xp] = CONVERT(u0*(v0*src[x0 + src_pitch*y0] +
                            v1*src[x0 + src_pitch*y1]) +
                        u1*(v0*src[x1 + src_pitch*y0] +
                            v1*src[x1 + src_pitch*y1]));
    }
  }

}
#undef CONVERT
#undef TYPE

#endif /* _IMG_LINEAR_C ******************************************************/
