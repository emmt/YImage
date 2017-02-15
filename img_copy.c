/*
 * img_copy.c --
 *
 * Copy and conversion of an image.
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

#ifndef _IMG_COPY_C
#define _IMG_COPY_C 1

#include <errno.h>
#include <math.h>

#include "img.h"

/* Definitions of macros. */
#ifndef NULL
# define NULL ((void *)0)
#endif
#define COPY(src,dst) CPT_JOIN5(copy, _, CPT_ABBREV(src), _, CPT_ABBREV(dst))

/* Brightness giben RGB components (see, e.g., color FAQ at
   http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html). */
#define LUMA(R,G,B)   (0.2126*(R) + 0.7152*(G) + 0.0722*(B))

/* Manage to include this file with a different source data type each time.
   This is the first level of "self" inclusion, a second level is needed to
   loop over the data type of the destination. */
#ifdef IMG_TYPE_INT8
# define SRC INT8
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT8
# define SRC UINT8
# include __FILE__
#endif
#ifdef IMG_TYPE_INT16
# define SRC INT16
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT16
# define SRC UINT16
# include __FILE__
#endif
#ifdef IMG_TYPE_INT32
# define SRC INT32
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT32
# define SRC UINT32
# include __FILE__
#endif
#ifdef IMG_TYPE_INT64
# define SRC INT64
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT64
# define SRC UINT64
# include __FILE__
#endif
#ifdef IMG_TYPE_FLOAT
# define SRC FLOAT
# include __FILE__
#endif
#ifdef IMG_TYPE_DOUBLE
# define SRC DOUBLE
# include __FILE__
#endif
#ifdef IMG_TYPE_SCOMPLEX
# define SRC SCOMPLEX
# include __FILE__
#endif
#ifdef IMG_TYPE_DCOMPLEX
# define SRC DCOMPLEX
# include __FILE__
#endif
#ifdef IMG_TYPE_RGB
# define SRC RGB
# include __FILE__
#endif
#ifdef IMG_TYPE_RGBA
# define SRC RGBA
# include __FILE__
#endif

/**
 * @brief Convert and copy the pixels of a rectangular region.
 *
 * This function copies a rectangular region of interest (ROI) of size
 * \a width by \a height with possible conversion of pixel type.
 *
 * @param width       The width of the ROI.
 * @param height      The height of the ROI.
 * @param src_addr    The base address of the source image.
 * @param src_type    The data type of the source image.
 * @param src_offset  The offset, in pixels relative to the base address, of
 *                    the first pixel of the source image.
 * @param src_pitch   The number of pixels between two successive rows of the
 *                    source image.
 * @param dst_addr    The base address of the destination image.
 * @param dst_type    The data type of the destination image.
 * @param dst_offset  The offset, in pixels relative to the base address, of
 *                    the first pixel of the destination image.
 * @param dst_pitch   The number of pixels between two successive rows of the
 *                    destination image.
 *
 * @return Normally \c IMG_SUCCESS; \c IMG_FAILURE in case of error and
 *         \c errno set to \c EFAULT if one of the addresses is invalid,
 *         to \c EINVAL if one of the other arguments is invalid.
 */
int img_copy(const long width,
             const long height,
             const void *src_addr,
             const int   src_type,
             const long  src_offset,
             const long  src_pitch,
             void       *dst_addr,
             const int   dst_type,
             const long  dst_offset,
             const long  dst_pitch)
{
  if ((src_addr == NULL) || (dst_addr == NULL)) {
    errno = EFAULT;
    return IMG_FAILURE;
  }
  if ((width <= 0) || (height <= 0) ||
      (src_pitch < width) || (dst_pitch < width)) {
    errno = EINVAL;
    return IMG_FAILURE;
  }

#define CALL(SRC,DST) COPY(SRC,DST)(width,height,src_addr,src_offset,\
                                    src_pitch,dst_addr,dst_offset,dst_pitch)

#define CASE2(SRC,DST) case IMG_TYPE_##DST: CALL(SRC,DST); break

#ifdef IMG_TYPE_INT8
# define CASEa(SRC) CASE2(SRC,INT8)
#else
# define CASEa(SRC)
#endif

#ifdef IMG_TYPE_UINT8
# define CASEb(SRC) CASEa(SRC); CASE2(SRC,UINT8)
#else
# define CASEb(SRC) CASEa(SRC)
#endif

#ifdef IMG_TYPE_INT16
# define CASEc(SRC) CASEb(SRC); CASE2(SRC,INT16)
#else
# define CASEc(SRC) CASEb(SRC)
#endif

#ifdef IMG_TYPE_UINT16
# define CASEd(SRC) CASEc(SRC); CASE2(SRC,UINT16)
#else
# define CASEd(SRC) CASEc(SRC)
#endif

#ifdef IMG_TYPE_INT32
# define CASEe(SRC) CASEd(SRC); CASE2(SRC,INT32)
#else
# define CASEe(SRC) CASEd(SRC)
#endif

#ifdef IMG_TYPE_UINT32
# define CASEf(SRC) CASEe(SRC); CASE2(SRC,UINT32)
#else
# define CASEf(SRC) CASEe(SRC)
#endif

#ifdef IMG_TYPE_INT64
# define CASEg(SRC) CASEf(SRC); CASE2(SRC,INT64)
#else
# define CASEg(SRC) CASEf(SRC)
#endif

#ifdef IMG_TYPE_UINT32
# define CASEh(SRC) CASEg(SRC); CASE2(SRC,UINT64)
#else
# define CASEh(SRC) CASEg(SRC)
#endif

#ifdef IMG_TYPE_FLOAT
# define CASEi(SRC) CASEh(SRC); CASE2(SRC,FLOAT)
#else
# define CASEi(SRC) CASEh(SRC)
#endif

#ifdef IMG_TYPE_DOUBLE
# define CASEj(SRC) CASEi(SRC); CASE2(SRC,DOUBLE)
#else
# define CASEj(SRC) CASEi(SRC)
#endif

#ifdef IMG_TYPE_SCOMPLEX
# define CASEk(SRC) CASEj(SRC); CASE2(SRC,SCOMPLEX)
#else
# define CASEk(SRC) CASEj(SRC)
#endif

#ifdef IMG_TYPE_DCOMPLEX
# define CASEl(SRC) CASEk(SRC); CASE2(SRC,DCOMPLEX)
#else
# define CASEl(SRC) CASEk(SRC)
#endif

#ifdef IMG_TYPE_RGB
# define CASEm(SRC) CASEl(SRC); CASE2(SRC,RGB)
#else
# define CASEm(SRC) CASEl(SRC)
#endif

#ifdef IMG_TYPE_RGBA
# define CASEn(SRC) CASEm(SRC); CASE2(SRC,RGBA)
#else
# define CASEn(SRC) CASEm(SRC)
#endif

#define CASE1(SRC)                              \
  case IMG_TYPE_##SRC:                          \
    switch (dst_type) {                         \
    CASEn(SRC);                                 \
    default:                                    \
      errno = EINVAL;                           \
      return IMG_FAILURE;                       \
    }                                           \
    break

  switch (src_type) {
#ifdef IMG_TYPE_INT8
    CASE1(INT8);
#endif
#ifdef IMG_TYPE_UINT8
    CASE1(UINT8);
#endif
#ifdef IMG_TYPE_INT16
    CASE1(INT16);
#endif
#ifdef IMG_TYPE_UINT16
    CASE1(UINT16);
#endif
#ifdef IMG_TYPE_INT32
    CASE1(INT32);
#endif
#ifdef IMG_TYPE_UINT32
    CASE1(UINT32);
#endif
#ifdef IMG_TYPE_INT64
    CASE1(INT64);
#endif
#ifdef IMG_TYPE_UINT64
    CASE1(UINT64);
#endif
#ifdef IMG_TYPE_FLOAT
    CASE1(FLOAT);
#endif
#ifdef IMG_TYPE_DOUBLE
    CASE1(DOUBLE);
#endif
#ifdef IMG_TYPE_SCOMPLEX
    CASE1(SCOMPLEX);
#endif
#ifdef IMG_TYPE_DCOMPLEX
    CASE1(DCOMPLEX);
#endif
#ifdef IMG_TYPE_RGB
    CASE1(RGB);
#endif
#ifdef IMG_TYPE_RGBA
    CASE1(RGBA);
#endif
  default:
    errno = EINVAL;
    return IMG_FAILURE;
  }

#undef CALL
#undef CASE1
#undef CASE2
#undef CASEa
#undef CASEb
#undef CASEc
#undef CASEd
#undef CASEe
#undef CASEf
#undef CASEg
#undef CASEh
#undef CASEi
#undef CASEj
#undef CASEk
#undef CASEl
#undef CASEm
#undef CASEn

  return IMG_SUCCESS;
}

#elif (_IMG_COPY_C == 1) /****************************************************/

#undef _IMG_COPY_C
#define _IMG_COPY_C 2

#ifdef IMG_TYPE_INT8
# define DST INT8
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT8
# define DST UINT8
# include __FILE__
#endif
#ifdef IMG_TYPE_INT16
# define DST INT16
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT16
# define DST UINT16
# include __FILE__
#endif
#ifdef IMG_TYPE_INT32
# define DST INT32
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT32
# define DST UINT32
# include __FILE__
#endif
#ifdef IMG_TYPE_INT64
# define DST INT64
# include __FILE__
#endif
#ifdef IMG_TYPE_UINT64
# define DST UINT64
# include __FILE__
#endif
#ifdef IMG_TYPE_FLOAT
# define DST FLOAT
# include __FILE__
#endif
#ifdef IMG_TYPE_DOUBLE
# define DST DOUBLE
# include __FILE__
#endif
#ifdef IMG_TYPE_SCOMPLEX
# define DST SCOMPLEX
# include __FILE__
#endif
#ifdef IMG_TYPE_DCOMPLEX
# define DST DCOMPLEX
# include __FILE__
#endif
#ifdef IMG_TYPE_RGB
# define DST RGB
# include __FILE__
#endif
#ifdef IMG_TYPE_RGBA
# define DST RGBA
# include __FILE__
#endif

/* Restore the state. */
#undef SRC
#undef _IMG_COPY_C
#define _IMG_COPY_C 1

#else /* _IMG_COPY_C == 2 ****************************************************/

#undef src_t
#undef SRC_STRIDE
#if CPT_IS_COMPLEX(SRC)
# define SRC_STRIDE 2
# if CPT_IS_SCOMPLEX(SRC)
#  define src_t float
# elif CPT_IS_DCOMPLEX(SRC)
#  define src_t double
# endif
#elif CPT_IS_COLOR(SRC)
# define src_t uint8_t
# if CPT_IS_RGB(SRC)
#  define SRC_STRIDE 3
# elif CPT_IS_RGBA(SRC)
#  define SRC_STRIDE 4
# endif
#else
# define src_t CPT_CTYPE(SRC)
# define SRC_STRIDE 1
#endif

#undef dst_t
#undef DST_STRIDE
#if CPT_IS_COMPLEX(DST)
# define DST_STRIDE 2
# if CPT_IS_SCOMPLEX(DST)
#  define dst_t float
# elif CPT_IS_DCOMPLEX(DST)
#  define dst_t double
# endif
#elif CPT_IS_COLOR(DST)
# define dst_t uint8_t
# if CPT_IS_RGB(DST)
#  define DST_STRIDE 3
# elif CPT_IS_RGBA(DST)
#  define DST_STRIDE 4
# endif
#else
# define dst_t CPT_CTYPE(DST)
# define DST_STRIDE 1
#endif

static void COPY(SRC,DST)(const long  width, const long  height,
                          const void *src_addr, const long src_offset,
                          long src_pitch, void *dst_addr,
                          const long dst_offset, long dst_pitch)
{
  const src_t *src = src_addr;
  dst_t *dst = dst_addr;
  long x, y;

#if SRC_STRIDE > 1
  src += SRC_STRIDE*src_offset;
  src_pitch *= SRC_STRIDE;
#else
  src += src_offset;
#endif

#if DST_STRIDE > 1
  dst += DST_STRIDE*dst_offset;
  dst_pitch *= DST_STRIDE;
#else
  dst += dst_offset;
#endif

  for (y = 0; y < height; ++y, src += src_pitch, dst += dst_pitch) {
    for (x = 0; x < width; ++x) {
#if CPT_IS_RGB(SRC)
      /* Conversion from a RGB color source image. */
# if CPT_IS_RGB(DST)
      dst[3*x] = src[3*x];
      dst[3*x+1] = src[3*x+1];
      dst[3*x+2] = src[3*x+2];
# elif CPT_IS_RGBA(DST)
      const dst_t max = 255;
      dst[4*x] = src[3*x];
      dst[4*x+1] = src[3*x+1];
      dst[4*x+2] = src[3*x+2];
      dst[4*x+3] = max;
# elif CPT_IS_COMPLEX(DST)
      const dst_t zero = 0;
      dst[2*x] = LUMA(src[3*x], src[3*x+1], src[3*x+2]);
      dst[2*x+1] = zero;
# else
      dst[x] = LUMA(src[3*x], src[3*x+1], src[3*x+2]);
# endif
#elif CPT_IS_RGBA(SRC)
      /* Conversion from a RGBA color source image. */
# if CPT_IS_RGB(DST)
      dst[3*x] = src[4*x];
      dst[3*x+1] = src[4*x+1];
      dst[3*x+2] = src[4*x+2];
# elif CPT_IS_RGBA(DST)
      dst[4*x] = src[4*x];
      dst[4*x+1] = src[4*x+1];
      dst[4*x+2] = src[4*x+2];
      dst[4*x+3] = src[4*x+3];
# elif CPT_IS_COMPLEX(DST)
      const dst_t zero = 0;
      dst[2*x] = LUMA(src[4*x], src[4*x+1], src[4*x+2]);
      dst[2*x+1] = zero;
# else
      dst[x] = LUMA(src[4*x], src[4*x+1], src[4*x+2]);
# endif
#elif CPT_IS_COMPLEX(SRC)
      /* Conversion from a complex source image. */
# if CPT_IS_RGB(DST)
      dst_t value = (dst_t)src[2*x];
      dst[3*x] = value;
      dst[3*x+1] = value;
      dst[3*x+2] = value;
# elif CPT_IS_RGBA(DST)
      const dst_t max = 255;
      dst_t value = (dst_t)src[2*x];
      dst[4*x] = value;
      dst[4*x+1] = value;
      dst[4*x+2] = value;
      dst[4*x+3] = max;
# elif CPT_IS_COMPLEX(DST)
      dst[2*x] = src[2*x];
      dst[2*x+1] = src[2*x+1];
# else
      dst[x] = src[2*x];
# endif
#else
      /* Conversion from a grayscale source image. */
# if CPT_IS_RGB(DST)
      dst_t value = (dst_t)src[x];
      dst[3*x] = value;
      dst[3*x+1] = value;
      dst[3*x+2] = value;
# elif CPT_IS_RGBA(DST)
      const dst_t max = 255;
      dst_t value = (dst_t)src[x];
      dst[4*x] = value;
      dst[4*x+1] = value;
      dst[4*x+2] = value;
      dst[4*x+3] = max;
# elif CPT_IS_COMPLEX(DST)
      const dst_t zero = 0;
      dst[2*x] = src[x];
      dst[2*x+1] = zero;
# else
      dst[x] = src[x];
# endif
#endif
    }
  }
}

#undef DST

#endif /* _IMG_COPY_C ********************************************************/
