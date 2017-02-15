/*
 * img_detect.c --
 *
 * Fast spot detection algorithms.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2011-2013 Éric Thiébaut <eric.thiebaut@univ-lyon1.fr>
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can use, modify
 * and/or redistribute the software under the terms of the CeCILL-C license as
 * circulated by CEA, CNRS and INRIA at the following URL
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

#ifndef _IMG_DETECT_C
#define _IMG_DETECT_C 1

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "c_pseudo_template.h"
#include "img.h"

#ifndef NULL
# define NULL ((void *)0)
#endif

#define CLEAR_RESULT_FIRST

/* These definitions are to simplify modifications of the code and also for
   clarity (the prototypes should match the definitions in "img.h").  Some
   definitions will be expanded by the template code (according to the current
   values of the macros TYPE and REAL). */
#define index_t           int
#define pixel_t           CPT_CTYPE(TYPE)
#define real_t            CPT_CTYPE(REAL)
#define DETECT_SPOT(TYPE) CPT_JOIN2(detect_spot_, CPT_ABBREV(TYPE))


/*
 * Title: Real-Time Processing
 *        ====================
 *
 * The image processing for the detection of events involves the following
 * steps:
 *
 *   1/ - de-interlacing of the raw image (optional, depends on the camera
 *        settings);
 *   2/ - subtraction of the background, this yields th so-called "clean"
 *        image;
 *   3/ - filtering of the clean image to produce the filtered image;
 *   4/ - detection of peaks in the filtered image and storage of detected
 *        events.
 *
 * This processing is implemented by functions <cpng_detect()> and
 * <cpng_detect_dalsa_cad6()> of the CPng library.
 *
 * Filtering and Detection:
 * ------------------------
 * The filtered F(x,y) image at pixel (x,y) is computed as
 * follows:
 *
 * : F(x,y) =   c0*I(x,y)
 * :          + c1*(I(x,y-1) + I(x-1,y) + I(x+1,y) + I(x,y+1))
 * :          + c2*(I(x-1,y-1) + I(x+1,y-1) + I(x-1,y+1) + I(x+1,y+1))     (1)
 *
 * with c0, c1 and c2 the coefficients of the filter and I(x,y) the clean
 * (background subtracted) image.  The purpose of the filtering is to enhance
 * the SNR and remove artifacts.
 *
 * A detection is considered positive if and only
 * if:
 *
 * : F(x,y) > max{t0, q1 + t1, q2 + t2}                                    (2)
 *
 * with t0, t1 and t2 the levels of the detection thresholds
 * and:
 *
 * : q1 = max{F(x,y-1), F(x-1,y), F(x+1,y),F(x,y+1)}                      (3a)
 * : q2 = max{F(x-1,y-1), F(x+1,y-1), F(x-1,y+1), F(x+1,y+1)}             (3b)
 *
 * which are respectively the maximum values in the filtered image for the
 * edge and corner neighbors of the pixel (x,y).
 *
 *
 * Data Types:
 * -----------
 * For the Dalsa CA-D6 camera, the column are interleaved and pixels in a row
 * of the raw image must be re-ordered.  This means that a complete row of
 * pixels must be available at a time.  An image by this camera is 532x516
 * pixels coded as unsigned 8-bit integers (raw_pixel_t is uint8_t).
 *
 * Background subtraction requires to use at least 9-bit signed integers
 * to store the clean image.  Hence the clean image and the background
 * are stored and computed with pixels coded as 16-bit signed integers.
 * For cache memory reasons, it may be faster to store the background with
 * 8-bit integers and convert their type "on the fly".
 *
 * Using 16-bit signed integers for pixels left enough dynamics to also
 * compute the convolution with the same data type.
 *
 *
 * In-line processing:
 * -------------------
 * Processing must be as fast as possible to achieve real-time operation (the
 * frame rate is 262 images per seconds for the Dalsa CA-D6 camera which
 * corresponds to ~ 3.8 ms/img).  There are two limitations for the speed of
 * the processing: (i) the number of required operations per pixels; (ii) the
 * slowness of the memory.  To maximize the available computational power,
 * several threads can be used (which rise the issues of balancing the
 * computation load and synchronizing).  Since reading a value in conventional
 * memory is much slower than performing an elementary operation, we choose to
 * do all the processing "in-line" rather than "sequentially" and to use a
 * limited amount of memory (see section "Workspace" below) in the hope that
 * it mostly remains in the CPU cache.  By "in-line" processing, we meant that
 * all the processing steps are performed for each pixel; on the contrary,
 * "sequential" processing would have meant that each processing step is
 * applied to the whole image in turn (hence a complete clean image and a
 * complete filtered image ahv to be written once and read several times).  A
 * consequence is that the raw (input) image has to be locked during all the
 * processing (this is also true for the background image but it is not
 * subject to change during acquisition and can be safely shared between the
 * threads).
 *
 * To check detection of an event at (x,y), the filtered image must have been
 * computed for pixels in the 3x3 box (x-1:x+1,y-1:y+1) around (x,y).  To
 * limit the size of necessary arrays and maximize the number of "local"
 * operations -- that is, operations performed per read of a "clean" pixel --
 * a convolution by the filter and a detection are done for each pixel read,
 * except for the edges.  Asumming sequential reading of the input image, the
 * pixel at (x+2,y+2) -- noted I3(x+2) in the Figure below -- must have been
 * read to achieve computation of all the filtered values needed to check
 * detection at (x,y).
 *
 * :                   +-----------+-----------+-----------+
 * :                   |           |           |           |
 * :   y+2             |   I3(x)   |  I3(x+1)  |  I3(x+2)  |
 * :                   |           |           |           |
 * :       +-----------+-----------+-----------+-----------+
 * :       |           |           |           |           |
 * :   y+1 |  I2(x-1)  |   I2(x)   |  I2(x+1)  |  I2(x+2)  |
 * :       |           |           |           |           |
 * :       +-----------+-----------+-----------+-----------+
 * :       |           |           |           |           |
 * :   y   |  I1(x-1)  |   I1(x)   |  I1(x+1)  |  I1(x+2)  |
 * :       |           |           |           |           |
 * :       +-----------+-----------+-----------+-----------+
 * :       |           |           |           |
 * :   y-1 |  I0(x-1)  |   I0(x)   |  I0(x+1)  |            
 * :       |           |           |           |
 * :       +-----------+-----------+-----------+
 * :            x-1          x          x+1          x+2
 *
 * Notation for the 7 workspace rows of the clean image and of the filtered
 * image:
 *
 * :   I0(x) = I(x,y-1)              F0(x) = F(x,y-1)
 * :   I1(x) = I(x,y)                F1(x) = F(x,y)  
 * :   I2(x) = I(x,y+1)              F2(x) = F(x,y+1)
 * :   I3(x) = I(x,y+2)
 *
 * For a potential detection at pixel (x,y), the in-line processing consists
 * in: reading the raw image at pixel (x+2,y+2), subtract the background level
 * at the same location, compute the filtered image at pixel (x+1,y+1):
 *
 * : I(x+2,y+2) = RAW(x+2,y+2) - BIAS(x+2,y+2)
 * :
 * : ==> I3(x+2) = RAW(x+2,y+2) - BIAS(x+2,y+2)
 * :
 * : F(x+1,y+1) =   c0*I(x+1,y+1)
 * :              + c1*(I(x+1,y) + I(x,y+1) + I(x+2,y+1) + I(x+1,y+2))
 * :              + c2*(I(x,y) + I(x+2,y) + I(x,y+2) + I(x+2,y+2))
 * :
 * : ==> F2(x+1) =   c0*I2(x+1)
 * :               + c1*(I1(x+1) + I2(x) + I2(x+2) + I3(x+1))
 * :               + c2*(I1(x) + I1(x+2) + I3(x) + I3(x+2))
 * :             = c0*s5 + c1*(s2 + s4 + s6) + c2*(s1 + s3)
 *
 * where the variables s1, s2, s3, s4, s5 and s6 are used to store the
 * pixel values and partial sums:
 *
 * : s1 = I(x,y)   + I(x,y+2)   = I1(x)   + I3(x)
 * : s2 = I(x+1,y) + I(x+1,y+2) = I1(x+1) + I3(x+1)
 * : s3 = I(x+2,y) + I(x+2,y+2) = I1(x+2) + I3(x+2)
 * : s4 = I(x,y+1)              = I2(x)
 * : s5 = I(x+1,y+1)            = I2(x+1)
 * : s6 = I(x+2,y+1)            = I2(x+2)
 *
 * Hence, after proper initialization, when a new clean image pixel is
 * computed, the updating of these variables is simply:
 *
 * : I3(x+2) = RAW(x+2,y+2) - BIAS(x+2,y+2)
 * : s1 = s2
 * : s2 = s2
 * : s3 = I1(x+2) + I3(x+2)
 * : s4 = s5
 * : s5 = s6
 * : s6 = I2(x+2)
 * : F2(x+1) = c0*s5 + c1*(s2 + s4 + s6) + c2*(s1 + s3)
 *
 * This minimizes the number of operations required to compute the new filter
 * value at F2(x+1) = F(x+1,y+1).  By unrolling loops (by a factor 3) it is
 * also possible to avoid the copying of the values.
 *
 * To test for detection -- Eq. (2), (3a) and (3b) -- we store values of the
 * filtered image and partial maxima in variables m1, m2, m3, m4, m5 and m6:
 *
 * : m1 = max{F(x,y),   F(x,y+2)}   = max{F1(x),   F3(x)}
 * : m2 = max{F(x+1,y), F(x+1,y+2)} = max{F1(x+1), F3(x+1)}
 * : m3 = max{F(x+2,y), F(x+2,y+2)} = max{F1(x+2), F3(x+2)}
 * : m4 = F(x,y)                    = F2(x)
 * : m5 = F(x+1,y)                  = F2(x+1)
 * : m6 = F(x+2,y)                  = F2(x+2)
 *
 * Once the new filtered value F2(x+1) = F(x+1,y+1) has been computed, the
 * updating of these variables and the detection test write:
 *   
 * : m1 = m2
 * : m2 = m3
 * : m3 = max{F1(x+2), F3(x+2)}
 * : m4 = m5
 * : m5 = m6
 * : m6 = F2(x+2)
 * : q1 = max{m2, m4, m6}
 * : q2 = max{m1, m3} 
 * : m5 > max{t0, q1 + t1, q2 + t2}
 *
 *
 * Workspace:
 * ----------
 * To avoid recomputation of the filtered image, at least 3 rows of the
 * filtered image must be stored at y-1, y and y+1.  These rows are denoted
 * F0, F1 and F2 respectively.  If a detection occurs, rows y-1, y and y+1 of
 * the clean image must be remembered to save the 3x3 box around the event,
 * and, to compute the filtered values in the same 3x3 box, row at y+2 must
 * also be stored.  This means 4 rows of pixels for the clean image, labeled
 * I0, I1, I2, and I3, from y-1 to y+2.  Hence the required workspace has a
 * total of 7 rows of pixels (assuming clean and filtered image use the same
 * pixel type).  To avoid copy of values the memorized rows are cyclically
 * permuted when a new row of pixels is considered.
 *
 * Glossary:
 * ---------
 *  raw image        - The image as given by the camera device.
 *  clean image      - The image after background subtraction.
 *  filtered image   - The image after background subtraction and convolution
 *                     by the detection filter.
 */

/* Macros to simplify the code. */

#define MIN(a,b)  ((a) <= (b) ? (a) : (b))
#define MAX(a,b)  ((a) >= (b) ? (a) : (b))

#define PUSH2(a1,a2,expr)          a1=a2; a2=(expr)
#define PUSH3(a1,a2,a3,expr)       a1=a2; a2=a3; a3=(expr)
#define PUSH4(a1,a2,a3,a4,expr)    a1=a2; a2=a3; a3=a4; a4=(expr)
#define PUSH5(a1,a2,a3,a4,a5,expr) a1=a2; a2=a3; a3=a4; a4=a5; a5=(expr)

#define ROLL2(tmp,a1,a2)          tmp=a1; a1=a2; a2=tmp
#define ROLL3(tmp,a1,a2,a3)       tmp=a1; a1=a2; a2=a3; a3=tmp
#define ROLL4(tmp,a1,a2,a3,a4)    tmp=a1; a1=a2; a2=a3; a3=a4; a4=tmp
#define ROLL5(tmp,a1,a2,a3,a4,a5) tmp=a1; a1=a2; a2=a3; a3=a4; a4=a5; a5=tmp

/* Apply the convolution filter (S0 is the intensity of the central pixel, S1
   is the sum of the intensities of the edge pixels, and S2 is the sum of the
   intensities of the corner pixels. */
#define CNVL(s0,s1,s2)       ((s0)*c0 + (s1)*c1 + (s2)*c2)

#define CNVL_BEG CNVL(s5, s2+s6, s3)       /* at beginning of a row */
#define CNVL_MID CNVL(s5, s2+s4+s6, s1+s3) /* in the middle of a row */
#define CNVL_END CNVL(s6, s3+s5, s2)       /* at the end of a row */

/* Case 1: x = 0, y = 0 */
#define CNVL_1(f,i0,i1,i2,x)                    \
  s2 = i2[0];                                   \
  s3 = i2[1];                                   \
  s5 = i1[0];                                   \
  s6 = i1[1];                                   \
  f[x] = CNVL_BEG

/* Case 2: 1 <= x <= width - 2, y = 0 */
#define CNVL_2(f,i0,i1,i2,x)                    \
  PUSH3(s1, s2, s3, i2[x+1]);                   \
  PUSH3(s4, s5, s6, i1[x+1]);                   \
  f[x] = CNVL_MID

/* Case 3: x = width - 1, y = 0 */
#define CNVL_3(f,i0,i1,i2,x)                    \
  f[x] = CNVL_END

/* Case 4: x = 0, 1 <= y <= height - 2 */
#define CNVL_4(f,i0,i1,i2,x)                    \
  s2 = i0[0] + i2[0];                           \
  s3 = i0[1] + i2[1];                           \
  s5 = i1[0];                                   \
  s6 = i1[1];                                   \
  f[x] = CNVL_BEG

/* Case 5: 1 <= x <= width - 2, 1 <= y <= height - 2 */
#define CNVL_5(f,i0,i1,i2,x)                    \
  PUSH3(s1, s2, s3, i0[x+1] + i2[x+1]);         \
  PUSH3(s4, s5, s6, i1[x+1]);                   \
  f[x] = CNVL_MID

/* Case 6: x = width - 1, 1 <= y <= height - 2 */
#define CNVL_6(f,i0,i1,i2,x)                    \
  f[x] = CNVL_END


/* Case 7: x = 0, y = height - 1 */
#define CNVL_7(f,i0,i1,i2,x)                    \
  s2 = i0[0];                                   \
  s3 = i0[1];                                   \
  s5 = i1[0];                                   \
  s6 = i1[1];                                   \
  f[x] = CNVL_BEG

/* Case 8: 1 <= x <= width - 2, y = height - 1 */
#define CNVL_8(f,i0,i1,i2,x)                    \
  PUSH3(s1, s2, s3, i0[x+1]);                   \
  PUSH3(s4, s5, s6, i1[x+1]);                   \
  f[x] = CNVL_MID

/* Case 9: x = width - 1, y = height - 1 */
#define CNVL_9(f,i0,i1,i2,x)                    \
  f[x] = CNVL_END

/* Macros to filter a whole row of pixels. */
#define FILTER_ROW(f,i0,i1,i2,k1,k2,k3) do {       \
    index_t __x = 0, __last = width - 1;           \
    CNVL_##k1(f, i0, i1, i2, 0);                   \
    while (++__x < __last) {                       \
      CNVL_##k2(f, i0, i1, i2, __x);               \
    }                                              \
    CNVL_##k3(f, i0, i1, i2, __last);              \
  } while (0)
#define FILTER_ROW_BOT(f,i0,i1,i2) FILTER_ROW(f,i0,i1,i2, 1,2,3)
#define FILTER_ROW_MID(f,i0,i1,i2) FILTER_ROW(f,i0,i1,i2, 4,5,6)
#define FILTER_ROW_TOP(f,i0,i1,i2) FILTER_ROW(f,i0,i1,i2, 7,8,9)

/* FIXME: __x < __last ===>  __x <= xmax */
 
/* Macro to apply bias and gain correction to a whole row of pixels. */
#define LOAD_ROW(i) do {                        \
    index_t __x;                                \
    for (__x = 0; __x < width; ++__x) {         \
      i[__x] = IMG(__x);                        \
    }                                           \
  } while (0)

/*---------------------------------------------------------------------------*/
/* Manage to include this file with a different data type each time. */

#if defined(IMG_TYPE_INT8)
# define TYPE INT8
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_UINT8)
# define TYPE UINT8
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_INT16)
# define TYPE INT16
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_UINT16)
# define TYPE UINT16
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_INT32)
# define TYPE INT32
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_UINT32)
# define TYPE UINT32
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_INT64)
# define TYPE INT64
# define REAL DOUBLE
# include __FILE__
#endif

#if defined(IMG_TYPE_UINT64)
# define TYPE UINT64
# define REAL DOUBLE
# include __FILE__
#endif

#if defined(IMG_TYPE_FLOAT)
# define TYPE FLOAT
# define REAL FLOAT
# include __FILE__
#endif

#if defined(IMG_TYPE_DOUBLE)
# define TYPE DOUBLE
# define REAL DOUBLE
# include __FILE__
#endif

#if defined(IMG_TYPE_SCOMPLEX) && 0
# define TYPE SCOMPLEX
# include __FILE__
#endif

#if defined(IMG_TYPE_DCOMPLEX) && 0
# define TYPE DCOMPLEX
# include __FILE__
#endif

#if defined(IMG_TYPE_RGB) && 0
# define TYPE RGB
# include __FILE__
#endif

#if defined(IMG_TYPE_RGBA) && 0
# define TYPE RGBA
# include __FILE__
#endif

int img_detect_spot(const void* src, const int type, 
                    const int width, const long height,
                    const double c0, const double c1, const double c2,
                    const double t0, const double t1, const double t2,
                    int dst[], long* count_ptr, double* ws)
{
  long count;

  /* Check arguments and initialization. */
  if (dst == NULL || src == NULL || ws == NULL) {
    errno = EFAULT;
    goto failure;
  }
  if (width < 1 || height < 1) {
    errno = EINVAL;
    goto failure;
  }
#ifdef CLEAR_RESULT_FIRST
  memset(dst, 0, width*height*sizeof(dst[0]));
#endif

#define CASE(TYPE)                                                      \
  case IMG_TYPE_##TYPE:                                                 \
    count = DETECT_SPOT(TYPE)((const CPT_CTYPE(TYPE)*)src,              \
                              width, height, c0, c1, c2, t0, t1, t2,    \
                              dst, (void*)ws);                          \
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
    errno = EINVAL;
    goto failure;
  }

#undef CASE

  if (count_ptr != NULL) {
    *count_ptr = count;
  }
  return IMG_SUCCESS;

failure:
  if (count_ptr != NULL) {
    *count_ptr = -1L;
  }
  return IMG_FAILURE;
}


#else /* _IMG_DETECT_C defined */

static long DETECT_SPOT(TYPE)(const pixel_t src[],
                              const index_t width,
                              const index_t height,
                              const real_t c0,
                              const real_t c1,
                              const real_t c2,
                              const real_t t0,
                              const real_t t1,
                              const real_t t2,
                              int dst[],
                              void* ws)
{
  const pixel_t* img0;
  const pixel_t* img1;
  const pixel_t* img2;
  const pixel_t* img3;
  real_t* tmp_ptr;
  real_t* flt0;
  real_t* flt1;
  real_t* flt2;
  long count;
  const real_t zero = 0;
  real_t s1, s2, s3, s4, s5, s6, s6new, s3new, fnew;
  real_t m1, m2, m3, m4, m5, m6, q1, q2;
  index_t x, xmax, y, ymax;
  int inside;


  /* No events for now. */
  count = 0L;
  if (width < 3 || height < 3) {
    return count;
  }
  
  /* Coordinates of first/last pixel where a spot can be detected. */
#undef xmin
#define xmin 1
  xmax = width - 2;
#undef ymin
#define ymin 1
  ymax = height - 2;

  /* Initialization of row pointers. */
  img0 = src;
  img1 = img0 + width;
  img2 = img1 + width;
  img3 = img2 + width;
  flt0 = (real_t*)ws;
  flt1 = flt0 + width;
  flt2 = flt1 + width*2;

  /* Compute the 2 first rows of the filtered image. */
  FILTER_ROW_BOT(flt1, img0, img1, img2);
  FILTER_ROW_MID(flt2, img1, img2, img3);
#ifndef CLEAR_RESULT_FIRST
  for (y = 0; y < ymin; ++y) {
    memset(&dst[y*width], 0, width*sizeof(dst[0]));
  }
#endif
  
  for (y = ymin; y <= ymax; ++y) {
    /* Permute the rows of the image and of the filtered image. */
    PUSH4(img0, img1, img2, img3, img3 + width);
    ROLL3(tmp_ptr, flt0, flt1, flt2);

    /* Initialize variables for the start of a row. */
    inside = (y < ymax);
    if (inside) {
      s1 = (real_t)img1[0] + (real_t)img3[0];
      s2 = (real_t)img1[1] + (real_t)img3[1];
      s3 = (real_t)img1[2] + (real_t)img3[2];
    } else {
      s1 = (real_t)img1[0];
      s2 = (real_t)img1[1];
      s3 = (real_t)img1[2];
    }
    s4 = (real_t)img2[0];
    s5 = (real_t)img2[1];
    s6 = (real_t)img2[2];
    flt2[0] = CNVL(s4, s1+s5, s2);
    flt2[1] = CNVL(s5, s2+s4+s6, s1+s3);
    m1 = zero;
    m2 = MAX(flt0[0], flt2[0]);
    m3 = MAX(flt0[1], flt2[1]);
    m4 = zero;
    m5 = flt1[0];
    m6 = flt1[1];
#ifndef CLEAR_RESULT_FIRST
    memset(&dst[y*width], 0, width*sizeof(dst[0]));
#endif
  
    /* Loop over all allowed columns. */
    for (x = xmin; x <= xmax; ++x) {
      /* Compute image value at pixel (x+2,y+2) and update partial sums.  We
         strongly rely on the optimizer and on the efficiency of branch
         prediction for the tests below. */
      if (x < xmax) {
        /* Rightmost column at x+2 is inside image. */
        if (inside) {
          /* Topmost row (img3) at y+2 is inside image. */
          s3new = (real_t)img1[x+2] + (real_t)img3[x+2];
        } else {
          s3new = (real_t)img1[x+2];
        }
        s6new = (real_t)img2[x+2];
      } else {
        /* Rightmost column at x+2 is outside image. */
        s3new = s6new = zero;
      }
      PUSH3(s1, s2, s3, s3new);
      PUSH3(s4, s5, s6, s6new);

      /* Compute filter value at pixel (x+1,y+1) and update partial maxima. */
      fnew = CNVL(s5, s2+s4+s6, s1+s3);
      flt2[x+1] = fnew;
      if (fnew < flt0[x+1]) {
        fnew = flt0[x+1];
      }
      PUSH3(m1, m2, m3, fnew);
      PUSH3(m4, m5, m6, flt1[x+1]);

      /* Check for detection. */
      if (m5 > t0) {
        q1 = MAX(m2, m4);
        if (q1 < m6) q1 = m6;
        if (m5 > q1 + t1) {
          q2 = MAX(m1, m3);
          if (m5 > q2 + t2) {
            dst[y*width + x] = 1;
            ++count;
          }
        }
      }
    }
  }
#ifndef CLEAR_RESULT_FIRST
  for (y = ymax + 1; y < height; ++y) {
    memset(&dst[y*width], 0, width*sizeof(dst[0]));
  }
#endif

  return count;
}

#undef TYPE
#undef REAL

#endif /* _IMG_DETECT_C */

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * c-basic-offset: 2
 * fill-column: 78
 * coding: utf-8
 * End:
 */
