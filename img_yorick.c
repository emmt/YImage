/*
 * img_yorick.c --
 *
 * Implementation of Yorick wrapper functions.
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pstdlib.h>
#include <yapi.h>

#include "img.h"
#include "img_version.h"

/* This macro returns the number of elements of a fixed size array. */
#define NUMBEROF(arr)  (sizeof(arr)/sizeof(arr[0]))

/* Macro to get rid of some GCC extensions when not compiling with GCC. */
#if ! (defined(__GNUC__) && __GNUC__ > 1)
#  undef __attribute__
#  define __attribute__(x) /* empty */
#endif

/* Refine definition of YError to avoid GCC warnings (about uninitialized
   variables or reaching end of non-void function): */
PLUG_API void y_error(const char *msg) __attribute__ ((noreturn));

/*---------------------------------------------------------------------------*/

#ifndef Y_OPAQUE
# define Y_OPAQUE 17
#endif

/* IMG_TYPE_BYTE is the counter part of Yorick's char type. */
#define IMG_TYPE_BYTE IMG_TYPE_UINT8

/* IMG_TYPE_COMPLEX is the only complex data type available in Yorick. */
#define IMG_TYPE_COMPLEX IMG_TYPE_DCOMPLEX

/* Check for the availability of pixel types corresponding to Yorick
   base types. */
#ifndef IMG_TYPE_SHORT
# warning IMG_TYPE_SHORT not defined
#endif
#ifndef IMG_TYPE_INT
# warning IMG_TYPE_INT not defined
#endif
#ifndef IMG_TYPE_LONG
# warning IMG_TYPE_LONG not defined
#endif

/*
 * CONVENTIONS FOR OPAQUE OBJECTS
 * ==============================
 *
 * To make the code easier to understand (and also to define some useful
 * macros), we have adpoted the following conventions:
 *
 *   yACTION_CLASS = function for Yorick API to perform ACTION (e.g. free,
 *                   push, get, ...) on an object of a given CLASS.
 *
 *   ytype_CLASS   = structure of type y_userobj_t which stores the definition
 *                   of the class for ypush_obj(), yfunc_obj() and
 *                   yget_obj().
 *
 *   CLASS_t       = C-type for an object instance.
 *
 *   CLASS_ACTION  = function to operate on an object of a given CLASS but
 *                   independently from Yorick.
 */

/* These macros can be used push/get and object pointer to/from the stack.
 * OBJPTR is an object pointer (of type CLASS_t), IARG is the stack index
 * and CLASS is the name of the object class.  It is assumed that the
 * following conventions are followed:
 *   1. ytype_CLASS is a structure of type y_userobj_t which stores the
 *      definition of the class.
 *   2. CLASS_t is the C-type of the object.
 */
#define YPUSH_OBJPTR(CLASS, OBJPTR) \
          (*(CLASS##_t **)ypush_obj(&ytype_##CLASS, sizeof(void *)) = (OBJPTR))
#define YGET_OBJPTR(CLASS, IARG) \
          (*(CLASS##_t **)yget_obj(IARG, &ytype_##CLASS))

/* Built-in functions defined in this file. */
extern void Y_img_get_version(int argc);
extern void Y__img_init(int argc);
extern void Y_img_morph_erosion(int argc);
extern void Y_img_morph_dilation(int argc);
extern void Y_img_morph_lmin_lmax(int argc);
extern void Y_img_morph_opening(int argc);
extern void Y_img_morph_closing(int argc);
extern void Y_is_image(int argc);
extern void Y_img_is_rgb(int argc);
extern void Y_img_is_rgba(int argc);
extern void Y_img_is_color(int argc);
extern void Y_img_get_channel(int argc);
extern void Y_img_get_red(int argc);
extern void Y_img_get_green(int argc);
extern void Y_img_get_blue(int argc);
extern void Y_img_get_alpha(int argc);
extern void Y_img_get_width(int argc);
extern void Y_img_get_height(int argc);
extern void Y_img_get_type(int argc);
extern void Y_img_estimate_noise(int argc);
extern void Y_img_cost_l2(int argc);

typedef struct _image image_t;
struct _image {
  void *data;
  long width, height;
  int type;   /* pixel type, one of the IMG_TYPE_* values */
};

static void get_image(int iarg, image_t *img);
static void new_image(image_t *img);

static void push_string(const char *value);
static void convert_image(int iarg, image_t *img, int new_img_type);
static int get_binop_type(int left_type, int right_type);
static void get_range_or_length(int iarg, long *start, long *stop, long *step,
                                long *length);

/*---------------------------------------------------------------------------*/
/* LINEAR TRANSFORM */


static void get_range_or_length(int iarg, long *start, long *stop, long *step,
                                long *length)
{
  switch (yarg_typeid(iarg)) {
  case Y_CHAR:
  case Y_SHORT:
  case Y_INT:
  case Y_LONG:
    if (yarg_rank(iarg) == 0) {
      long value = ygets_l(iarg);
      if (value <= 0) {
        y_error("invalid dimension length");
      }
      *length = value;
      *start  = 1;
      *stop   = value;
      *step   = 1;
      return;
    }
    break;
  case Y_RANGE:
    {
      long range[3];
      int flags;
      flags = yget_range(iarg, range);
      if ((flags & (Y_MMMARK | Y_PSEUDO | Y_RUBBER | Y_RUBBER1 |
                    Y_MIN_DFLT | Y_MAX_DFLT)) != 0) {
        y_error("range must be in the form start:stop[:step]");
      }
      if ((range[2] > 0) && (range[0] <= range[1])) {
        *length = (range[1] - range[0])/range[2] + 1;
      } else if ((range[2] < 0) && (range[0] >= range[1])) {
        *length = (range[0] - range[1])/(-range[2]) + 1;
      } else {
        y_error("index range step has wrong sign");
      }
      *start = range[0];
      *stop  = range[1];
      *step  = range[2];
    }
  }
  y_error("expecting a dimension length or a range");
}

void Y_img_extract_rectangle(int argc)
{
  double a[6];
  image_t img;
  void *src, *dst;
  long dst_x0, dst_x1, dst_xstep, dst_width,  src_width;
  long dst_y0, dst_y1, dst_ystep, dst_height, src_height;
  int inverse, type;

  inverse = 0;
  if (argc == 9) {
    a[0] = ygets_d(5);
    a[1] = ygets_d(4);
    a[2] = ygets_d(3);
    a[3] = ygets_d(2);
    a[4] = ygets_d(1);
    a[5] = ygets_d(0);
    yarg_drop(6);
  } else if (argc == 4) {
    long ntot;
    double *tmp = ygeta_d(0, &ntot, NULL);
    if (ntot != 6) y_error("coefficients must be a 6-element array");
    memcpy(a, tmp, 6*sizeof(double));
    yarg_drop(1);
  } else if (argc == 3) {
    /* FIXME: avoid interpolation to speed up the code. */
    a[0] = 0.0;
    a[1] = 1.0;
    a[2] = 0.0;
    a[3] = 0.0;
    a[4] = 0.0;
    a[5] = 1.0;
    inverse = 1;
  } else {
    y_error("wrong number of arguments");
  }
  get_image(2, &img);
  type = img.type;
  if ((type == IMG_TYPE_COMPLEX) ||
      (type == IMG_TYPE_RGB) || (type == IMG_TYPE_RGBA)) {
    y_error("operations not yet implemented for color or complex images");
  }
  src_width = img.width;
  src_height = img.height;
  src = img.data;
  get_range_or_length(1, &dst_x0, &dst_x1, &dst_xstep, &dst_width);
  get_range_or_length(0, &dst_y0, &dst_y1, &dst_ystep, &dst_height);
  if ((dst_xstep != 1) || (dst_ystep != 1)) {
    y_error("step != 1 not yet implemented");
  }

  /* Compute the inverse coordinate transform. */
  if (! inverse) {
    if (img_inverse_linear_transform(a, 6, a) != IMG_SUCCESS) {
      y_error("coordinate transform is singular");
    }
  }

  /* Adjust the affine coefficients of the inverse transform because, in
     img_extract_rectangle, the coordinates in the source and destination
     images start at (0,0), not at (SRC_X0,SRC_Y0)=(1,1) nor at
     (DST_X0,DST_Y0). */
#define src_x0 1.0
#define src_y0 1.0
  a[0] += a[1]*dst_x0 + a[2]*dst_y0 - src_x0;
  a[3] += a[4]*dst_x0 + a[5]*dst_y0 - src_y0;
#undef src_x0
#undef src_y0

  /* Allocate the output image. */
  img.width = dst_width;
  img.height = dst_height;
  new_image(&img);
  dst = img.data;

  /* Perform the operation. */
  if (img_extract_rectangle(src, type, 0, src_width, src_height, src_width,
                            dst, type, 0, dst_width, dst_height, dst_width,
                            a, 1) != IMG_SUCCESS) {
    y_error("unexpected failure");
  }
}

/*---------------------------------------------------------------------------*/
/* IMAGE NOISE */

void Y_img_estimate_noise(int argc)
{
  image_t img;
  double result;
  long x0, x1, y0, y1;
  int method;

  /* Get arguments. */
  if ((argc != 1) && (argc != 2) && (argc != 5) && (argc != 6)) {
    y_error("wrong number of arguments");
  }
  get_image(argc - 1, &img);
  if ((argc == 2) || (argc == 6)) {
    method = ygets_i(0);
  } else {
    method = 0;
  }
  if ((argc == 5) || (argc == 6)) {
    x0 = ygets_l(argc - 2);
    x1 = ygets_l(argc - 3);
    y0 = ygets_l(argc - 4);
    y1 = ygets_l(argc - 5);
    if (x0 < 1) x0 += img.width;
    if (x1 < 1) x1 += img.width;
    if (y0 < 1) y0 += img.height;
    if (y1 < 1) y1 += img.height;
    if ((x0 < 1) || (x0 > x1) || (x1 > img.width) ||
        (y0 < 1) || (y0 > y1) || (y1 > img.height)) {
      y_error("out of range sub-image bound(s)");
    }
    --x0;
    --y0;
  } else {
    x0 = 0;
    x1 = img.width;
    y0 = 0;
    y1 = img.height;
  }
  result = img_estimate_noise(img.type, img.data, x0 + img.width*y0,
                              x1 - x0, y1 - y0, img.width, method);
  if (result < 0.0) y_error("bad pixel type");
  ypush_double(result);
}

/*---------------------------------------------------------------------------*/
/* SUB-IMAGE COMPARISON */

void Y_img_cost_l2(int argc)
{
  double scl, bg, res;
  image_t a, b;
  long ax0, ax1, ay0, ay1;
  long bx0, bx1, by0, by1;
  long dx, dy;
  int type;

  /* Get arguments. */
  if (argc != 14) {
    y_error("wrong number of arguments");
  }
  scl = ygets_d(0);
  bg  = ygets_d(1);
  dx  = ygets_l(2);
  dy  = ygets_l(3);
  by1 = ygets_l(4);
  by0 = ygets_l(5);
  bx1 = ygets_l(6);
  bx0 = ygets_l(7);
  get_image(8, &b);
  ay1 = ygets_l(9);
  ay0 = ygets_l(10);
  ax1 = ygets_l(11);
  ax0 = ygets_l(12);
  get_image(13, &a);

  /* Check arguments. */
  type = get_binop_type(a.type, b.type);
  if (type == IMG_TYPE_NONE) {
    y_error("incompatible image types");
  }
  if ((type == IMG_TYPE_RGB) || (type == IMG_TYPE_RGBA) ||
      (type == IMG_TYPE_SCOMPLEX) || (type == IMG_TYPE_DCOMPLEX)) {
    y_error("operation not yet implemented for complex or color images");
  }
  if (scl < 0.0) {
    y_error("scale factor must be non-negative");
  }
  if (ax0 < 1) ax0 += a.width;
  if (ax1 < 1) ax1 += a.width;
  if (ay0 < 1) ay0 += a.height;
  if (ay1 < 1) ay1 += a.height;
  if (bx0 < 1) bx0 += b.width;
  if (bx1 < 1) bx1 += b.width;
  if (by0 < 1) by0 += b.height;
  if (by1 < 1) by1 += b.height;
  fprintf(stderr, "a(%ld:%ld, %ld:%ld) - b(%ld:%ld, %ld:%ld)\n",
          ax0, ax1, ay0, ay1,
          bx0, bx1, by0, by1);
  if ((ax0 < 1) || (ax0 > ax1) || (ax1 > a.width) ||
      (ay0 < 1) || (ay0 > ay1) || (ay1 > a.height) ||
      (bx0 < 1) || (bx0 > bx1) || (bx1 > b.width) ||
      (by0 < 1) || (by0 > by1) || (by1 > b.height)) {
    y_error("out of range sub-image bound(s)");
  }

  if (a.type != type) {
    convert_image(13, &a, type);
  }
  if (b.type != type) {
    convert_image(8, &b, type);
  }

  /* Convert Yorick inclusive and 1-based bounds into bounding box
     inclusive-exclusive and 0-based coordinates.  Then compute the result. */
  --ax0;
  --ay0;
  --bx0;
  --by0;

  res = img_cost_l2(type,
                    a.data, ax0 + a.width*ay0, ax1 - ax0, ay1 - ay0, a.width,
                    b.data, bx0 + b.width*by0, bx1 - bx0, by1 - by0, b.width,
                    dx, dy, bg, scl);
  if (res < 0.0) {
    y_error("bad pixel type");
  }
  ypush_double(res);
}

/*---------------------------------------------------------------------------*/
/* MORPHO-MATH */

#define EROSION   0
#define DILATION  1
#define CONTRAST  2
#define OPENING   3
#define CLOSING   4

static void img_morph_operation(int argc, int what)
{
  long r, lmin_ref, lmax_ref;
  image_t img;
  void *src;
  long *ws;
  int contrast = (what == CONTRAST);

  if ((contrast ? 4 : 2) != argc) {
    y_error("wrong number of arguments");
  }
  if (contrast) {
    lmax_ref = yget_ref(0);
    lmin_ref = yget_ref(1);
    if (lmax_ref < 0 || lmin_ref < 0) {
      y_error("LMIN and LMAX must be simple variables");
    }
    yarg_drop(2); /* left only IMG and R on top of the stack */
  } else {
    lmin_ref = lmax_ref = -1;
  }
  r = ygets_l(0);
  yarg_drop(1);
  get_image(0, &img);
  if (r <= 0) {
    if (r < 0) {
      y_error("radius of structuring element must be non-negative");
    }
    if (contrast) {
      yput_global(lmax_ref, 0);
      yput_global(lmin_ref, 0);
    }
    return;
  }
  ypush_check(3);
  ws = ypush_scratch((2*r + 1)*sizeof(long), NULL);
  src = img.data;
  if (what == EROSION) {
    new_image(&img);
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, img.data, img.width,
                            NULL, 0) != IMG_SUCCESS) {
      goto error;
    }
  } else if (what == DILATION) {
    new_image(&img);
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, NULL, 0,
                            img.data, img.width) != IMG_SUCCESS) {
      goto error;
    }
  } else if (what == CONTRAST) {
    void *lmin_ptr, *lmax_ptr;
    new_image(&img);
    lmin_ptr = img.data;
    yput_global(lmin_ref, 0);
    new_image(&img);
    lmax_ptr = img.data;
    yput_global(lmax_ref, 0);
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, lmin_ptr, img.width,
                            lmax_ptr, img.width) != IMG_SUCCESS) {
      goto error;
    }
    yarg_drop(4); /* clear the stack */
    ypush_nil();
  } else if (what == OPENING) {
    /* Perform an erosion followed by a dilation. */
    new_image(&img);
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, img.data,
                            img.width, NULL, 0) != IMG_SUCCESS) {
      goto error;
    }
    yarg_swap(0, 2); /* exchange input image with temporary result */
    yarg_drop(1); /* get rid of no longer needed input image */
    src = img.data; /* temporary result is the source of next operation */
    new_image(&img); /* allocate image for next operation */
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, NULL, 0,
                            img.data, img.width) != IMG_SUCCESS) {
      goto error;
    }
  } else if (what == CLOSING) {
    /* Perform a dilation followed by an erosion. */
    new_image(&img);
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, NULL, 0,
                            img.data, img.width) != IMG_SUCCESS) {
      goto error;
    }
    yarg_swap(0, 2); /* exchange input image with temporary result */
    yarg_drop(1); /* get rid of no longer needed input image */
    src = img.data; /* temporary result is the source of next operation */
    new_image(&img); /* allocate image for next operation */
    if (img_morph_lmin_lmax(img.type, img.width, img.height,
                            src, img.width, r, ws, img.data,
                            img.width, NULL, 0) != IMG_SUCCESS) {
      goto error;
    }
  }
  return;

 error:
  /* At this point, this is the only possible error. */
  y_error("bad pixel type");
}

extern void Y_img_morph_erosion(int argc)
{
  img_morph_operation(argc, EROSION);
}

extern void Y_img_morph_dilation(int argc)
{
  img_morph_operation(argc, DILATION);
}

extern void Y_img_morph_lmin_lmax(int argc)
{
  img_morph_operation(argc, CONTRAST);
}

extern void Y_img_morph_opening(int argc)
{
  img_morph_operation(argc, OPENING);
}

extern void Y_img_morph_closing(int argc)
{
  img_morph_operation(argc, CLOSING);
}

/*---------------------------------------------------------------------------*/
/* DETECTION */

extern void Y_img_detect_spot(int argc)
{
  image_t img;
  const void* src;
  int* dst;
  double* ws;
  double c0, c1, c2, t0, t1, t2;
  long count;
  int type, iarg;

  if (argc != 7) y_error("wrong number of arguments");
  iarg = argc - 1;
  get_image(iarg, &img);
  type = img.type;
  if (type < IMG_TYPE_INT8 || type > IMG_TYPE_DOUBLE) {
    y_error("bad image type");
  }
  src = img.data;
  --iarg; c0 = ygets_d(iarg);
  --iarg; c1 = ygets_d(iarg);
  --iarg; c2 = ygets_d(iarg);
  --iarg; t0 = ygets_d(iarg);
  --iarg; t1 = ygets_d(iarg);
  --iarg; t2 = ygets_d(iarg);
  ws = (double*)ypush_scratch(3*img.width*sizeof(double), NULL);
  img.type = IMG_TYPE_INT;
  new_image(&img);
  dst = (int*)img.data;
  if (img_detect_spot(src, type, img.width, img.height, c0, c1, c2,
                      t0, t1, t2, dst, &count, ws) != IMG_SUCCESS) {
    y_error("operation failed (bug?)");
  }
}

/*---------------------------------------------------------------------------*/
/* IMAGE MANAGEMENT */

static int yor_type_min = 0, yor_type_max = 0;
static int img_type_min = 0, img_type_max = 0;
static int *type_convert_table = NULL;
static int *img_type_to_yor_type = NULL;
static int *yor_type_to_img_type = NULL;

extern void Y_is_image(int argc)
{
  long rank, dims[Y_DIMSIZE];
  int type, result;

  if (argc != 1) y_error("wrong number of arguments");
  type = yarg_typeid(0);
  result = 0;
  if ((type >= yor_type_min) && (type <= yor_type_max)
      && (yor_type_to_img_type[type] != IMG_TYPE_NONE)) {
    ygeta_any(0, NULL, dims, NULL);
    rank = dims[0];
    if (rank == 2) {
      result = ((type == Y_COMPLEX) ? 2 : 1);
    } else if ((rank == 3) && (type == IMG_TYPE_BYTE)
               && ((dims[1] == 3) || (dims[1] == 4))) {
      result = 3;
    }
  }
  ypush_int(result);
}

extern void Y_img_get_type(int argc)
{
  long rank, dims[Y_DIMSIZE];
  int type;

  if (argc != 1) y_error("wrong number of arguments");
  type = yarg_typeid(0);
  if ((type < yor_type_min) || (type > yor_type_max)
      || (yor_type_to_img_type[type] == IMG_TYPE_NONE)) {
    y_error("bad data type for an image");
  }
  ygeta_any(0, NULL, dims, NULL);
  rank = dims[0];
  if (rank == 2) {
    ypush_long((long)type);
    return;
  }
  if ((rank == 3) && (type == Y_CHAR)) {
    if (dims[1] == 3) {
      ypush_long(Y_OPAQUE + 1);
      return;
    }
    if (dims[1] == 4) {
      ypush_long(Y_OPAQUE + 2);
      return;
    }
  }
  y_error("bad dimensions for an image");
}

extern void Y_img_is_color(int argc)
{
  image_t img;
  if (argc != 1) y_error("wrong number of arguments");
  get_image(0, &img);
  ypush_int((((img.type == IMG_TYPE_RGB) ||
              (img.type == IMG_TYPE_RGBA)) ? 1 : 0));
}

extern void Y_img_is_rgb(int argc)
{
  image_t img;
  if (argc != 1) y_error("wrong number of arguments");
  get_image(0, &img);
  ypush_int(((img.type == IMG_TYPE_RGB) ? 1 : 0));
}

extern void Y_img_is_rgba(int argc)
{
  image_t img;
  if (argc != 1) y_error("wrong number of arguments");
  get_image(0, &img);
  ypush_int(((img.type == IMG_TYPE_RGBA) ? 1 : 0));
}

extern void Y_img_is_complex(int argc)
{
  image_t img;
  if (argc != 1) y_error("wrong number of arguments");
  get_image(0, &img);
  ypush_int(((img.type == IMG_TYPE_COMPLEX) ? 1 : 0));
}

static void get_channel(int argc, long chn);

extern void Y_img_get_channel(int argc)
{
  if (argc != 2) y_error("wrong number of arguments");
  get_channel(1, ygets_l(0));
}

extern void Y_img_get_red(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  get_channel(0, 1);
}

extern void Y_img_get_green(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  get_channel(0, 2);
}

extern void Y_img_get_blue(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  get_channel(0, 3);
}

extern void Y_img_get_alpha(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  get_channel(0, 4);
}

static void get_channel(int iarg, long chn)
{
  image_t img;
  long i, j, npixels, stride;
  unsigned char *src, *dst;

  get_image(iarg, &img);
  if (img.type == IMG_TYPE_RGB) {
    stride = 3;
  } else if (img.type == IMG_TYPE_RGBA) {
    stride = 4;
  } else {
    stride = 0;
  }
  if ((chn < 1) || (chn > 4)) {
    y_error("bad color channel");
  }
  if (chn > stride) {
    if (chn == 1) y_error("image has no red channel");
    if (chn == 2) y_error("image has no green channel");
    if (chn == 3) y_error("image has no blue channel");
    if (chn == 4) y_error("image has no alpha channel");
  }
  src = (unsigned char *)img.data;
  img.type = IMG_TYPE_BYTE;
  new_image(&img);
  dst = (unsigned char *)img.data;
  npixels = img.width*img.height;
  for (i = 0, j = chn - 1; i < npixels; ++i, j += stride) {
    dst[i] = src[j];
  }
}

extern void Y_img_get_width(int argc)
{
  image_t img;

  if (argc != 1) {
    y_error("wrong number of arguments");
  }
  get_image(0, &img);
  ypush_long(img.width);
}

extern void Y_img_get_height(int argc)
{
  image_t img;

  if (argc != 1) {
    y_error("wrong number of arguments");
  }
  get_image(0, &img);
  ypush_long(img.height);
}

/*---------------------------------------------------------------------------*/
/* IMAGE SEGMENTATION */

static img_segmentation_t *yget_img_segmentation(int iarg);
static void ypush_img_segmentation(img_segmentation_t *sgm);
static void yfree_img_segmentation(void *); /* do not call directly */
static y_userobj_t ytype_img_segmentation = {
  "img_segmentation", yfree_img_segmentation,
  NULL, NULL, NULL, NULL
};

static void yfree_img_segmentation(void *ptr)
{
  if ((ptr != NULL) && (*(void **)ptr != NULL)) {
    img_segmentation_unlink(*(img_segmentation_t **)ptr);
  }
}

static img_segmentation_t *yget_img_segmentation(int iarg)
{
  img_segmentation_t *ptr = YGET_OBJPTR(img_segmentation, iarg);
  if (ptr == NULL) {
    y_error("expecting an img_segmentation object");
  }
  return ptr;
}

static void ypush_img_segmentation(img_segmentation_t *sgm)
{
  /* Since yfree_img_segmentation() will be called at the end of the life of
     the Yorick object pushed on top of the stack, we must increase its
     reference count. */
  YPUSH_OBJPTR(img_segmentation, img_segmentation_link(sgm));
}

void Y_img_segmentation_new(int argc)
{
  image_t img;
  double threshold;
  img_segmentation_t *sgm;

  /* Get arguments. */
  if (argc != 2) {
    y_error("wrong number of arguments");
  }
  get_image(1, &img);
  threshold = ygets_d(0);
  sgm = img_segmentation_new(img.data, img.type, 0, img.width, img.height,
                             img.width, threshold);
  if (sgm == NULL) {
    int code = errno;
    if (code == ENOMEM) {
      y_error("insufficient memory");
    } else if (code == EINVAL) {
      y_error("bad pixel type");
    } else {
      y_error("unexpected error in img_segmentation_new()");
    }
  }
  ypush_img_segmentation(sgm);
}

void Y_img_segmentation_get_number(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_segmentation_get_number(yget_img_segmentation(0)));
}

void Y_img_segmentation_get_image_width(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_segmentation_get_image_width(yget_img_segmentation(0)));
}

void Y_img_segmentation_get_image_height(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_segmentation_get_image_height(yget_img_segmentation(0)));
}

void Y_img_segmentation_get_nrefs(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_segmentation_get_nrefs(yget_img_segmentation(0)));
}

#define BUILTIN(what, pushs, pusha)                                     \
  void Y_img_segmentation_get_##what(int argc)                          \
  {                                                                     \
    img_segmentation_t *ws;                                             \
    long j, number;                                                     \
                                                                        \
    if ((argc != 1) && (argc != 2)) {                                   \
      y_error("wrong number of arguments");                             \
    }                                                                   \
    ws = yget_img_segmentation(argc - 1);                               \
    number = img_segmentation_get_number(ws);                           \
    if (argc == 2) {                                                    \
      j = ygets_l(0);                                                   \
      if (j < 1) j += number;                                           \
      if ((j <= 0) || (j > number)) y_error("out of range index");      \
      pushs(img_segmentation_get_##what(ws, j - 1));                    \
    } else if (number > 0) {                                            \
      long dims[2];                                                     \
      dims[0] = 1;                                                      \
      dims[1] = number;                                                 \
      if (img_segmentation_get_##what##s(ws, pusha(dims), number)       \
          != IMG_SUCCESS) y_error("bug in img_segmentation_get_" #what "()"); \
    } else {                                                            \
      ypush_nil();                                                      \
    }                                                                   \
  }
BUILTIN(count, ypush_long, ypush_l)
BUILTIN(xmin, ypush_long, ypush_l)
BUILTIN(xmax, ypush_long, ypush_l)
BUILTIN(ymin, ypush_long, ypush_l)
BUILTIN(ymax, ypush_long, ypush_l)
BUILTIN(width, ypush_long, ypush_l)
BUILTIN(height, ypush_long, ypush_l)
BUILTIN(xcen, ypush_double, ypush_d)
BUILTIN(ycen, ypush_double, ypush_d)
#undef BUILTIN

#define BUILTIN(what, push)                                             \
  void Y_img_segmentation_get_##what(int argc)                          \
  {                                                                     \
    long i, nsegments, npoints, dims[2];                                \
    img_segmentation_t *ws;                                             \
                                                                        \
    if (argc != 2) y_error("wrong number of arguments");                \
    i = ygets_l(0);                                                     \
    ws = yget_img_segmentation(1);                                      \
    nsegments = img_segmentation_get_number(ws);                        \
    if (i < 1) i += nsegments;                                          \
    if ((i < 1) || (i > nsegments)) y_error("out of bound index");      \
    --i; /* Yorick -> C indexing */                                     \
    npoints = img_segmentation_get_count(ws, i);                        \
    dims[0] = 1;                                                        \
    dims[1] = npoints;                                                  \
    img_segmentation_get_##what(ws, i, push(dims), npoints);            \
  }
BUILTIN(x, ypush_l)
BUILTIN(y, ypush_l)
BUILTIN(link, ypush_l)
#undef BUILTIN

void Y_img_segmentation_select(int argc)
{
  long j, number, dims[Y_DIMSIZE], *index, *temp;
  img_segmentation_t *src, *dst, **ptr;
  int temporary;

  /* Parse and check arguments. */
  if (argc != 2) y_error("wrong number of arguments");
  src = yget_img_segmentation(1);
  temporary = (yget_ref(0) < 0); /* topmost argument is temporary? */
  index = ygeta_l(0, &number, dims);

  /* Decrement Yorick index (YAPI warrants that there at least 8 free elements
     on top of the stack). */
  if (number == 1) {
    j = index[0] - 1;
    index = &j;
  } else if (temporary) {
    for (j = 0; j < number; ++j) {
      --index[j];
    }
  } else {
    dims[0] = 1;
    dims[1] = number;
    temp = ypush_l(dims);
    for (j = 0; j < number; ++j) {
      temp[j] = index[j] - 1;
    }
    index = temp;
  }

  /* Select sub-segmentation and make it into an opaque Yorick object. */
  dst = img_segmentation_select(src, index, number);
  if (dst == NULL) {
    int code = errno;
    if (code == ENOMEM) {
      y_error("insufficient memory");
    } else if (code == EINVAL) {
      y_error("out of range index");
    } else {
      y_error("unexpected error in img_segmentation_select()");
    }
  }
  ptr = ypush_obj(&ytype_img_segmentation, sizeof(void *));
  *ptr = img_segmentation_link(dst);
}

/*---------------------------------------------------------------------------*/
/* CHAINS OF IMAGE SEGMENTS */

static img_chainpool_t *yget_img_chainpool(int iarg);
static void yfree_img_chainpool(void *); /* do not call directly */

static y_userobj_t ytype_img_chainpool = {
  "img_chainpool", yfree_img_chainpool,
  NULL, NULL, NULL, NULL
};

static void yfree_img_chainpool(void *ptr)
{
  if ((ptr != NULL) && (*(void **)ptr != NULL)) {
    img_chainpool_destroy(*(img_chainpool_t **)ptr);
  }
}

static img_chainpool_t *yget_img_chainpool(int iarg)
{
  img_chainpool_t *ptr = YGET_OBJPTR(img_chainpool, iarg);
  if (ptr == NULL) {
    y_error("expecting an img_chainpool object");
  }
  return ptr;
}

void Y_img_chainpool_new(int argc)
{
  static char *knames[] = {"satol", "srtol", "drmin", "drmax",
                           "slope", "aatol", "artol", "prec",
                           "lmin", "lmax", NULL};
  static long kglobs[NUMBEROF(knames)];
  img_segmentation_t *sgm;
  img_chainpool_t *chn;
  double satol, srtol, drmin, drmax, slope, aatol, artol, prec;
  long lmin, lmax;
  int kiargs[NUMBEROF(knames) - 1], iarg, n;

  /* Parse arguments and keywords. */
  sgm = NULL;
  yarg_kw_init(knames, kglobs, kiargs);
  n = 0;
  for (iarg = argc - 1; iarg >= 0; --iarg) {
    iarg = yarg_kw(iarg, kglobs, kiargs);
    if (iarg < 0) break;
    if (n == 0) {
      sgm = yget_img_segmentation(iarg);
      ++n;
    } else {
      break;
    }
  }
  if (n != 1) {
    y_error("wrong number of arguments");
  }
#define GET_KEYWORD(n, param, defval, get, check, reason)       \
  if ((iarg = kiargs[n]) >= 0) {                                \
    param = get(iarg);                                          \
    if (!(check)) y_error(reason);                              \
  } else param = defval
  GET_KEYWORD(0, satol, 2.0, ygets_d, satol >= 0.0,
              "bad absolute size tolerance (SATOL)");
  GET_KEYWORD(1, srtol, 0.05, ygets_d, srtol >= 0.0,
              "bad relative size tolerance (SRTOL)");
  GET_KEYWORD(2, drmin, 0.4, ygets_d, drmin >= 0.0,
              "bad minimun relative distance between characters (DRMIN)");
  GET_KEYWORD(3, drmax, 2.5, ygets_d, drmax >= drmin,
              "bad maximum relative distance between characters (DRMAX)");
  GET_KEYWORD(4, slope, 0.3, ygets_d, slope >= 0.0 && slope <= 2.0,
              "bad value for maximum slope (SLOPE)");
  GET_KEYWORD(5, aatol, 2.0, ygets_d, aatol > 0.0,
              "bad absolute alignement tolerance (AATOL)");
  GET_KEYWORD(6, artol, 0.05, ygets_d, artol > 0.0,
              "bad relative alignement tolerance (ARTOL)");
  GET_KEYWORD(7, prec, 0.05, ygets_d, prec >= 0.0,
              "bad precision (PREC)");
  GET_KEYWORD(8, lmin, 3, ygets_l, lmin >= 2,
              "bad minimum number of characters per chain (LMIN)");
  GET_KEYWORD(9, lmax, 10, ygets_l, lmax >= lmin,
              "bad maximum number of characters per chain (LMAX)");
#undef GET_KEYWORD
  errno = 0;
  chn = img_chainpool_new(sgm, satol, srtol, drmin, drmax, slope,
                          aatol, artol, prec, lmin, lmax);
  if (chn == NULL) {
    if (errno == ENOMEM) {
      y_error("not enough memory");
    } else if (errno == EFAULT) {
      y_error("bad address");
    } else if (errno == EINVAL) {
      y_error("bad argument");
    } else {
      ypush_nil();
    }
  } else {
    YPUSH_OBJPTR(img_chainpool, chn);
  }
}

void Y_img_chainpool_get_number(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_chainpool_get_number(yget_img_chainpool(0)));
}

void Y_img_chainpool_get_image_width(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_chainpool_get_image_width(yget_img_chainpool(0)));
}

void Y_img_chainpool_get_image_height(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_long(img_chainpool_get_image_height(yget_img_chainpool(0)));
}

void Y_img_chainpool_get_segmentation(int argc)
{
  if (argc != 1) y_error("wrong number of arguments");
  ypush_img_segmentation(img_chainpool_get_segmentation(yget_img_chainpool(0)));
}

void Y_img_chainpool_get_segments(int argc)
{
  long j, length, nchains;
  long dims[2], *index;
  img_chainpool_t *chn;

  if (argc != 2) {
    y_error("wrong number of arguments");
  }
  chn = yget_img_chainpool(argc - 1);
  nchains = img_chainpool_get_number(chn);
  j = ygets_l(0);
  if (j < 1) j += nchains; /* Yorick convention */
  --j; /* Yorick -> C indexing */
  length = img_chainpool_get_length(chn, j);
  if (length <= 0) y_error("out of bound index");
  dims[0] = 1;
  dims[1] = length;
  index = ypush_l(dims);
  if (img_chainpool_get_segments(chn, j, index, length) != IMG_SUCCESS) {
    y_error("unexpected error (BUG?)");
  }
  for (j = 0; j < length; ++j) {
    ++index[j]; /* C -> Yorick indexing */
  }
}

#define BUILTIN(what, pushs, pusha)                                   \
void Y_img_chainpool_get_##what(int argc)                             \
{                                                                     \
  long nchains;                                                       \
  img_chainpool_t *chn;                                               \
                                                                      \
  if ((argc != 1) && (argc != 2)) {                                   \
    y_error("wrong number of arguments");                             \
  }                                                                   \
  chn = yget_img_chainpool(argc - 1);                                 \
  nchains = img_chainpool_get_number(chn);                            \
  if (argc == 2) {                                                    \
    long j = ygets_l(0);                                              \
    if (j < 1) j += nchains; /* Yorick convention */                  \
    if ((j < 1) || (j > nchains)) y_error("out of bound index");      \
    pushs(img_chainpool_get_##what(chn, j - 1));                      \
  } else {                                                            \
    long dims[2];                                                     \
    dims[0] = 1;                                                      \
    dims[1] = nchains;                                                \
    img_chainpool_get_##what##s(chn, pusha(dims), nchains);           \
  }                                                                   \
}
BUILTIN(vertical_shear, ypush_double, ypush_d)
BUILTIN(horizontal_shear, ypush_double, ypush_d)
BUILTIN(xmin, ypush_double, ypush_d)
BUILTIN(xmax, ypush_double, ypush_d)
BUILTIN(ymin, ypush_double, ypush_d)
BUILTIN(ymax, ypush_double, ypush_d)
BUILTIN(length, ypush_long, ypush_l)
#undef BUILTIN

/*---------------------------------------------------------------------------*/
/* UTILITIES */

extern void Y_img_get_version(int argc)
{
  if (argc != 1) {
    y_error("wrong number of arguments");
  }
  if (yarg_nil(0)) {
    char buffer[80];
    sprintf(buffer, "%d.%d.%d", IMG_VERSION_MAJOR,
            IMG_VERSION_MINOR, IMG_VERSION_PATCH);
    push_string(buffer);
  } else {
    long what = ygets_l(0);
    if (what == 0) {
      ypush_long(IMG_VERSION_MAJOR);
    } else if (what == 1) {
      ypush_long(IMG_VERSION_MINOR);
    } else if (what == 2) {
      ypush_long(IMG_VERSION_PATCH);
    } else {
      y_error("argument must be: nil, 0, 1 or 2");
    }
  }
}

extern void Y__img_init(int argc)
{
  volatile void *temp;
  int yor_ntypes, img_ntypes, i;

  temp = type_convert_table;
  type_convert_table = NULL;
  img_type_to_yor_type = NULL;
  yor_type_to_img_type = NULL;
  if (temp != NULL) {
    free((void *)temp);
  }

#define INIT_BOUND(name, value) \
  name##_min = name##_max = value
#define UPDATE_BOUND(name, value) \
  if (value < name##_min) name##_min = value; \
  if (value > name##_max) name##_max = value

  INIT_BOUND(yor_type, Y_CHAR);
  UPDATE_BOUND(yor_type, Y_SHORT);
  UPDATE_BOUND(yor_type, Y_INT);
  UPDATE_BOUND(yor_type, Y_LONG);
  UPDATE_BOUND(yor_type, Y_FLOAT);
  UPDATE_BOUND(yor_type, Y_DOUBLE);
  UPDATE_BOUND(yor_type, Y_COMPLEX);

  INIT_BOUND(img_type, IMG_TYPE_BYTE);
#ifdef IMG_TYPE_SHORT
  UPDATE_BOUND(img_type, IMG_TYPE_SHORT);
#endif
#ifdef IMG_TYPE_INT
  UPDATE_BOUND(img_type, IMG_TYPE_INT);
#endif
#ifdef IMG_TYPE_LONG
  UPDATE_BOUND(img_type, IMG_TYPE_LONG);
#endif
  UPDATE_BOUND(img_type, IMG_TYPE_FLOAT);
  UPDATE_BOUND(img_type, IMG_TYPE_DOUBLE);
  UPDATE_BOUND(img_type, IMG_TYPE_COMPLEX);
  UPDATE_BOUND(img_type, IMG_TYPE_RGB);
  UPDATE_BOUND(img_type, IMG_TYPE_RGBA);

#undef INIT_BOUND
#undef UPDATE_BOUND

  yor_ntypes = yor_type_max - yor_type_min + 1;
  img_ntypes = img_type_max - img_type_min + 1;

  type_convert_table = (int *)malloc((yor_ntypes + img_ntypes)*sizeof(int));
  if (type_convert_table == NULL) {
    y_error("insufficient memory for image type conversion table");
  }

  img_type_to_yor_type = type_convert_table - img_type_min;
  yor_type_to_img_type = type_convert_table + img_ntypes - yor_type_min;

  /* Initialize conversion tables with "bad" types. */
  for (i = img_type_min; i <= img_type_max; ++i) {
    img_type_to_yor_type[i] = yor_type_min - 1;
  }
  for (i = yor_type_min; i <= yor_type_max; ++i) {
    yor_type_to_img_type[i] = IMG_TYPE_NONE;
  }

  /* Set image to Yorick type conversion rules. */
#define CONVERT(itype, ytype)          \
  img_type_to_yor_type[itype] = ytype; \
  yor_type_to_img_type[ytype] = itype
  CONVERT(IMG_TYPE_BYTE, Y_CHAR);
#ifdef IMG_TYPE_SHORT
  CONVERT(IMG_TYPE_SHORT, Y_SHORT);
#endif
#ifdef IMG_TYPE_INT
  CONVERT(IMG_TYPE_INT, Y_INT);
#endif
#ifdef IMG_TYPE_LONG
  CONVERT(IMG_TYPE_LONG, Y_LONG);
#endif
  CONVERT(IMG_TYPE_FLOAT, Y_FLOAT);
  CONVERT(IMG_TYPE_DOUBLE, Y_DOUBLE);
  CONVERT(IMG_TYPE_COMPLEX, Y_COMPLEX);
#undef CONVERT
  img_type_to_yor_type[IMG_TYPE_RGB] = Y_CHAR;
  img_type_to_yor_type[IMG_TYPE_RGBA] = Y_CHAR;

#if 0 /* DEBUG */
#define PRT(expr) fprintf(stderr, "%20s = %3d\n", #expr, expr)
  PRT(CPT_INT8);
  PRT(CPT_UINT8);
  PRT(CPT_INT16);
  PRT(CPT_UINT16);
  PRT(CPT_INT32);
  PRT(CPT_UINT32);
  PRT(CPT_INT64);
  PRT(CPT_UINT64);
  PRT(CPT_FLOAT);
  PRT(CPT_DOUBLE);
  PRT(CPT_SCOMPLEX);
  PRT(CPT_DCOMPLEX);
# ifdef CPT_CHAR
  PRT(CPT_CHAR);
# endif
# ifdef CPT_UCHAR
  PRT(CPT_UCHAR);
# endif
# ifdef CPT_SHORT
  PRT(CPT_SHORT);
# endif
# ifdef CPT_USHORT
  PRT(CPT_USHORT);
# endif
# ifdef CPT_INT
  PRT(CPT_INT);
# endif
# ifdef CPT_UINT
  PRT(CPT_UINT);
# endif
# ifdef CPT_LONG
  PRT(CPT_LONG);
# endif
# ifdef CPT_ULONG
  PRT(CPT_ULONG);
# endif
# ifdef CPT_LLONG
  PRT(CPT_LLONG);
# endif
# ifdef CPT_ULLONG
  PRT(CPT_ULLONG);
# endif
  PRT(IMG_TYPE_INT8);
  PRT(IMG_TYPE_UINT8);
  PRT(IMG_TYPE_INT16);
  PRT(IMG_TYPE_UINT16);
  PRT(IMG_TYPE_INT32);
  PRT(IMG_TYPE_UINT32);
  PRT(IMG_TYPE_INT64);
  PRT(IMG_TYPE_UINT64);
  PRT(IMG_TYPE_FLOAT);
  PRT(IMG_TYPE_DOUBLE);
  PRT(IMG_TYPE_SCOMPLEX);
  PRT(IMG_TYPE_DCOMPLEX);
  PRT(IMG_TYPE_RGB);
  PRT(IMG_TYPE_RGBA);
  PRT(IMG_TYPE_BYTE);
# ifdef IMG_TYPE_SHORT
  PRT(IMG_TYPE_SHORT);
# endif
# ifdef IMG_TYPE_INT
  PRT(IMG_TYPE_INT);
# endif
# ifdef IMG_TYPE_LONG
  PRT(IMG_TYPE_LONG);
# endif
# ifdef IMG_TYPE_COMPLEX
  PRT(IMG_TYPE_COMPLEX);
# endif
#undef PRT

#define PRT(arr) fprintf(stderr, "%s[%d] = %3d\n", #arr, i, arr[i])
  for (i = img_type_min; i <= img_type_max; ++i) {
    PRT(img_type_to_yor_type);
  }
  for (i = yor_type_min; i <= yor_type_max; ++i) {
    PRT(yor_type_to_img_type);
  }
#undef PRT

#endif /* DEBUG */
}

static void push_string(const char *value)
{
  ypush_q((long *)NULL)[0] = (value ? p_strcpy((char *)value) : NULL);
}

static void get_image(int iarg, image_t *img)
{
  long rank, dims[Y_DIMSIZE];
  int yor_type, img_type;

  img->data = ygeta_any(iarg, NULL, dims, &yor_type);
  if ((yor_type >= yor_type_min) && (yor_type <= yor_type_max)) {
    img_type = yor_type_to_img_type[yor_type];
  } else {
    img_type = IMG_TYPE_NONE;
  }
  if (img_type == IMG_TYPE_NONE) {
    y_error("bad data type for an image -- type \"help, is_image;\" for more details");
  }
  rank = dims[0];
  if (rank == 2) {
    img->width = dims[1];
    img->height = dims[2];
    img->type = img_type;
    return;
  } else if ((rank == 3) && (img_type == IMG_TYPE_BYTE)) {
    img->width = dims[2];
    img->height = dims[3];
    if (dims[1] == 3) {
      img->type = IMG_TYPE_RGB;
      return;
    } else if (dims[1] == 4) {
      img->type = IMG_TYPE_RGBA;
      return;
    }
  }
  y_error("bad dimensions for an image -- type \"help, is_image;\" for more details");
}

static void new_image(image_t *img)
{
  long dims[4];
  int img_type, yor_type;

  img_type = img->type;
  if ((img_type >= img_type_min) && (img_type <= img_type_max)) {
    yor_type = img_type_to_yor_type[img_type];
  } else {
    yor_type = yor_type_min - 1;
  }
  switch (yor_type) {
  case Y_CHAR:
    if (img_type == IMG_TYPE_RGB) {
      dims[0] = 3;
      dims[1] = 3;
      dims[2] = img->width;
      dims[3] = img->height;
    } else if (img_type == IMG_TYPE_RGBA) {
      dims[0] = 3;
      dims[1] = 4;
      dims[2] = img->width;
      dims[3] = img->height;
    } else {
      dims[0] = 2;
      dims[1] = img->width;
      dims[2] = img->height;
    }
    img->data = (void *)ypush_c(dims);
    break;
#define NEW2(pusher)                              \
    dims[0] = 2;                                  \
    dims[1] = img->width;                         \
    dims[2] = img->height;                        \
    img->data = (void *)pusher(dims);             \
    break
  case Y_SHORT:   NEW2(ypush_s);
  case Y_INT:     NEW2(ypush_i);
  case Y_LONG:    NEW2(ypush_l);
  case Y_FLOAT:   NEW2(ypush_f);
  case Y_DOUBLE:  NEW2(ypush_d);
  case Y_COMPLEX: NEW2(ypush_z);
#undef NEW2
  default:
    y_error("bad pixel type");
  }
}

static void convert_image(int iarg, image_t *img, int new_img_type)
{
  int old_img_type = img->type;
  const void *old_data = img->data;
  img->type = new_img_type;
  new_image(img);
  if (img_copy(img->width, img->height,
               old_data, old_img_type, 0, img->width,
               img->data, img->type, 0, img->width) != IMG_SUCCESS) {
    y_error("incompatible pixel types");
  }
  yarg_swap(iarg + 1, 0);
  yarg_drop(1);
}

#define TBL_ROW(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15) \
            {IMG_TYPE_##a01, IMG_TYPE_##a02, IMG_TYPE_##a03, IMG_TYPE_##a04, \
             IMG_TYPE_##a05, IMG_TYPE_##a06, IMG_TYPE_##a07, IMG_TYPE_##a08, \
             IMG_TYPE_##a09, IMG_TYPE_##a10, IMG_TYPE_##a11, IMG_TYPE_##a12, \
             IMG_TYPE_##a13, IMG_TYPE_##a14, IMG_TYPE_##a15}

static const int binop_type_table[15][15] = {
  /*        NONE,     INT8,     UINT8,    INT16,    UINT16,
   *        INT32,    UINT32,   INT64,    UINT64,   FLOAT,
   *        DOUBLE,   SCOMPLEX, DCOMPLEX  RGB,      RGBA   */
  /* NONE */
  TBL_ROW(NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     NONE,     NONE),
  /* INT8 */
  TBL_ROW(NONE,     INT8,     INT8,     INT16,    INT16,
          INT32,    INT32,    INT64,    INT64,    FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* UINT8 */
  TBL_ROW(NONE,     INT8,     UINT8,    INT16,    UINT16,
          INT32,    UINT32,   INT64,    UINT64,   FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* INT16 */
  TBL_ROW(NONE,     INT16,    INT16,    INT16,    INT16,
          INT32,    INT32,    INT64,    INT64,    FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* UINT16 */
  TBL_ROW(NONE,     INT16,    UINT16,   INT16,    UINT16,
          INT32,    UINT32,   INT64,    UINT64,   FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* INT32 */
  TBL_ROW(NONE,     INT32,    INT32,    INT32,    INT32,
          INT32,    INT32,    INT64,    INT64,    FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* UINT32 */
  TBL_ROW(NONE,     INT32,    UINT32,   INT32,    UINT32,
          INT32,    UINT32,   INT64,    UINT64,   FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* INT64 */
  TBL_ROW(NONE,     INT64,    INT64,    INT64,    INT64,
          INT64,    INT64,   INT64,     INT64,    FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* UINT64 */
  TBL_ROW(NONE,     INT64,    UINT64,   INT64,    UINT64,
          INT64,    UINT64,   INT64,    UINT64,   FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* FLOAT */
  TBL_ROW(NONE,     FLOAT,    FLOAT,    FLOAT,    FLOAT,
          FLOAT,    FLOAT,    FLOAT,    FLOAT,    FLOAT,
          DOUBLE,   SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* DOUBLE */
  TBL_ROW(NONE,     DOUBLE,   DOUBLE,   DOUBLE,   DOUBLE,
          DOUBLE,   DOUBLE,   DOUBLE,   DOUBLE,   DOUBLE,
          DOUBLE,   DCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* SCOMPLEX */
  TBL_ROW(NONE,     SCOMPLEX, SCOMPLEX, SCOMPLEX, SCOMPLEX,
          SCOMPLEX, SCOMPLEX, SCOMPLEX, SCOMPLEX, SCOMPLEX,
          DCOMPLEX, SCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* DCOMPLEX */
  TBL_ROW(NONE,     DCOMPLEX, DCOMPLEX, DCOMPLEX, DCOMPLEX,
          DCOMPLEX, DCOMPLEX, DCOMPLEX, DCOMPLEX, DCOMPLEX,
          DCOMPLEX, DCOMPLEX, DCOMPLEX, NONE,     NONE),
  /* RGB */
  TBL_ROW(NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     RGB,      RGBA),
  /* RGBA */
  TBL_ROW(NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     NONE,     NONE,
          NONE,     NONE,     NONE,     RGBA,     RGBA),
};

#undef TBL_ROW

static int get_binop_type(int left_type, int right_type)
{
  if ((left_type < IMG_TYPE_MIN) || (left_type > IMG_TYPE_MAX) ||
      (right_type < IMG_TYPE_MIN) || (right_type > IMG_TYPE_MAX)) {
    return IMG_TYPE_NONE;
  }
  return binop_type_table[left_type][right_type];
}

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
