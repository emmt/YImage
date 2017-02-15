/*
 * img_segment.c --
 *
 * Implementation of image segmentation and chaining of segments.
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

#ifndef _IMG_SEGMENT_C
#define _IMG_SEGMENT_C 1

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#include "c_pseudo_template.h"
#include "img.h"
#include "itemstack.h"
#include "itempool.h"

#define link_t img_link_t

#ifndef NULL
# define NULL ((void*)0L)
#endif

#ifdef DEBUG
# undef NDEBUG
#endif /* DEBUG */
#ifdef NDEBUG
# undef DEBUG
#endif /* NDEBUG */

#define TRUE     (1)
#define FALSE    (0)

#define SUCCESS  (0)
#define FAILURE (-1)

#define OFFSET_OF(TYPE, MEMBER)      ((char *)&((TYPE *)0)->MEMBER - (char *)0)
#define NEW_ARRAY(TYPE, NUMBER)      ((TYPE *)malloc((NUMBER)*sizeof(TYPE)))
#define NEW_ARRAY_ZERO(TYPE, NUMBER) ((TYPE *)calloc((NUMBER), sizeof(TYPE)))

#define STRINGIFY(x)  STRINGIFY1(x)
#define STRINGIFY1(x) # x

/* Assertion. */
#ifdef DEBUG
# define ASSERT(expr, final)                                            \
      if (expr) ;                                                       \
      else do {                                                         \
          fprintf(stderr, "assertion failed: %s\n in function %s,"      \
                  " file \"%s\", line %d\n", STRINGIFY(expr),           \
                  __FUNCTION__, __FILE__, __LINE__);                    \
          final;                                                        \
        } while (0)
static int failure(int code)
{
  errno = code;
  return FAILURE;
}
#else
# define ASSERT(expr, final)
#endif

#ifdef DEBUG
# define DEBUG_INFO(msg) fprintf(stderr, msg " in \"%s\" at line %d\n", \
                                 __FILE__, __LINE__)
#else
# define DEBUG_INFO(msg)
#endif

/*---------------------------------------------------------------------------*/
/* Definitions that will be expanded by the template code. */

#define BUILD_LINKS(TYPE)  CPT_JOIN2(build_links_, CPT_ABBREV(TYPE))
#define pixel_t            CPT_CTYPE(TYPE)


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

/*---------------------------------------------------------------------------*/
/* Private structures. */

typedef struct _bbox      bbox_t;
typedef struct _point     point_t;
typedef struct _segment   segment_t;
typedef struct _chainable chainable_t;
typedef struct _chainlink chainlink_t;
typedef struct _chain     chain_t;

struct _bbox {
  double xmin, xmax, ymin, ymax;
};

struct _point {
  uint16_t link;
  int16_t x, y;
};

/* Members common to all "chainable" items. */
#define CHAINABLE_MEMBERS                                               \
  long level;             /* = 0 for segments, >= 1 for links */        \
  long nparents;          /* number of parents */                       \
  chainlink_t *first_link /* first link that has this item as left child */

struct _chainable {
  CHAINABLE_MEMBERS;
};

struct _segment {
  /* Members common to all chainable elements. */
  CHAINABLE_MEMBERS;

  /* Members specific to segments. */
  double xcen, ycen; /* coordinate of center */
  point_t *point; /* coordinate of pixels */
  long count; /* number of pixels */
  long xmin, xmax, ymin, ymax; /* bounding box */
  long width, height; /* width and height of bounding box */
};

struct _chainlink {
  /* Members common to all chainable elements. */
  CHAINABLE_MEMBERS;

  /* Members specific to chainlinks. */
  chainlink_t *next;        /* next one in the list of all chain-links */
  chainlink_t *next_link;   /* next link that shares the same left child */
  chainable_t *left_child;  /* left chainable item */
  chainable_t *right_child; /* right chainable item */
  segment_t *first;         /* first segment in the chain defined by this item */
  segment_t *last;          /* last segment in the chain defined by this item */
};

struct _chain {
  double vertical_shear, horizontal_shear;
  double xmin, xmax, ymin, ymax;
  double a[4];
  long length;
  chain_t *next;
  segment_t *segment[1]; /* actual size is sufficient for LENGTH segments */
};

static void get_bbox(bbox_t *bbox, const segment_t *s, const double a[]);

/*---------------------------------------------------------------------------*/
/* Private data. */

/* A private stack of items is used to manage temporary memory needed
   by the various routines. This makes the code robust to interrupts (for
   instance when linking with Yorick) and easier to write avoiding memory
   leaks but this is not thread safe (for the moment). */

static itemstack_t *stack = NULL;
static void *clear_and_pop(itemstack_t *stack);

#define DUMP_STACK(n)       itemstack_dump(stack, (n), stderr)
#define CLEAR_STACK(nil)    itemstack_drop(stack, -1L)
#define POP_STACK(nil)      clear_and_pop(stack)

#define SETUP_STACK(errcode)                    \
  if (stack == NULL) {                          \
    stack = itemstack_new(0);                   \
    if (stack == NULL) return errcode;          \
  } else CLEAR_STACK()

#define PUSH_NEW_ARRAY(TYPE, NUMBER)  \
  ((TYPE *)itemstack_push_dynamic(stack, (NUMBER)*sizeof(TYPE), 0))

#define PUSH_NEW_ARRAY_ZERO(TYPE, NUMBER)  \
  ((TYPE *)itemstack_push_dynamic(stack, (NUMBER)*sizeof(TYPE), 1))

typedef void destroy_t(void *);

#define PUSH_ITEM(ITEM, DESTROY)  itemstack_push(stack, ITEM, DESTROY)
#define DROP_ITEM(NUMBER)         itemstack_drop(stack, NUMBER)

/*---------------------------------------------------------------------------*/
/* High level segmentation. */

struct _img_segmentation {
  int nrefs;          /* number of references on this object */
  segment_t *segment; /* array of segments */
  long number;        /* number of segments */
  long width, height; /* size of the source image */
  point_t point[1];   /* points in all segments */
};

static img_segmentation_t *allocate(long npoints, long nsegments);

img_segmentation_t *img_segmentation_new(const void *img,
					 const int type,
					 const long offset,
					 const long width,
					 const long height,
					 const long stride,
					 const double threshold)
{
  img_segmentation_t *ws;
  link_t *link;
  point_t *point;
  long i, j, nsegments, npixels;
  long *region, *index;
  int status;

  /* Setup memory managment. */
  SETUP_STACK(NULL);
  ws = NULL;

  /* Allocate enough space to index the maximum number of regions (this is
     done before the "link" array to allow for dropping this later when no
     longer needed). */
  npixels = width*height;
  index = PUSH_NEW_ARRAY(long, 2*npixels);
  if (index == NULL) {
    goto done;
  }

  /* Build the links of the pixels. */
  link = PUSH_NEW_ARRAY(link_t, npixels);

#define CASE(TYPE)                                                      \
  case IMG_TYPE_##TYPE:                                                 \
    status = BUILD_LINKS(TYPE)((const CPT_CTYPE(TYPE) *)img, offset,    \
                               stride, link, 0, width, width, height,   \
                               threshold);                              \
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
    status = IMG_FAILURE;
  }
  if (status != IMG_SUCCESS) {
    goto done;
  }

#undef CASE

  /* Allocate a new image segmentation opaque object. */
  ws = allocate(npixels, 0);
  if (ws == NULL) {
    goto done;
  }
  ws->width = width;
  ws->height = height;

  /* This macro stores IDX-th pixel into current segment. */
#define STORE(IDX)				\
  point->link = link[IDX];			\
  point->x = (IDX)%width;			\
  point->y = (IDX)/width;			\
  ++point;					\
  link[IDX] |= OWNED;				\
  region[++size] = (IDX)

  /* This macro stores IDX-th pixel into current segment if current
     pixel is linked to direction DIR. */
#define CHECK(DIR, IDX)				\
  if ((s & (DIR)) != 0) {			\
    long _i = (IDX);				\
    if ((link[_i] & OWNED) == 0) {		\
      STORE(_i);                                \
    }						\
  }

  /* Build the segments. */
#define OWNED  IMG_LINK_OWNED
  point = ws->point;
  nsegments = 0;
  region = index;
  for (i = 0; i < npixels; ++i) {
    /* Check whether current pixel is not already owned by a segment. */
    if ((link[i] & OWNED) == 0) {
      long size = 0;
      STORE(i);
      for (j = 1; j <= size; ++j) {
        long k = region[j];
        int s = link[k];
        CHECK(IMG_LINK_WEST,  k - 1);
	CHECK(IMG_LINK_EAST,  k + 1);
	CHECK(IMG_LINK_SOUTH, k - width);
	CHECK(IMG_LINK_NORTH, k + width);
      }
      region[0] = size; /* store the number of pixels in the region */
      region += (size + 1); /* move to base of next region */
      ++nsegments;
    }
  }

#undef CHECK
#undef STORE
#undef OWNED

  /* Free no longer needed "link" array. */
  DROP_ITEM(1);

  /* Create the image segmentation object. */
  if (nsegments > 0) {
    segment_t *segment;
    segment = NEW_ARRAY_ZERO(segment_t, nsegments);
    if (segment == NULL) {
      img_segmentation_unlink(ws);
      ws = NULL;
      goto done;
    }
    ws->segment = segment;
    ws->number = nsegments;
    point = ws->point;
    region = index;
    for (i = 0; i < nsegments; ++i) {
      int16_t x, xmin, xmax, y, ymin, ymax;
      long count = region[0]; /* number of elements in the current region */
      xmin = xmax = point[0].x;
      ymin = ymax = point[0].y;
      for (j = 1; j < count; ++j) {
	x = point[j].x;
	if (x < xmin) xmin = x;
	if (x > xmax) xmax = x;
	y = point[j].y;
	if (y < ymin) ymin = y;
	if (y > ymax) ymax = y;
      }
      ws->segment[i].xcen = (xmin + xmax)*0.5;
      ws->segment[i].ycen = (ymin + ymax)*0.5;
      ws->segment[i].point = point;
      ws->segment[i].count = count;
      ws->segment[i].xmin = xmin;
      ws->segment[i].xmax = xmax;
      ws->segment[i].ymin = ymin;
      ws->segment[i].ymax = ymax;
      ws->segment[i].width = xmax - xmin + 1;
      ws->segment[i].height = ymax - ymin + 1;
      point += count;
      region += count + 1;
    }
  }

  /* Free all stacked memory blocks and return the result. */
 done:
  CLEAR_STACK();
  return ws;
}

int img_segmentation_get_nrefs(img_segmentation_t *ws)
{
  return ((ws != NULL) ? ws->nrefs : -1);
}

img_segmentation_t *img_segmentation_link(img_segmentation_t *ws)
{
  if (ws != NULL) {
    ++ws->nrefs;
  }
  return ws;
}

void img_segmentation_unlink(img_segmentation_t *ws)
{
  if ((ws != NULL) && (--ws->nrefs <= 0)) {
    if (ws->segment != NULL) {
      free(ws->segment);
    }
    free(ws);
  }
}

img_segmentation_t *img_segmentation_select(const img_segmentation_t *src,
					    const long list[],
					    const long number)
{
  long i, j, nsegments, npoints;
  segment_t *segment;
  point_t *point;
  img_segmentation_t *dst;

  /* Check arguments and indices of segments to select. */
  if ((src == NULL) || (list == NULL)) {
    errno = EFAULT;
    return NULL;
  }
  if (number <= 0) {
    errno = EINVAL;
    return NULL;
  }
  npoints = 0;
  nsegments = src->number;
  segment = src->segment;
  for (i = 0; i < number; ++i) {
    j = list[i];
    if ((j < 0) || (j >= nsegments)) {
      errno = EINVAL;
      return NULL;
    }
    npoints += segment[j].count;
  }

  /* Build the copy. */
  dst = allocate(npoints, number);
  if (dst == NULL) {
    return NULL;
  }
  dst->width = src->width;
  dst->height = src->height;
  point = dst->point;
  for (i = 0; i < number; ++i) {
    j = list[i];
    npoints = src->segment[j].count;
    dst->segment[i] = src->segment[j];
    dst->segment[i].point = memcpy(point, src->segment[j].point,
				   npoints*sizeof(point_t));
    point += npoints;
  }
  return dst;
}

static img_segmentation_t *allocate(long npoints, long nsegments)
{
  img_segmentation_t *ws;
  size_t nbytes;

  nbytes = OFFSET_OF(img_segmentation_t, point) + npoints*sizeof(point_t);
  ws = (img_segmentation_t *)malloc(nbytes);
  if (ws == NULL) {
    return NULL;
  }
  ws->nrefs = 0;
  if (nsegments > 0) {
    ws->segment = NEW_ARRAY_ZERO(segment_t, nsegments);
    if (ws->segment == NULL) {
      free(ws);
      return NULL;
    }
    ws->number = nsegments;
  } else {
    ws->segment = NULL;
    ws->number = 0;
  }
  ws->width = 0;
  ws->height = 0;
  return ws;
}

/**
 * @brief Get the number of segments in an image segmentation.
 *
 * @param sgm   The address of the image segmentation instance.
 *
 * @return The the number of segments in \a sgm.  On error, \c 0 is returned
 *         and \c errno is set.
 */
long img_segmentation_get_number(img_segmentation_t *sgm)
{
  if (sgm == NULL) {
    errno = EFAULT;
    return 0;
  }
  return sgm->number;
}

/**
 * @brief Get the width of the image from its segmentation.
 *
 * @param sgm   The address of the image segmentation instance.
 *
 * @return The width of the image from wich the segmentation was built.  On
 *         error, \c 0 is returned and \c errno is set.
 */
long img_segmentation_get_image_width(img_segmentation_t *sgm)
{
  if (sgm == NULL) {
    errno = EFAULT;
    return 0;
  }
  return sgm->width;
}

/**
 * @brief Get the height of the image from its segmentation.
 *
 * @param sgm   The address of the image segmentation instance.
 *
 * @return The height of the image from wich the segmentation was built.  On
 *         error, \c 0 is returned and \c errno is set.
 */
long img_segmentation_get_image_height(img_segmentation_t *sgm)
{
  if (sgm == NULL) {
    errno = EFAULT;
    return 0;
  }
  return sgm->height;
}

#define GET_MEMBER(type, what)                                  \
                                                                \
int img_segmentation_get_##what##s(img_segmentation_t *ws,      \
                                   type what[], long number)    \
{                                                               \
  long j;                                                       \
  segment_t *segment;                                           \
                                                                \
  if ((ws == NULL) || (what == NULL)) {                         \
    errno = EFAULT;                                             \
    return IMG_FAILURE;                                         \
  }                                                             \
  if (number != ws->number) {                                   \
    errno = EINVAL;                                             \
    return IMG_FAILURE;                                         \
  }                                                             \
  segment = ws->segment;                                        \
  for (j = 0; j < number; ++j) {                                \
    what[j] = segment[j].what;                                  \
  }                                                             \
  return IMG_SUCCESS;                                           \
}                                                               \
                                                                \
type img_segmentation_get_##what(img_segmentation_t *ws,        \
                                 long j)                        \
{                                                               \
  if (ws == NULL) {                                             \
    errno = EFAULT;                                             \
    return 0;                                                   \
  }                                                             \
  if ((j < 0) || (j >= ws->number)) {                           \
    errno = EINVAL;                                             \
    return 0;                                                   \
  }                                                             \
  return ws->segment[j].what;                                   \
}

GET_MEMBER(double, xcen)
GET_MEMBER(double, ycen)
GET_MEMBER(long, count)
GET_MEMBER(long, xmin)
GET_MEMBER(long, xmax)
GET_MEMBER(long, ymin)
GET_MEMBER(long, ymax)
GET_MEMBER(long, width)
GET_MEMBER(long, height)

#undef GET_MEMBER

#define GET_MEMBER(type, what)                          \
int img_segmentation_get_##what(img_segmentation_t *ws, \
                                const long i,           \
                                type what[],            \
                                const long number)      \
{                                                       \
  long j;                                               \
  segment_t *s;                                         \
  point_t *p;                                           \
                                                        \
  if ((ws == NULL) || (what == NULL)) {                 \
    errno = EFAULT;                                     \
    return IMG_FAILURE;                                 \
  }                                                     \
  if ((i < 0) || (i >= ws->number)) {                   \
    errno = EINVAL;                                     \
    return IMG_FAILURE;                                 \
  }                                                     \
  s = &ws->segment[i];                                  \
  if (number != s->count) {                             \
    errno = EINVAL;                                     \
    return IMG_FAILURE;                                 \
  }                                                     \
  p = s->point;                                         \
  for (j = 0; j < number; ++j) {                        \
    what[j] = p[j].what;                                \
  }                                                     \
  return IMG_SUCCESS;                                   \
}

GET_MEMBER(long, x)
GET_MEMBER(long, y)
GET_MEMBER(long, link)

#undef GET_MEMBER

/*---------------------------------------------------------------------------*/
/* Chaining of segments. */

struct _img_chainpool {
  long nchains; /* number of chains in the pool */
  img_segmentation_t *segmentation; /* the image segmentation */
  chain_t *chain[1]; /* actual size is sufficient for NCHAINS segments */
};

/**
 * @brief Destroy a pool of chained image segments.
 *
 * This function releases all ressources associated with the pool \a chn of
 * chained image segments.  The pool \a chn of chained image segments must no
 * be used after calliong this function.
 *
 * @param chn  The address of a pool of chained image segments (can be \c
 *             NULL).
 *
 * @see img_chainpool_new().
 */
void img_chainpool_destroy(img_chainpool_t *chn)
{
  long k;

  if (chn != NULL) {
    if (chn->segmentation != NULL) {
      img_segmentation_unlink(chn->segmentation);
    }
    for (k = 0; k < chn->nchains; ++k) {
      if (chn->chain[k] != NULL) {
        free(chn->chain[k]);
      }
    }
    free(chn);
  }
}

/**
 * @brief Get the number of chains in a pool of chained image segments.
 *
 * @param chn  The address of a pool of chained image segments.
 *
 * @return The number of chains in the pool of chained image segments.
 *
 * @see img_chainpool_new().
 */
long img_chainpool_get_number(img_chainpool_t *chn)
{
  if (chn != NULL) return chn->nchains;
  errno = EFAULT;
  return 0;
}

/**
 * @brief Get the width of the image from its chainpool.
 *
 * @param chn   The address of the chainpool instance.
 *
 * @return The width of the image from wich the chainpool was built.  On
 *         error, \c 0 is returned and \c errno is set.
 */
long img_chainpool_get_image_width(img_chainpool_t *chn)
{
  if (chn == NULL) {
    errno = EFAULT;
    return 0;
  }
  return chn->segmentation->width;
}

/**
 * @brief Get the height of the image from its chainpool.
 *
 * @param chn   The address of the chainpool instance.
 *
 * @return The height of the image from wich the chainpool was built.  On
 *         error, \c 0 is returned and \c errno is set.
 */
long img_chainpool_get_image_height(img_chainpool_t *chn)
{
  if (chn == NULL) {
    errno = EFAULT;
    return 0;
  }
  return chn->segmentation->height;
}

/**
 * @brief Get the address of the image segmentation used by a pool of chained
 *        image segments.
 *
 * @param chn  The address of a pool of chained image segments.
 *
 * @return The address of the image segmentation used by the pool of chained
 *         image segments.
 *
 * @see img_chainpool_new().
 */
img_segmentation_t *img_chainpool_get_segmentation(img_chainpool_t *chn)
{
  if (chn != NULL) return chn->segmentation;
  errno = EFAULT;
  return NULL;
}

#define GET_MEMBER(type, what)                          \
                                                        \
type img_chainpool_get_##what(img_chainpool_t *chn,     \
                              long j)                   \
{                                                       \
  if (chn == NULL) {                                    \
    errno = EFAULT;                                     \
    return 0;                                           \
  }                                                     \
  if ((j < 0) || (j >= chn->nchains)) {                 \
    errno = EINVAL;                                     \
    return 0;                                           \
  }                                                     \
  return chn->chain[j]->what;                           \
}                                                       \
                                                        \
int img_chainpool_get_##what##s(img_chainpool_t *chn,   \
                                type what[], long n)    \
{                                                       \
  long j;                                               \
  chain_t **chain;                                      \
                                                        \
  if ((chn == NULL) || ((what == NULL) && (n != 0))) {  \
    errno = EFAULT;                                     \
    return IMG_FAILURE;                                 \
  }                                                     \
  if (n != chn->nchains) {                              \
    errno = EINVAL;                                     \
    return IMG_FAILURE;                                 \
  }                                                     \
  chain = chn->chain;                                   \
  for (j = 0; j < n; ++j) {                             \
    what[j] = chain[j]->what;                           \
  }                                                     \
  return IMG_SUCCESS;                                   \
}

GET_MEMBER(double, vertical_shear)
GET_MEMBER(double, horizontal_shear)
GET_MEMBER(double, xmin)
GET_MEMBER(double, xmax)
GET_MEMBER(double, ymin)
GET_MEMBER(double, ymax)
GET_MEMBER(long, length)
#undef GET_MEMBER

/**
 * @brief Get the indices of the segments of a chain.
 *
 * @param chn    The address of a pool of chained image segments.
 * @param j      The index of the chain to consider.
 * @param list   The output array to store the segment indices.
 * @param n      The number of elements in \a list, must match the length
 *               of the chain to extract, \e cf. img_chainpool_get_length().
 *
 * @return \c IMG_SUCCESS or \c IMG_FAILURE.
 */
int img_chainpool_get_segments(img_chainpool_t *chn,
                               long j, long list[], long n)
{
  chain_t *chain;
  segment_t *s0, **s1;

  if ((chn == NULL) || ((list == NULL) && (n != 0))) {
    errno = EFAULT;
    return IMG_FAILURE;
  }
  if ((j < 0) || (j >= chn->nchains)) {
    errno = EINVAL;
    return IMG_FAILURE;
  }
  chain = chn->chain[j];
  if (n != chain->length) {
    errno = EINVAL;
    return IMG_FAILURE;
  }

  s0 = chn->segmentation->segment;
  s1 = chain->segment;
  for (j = 0; j < n; ++j) {
    list[j] = s1[j] - s0;
  }
  return IMG_SUCCESS;
}

/* Structure to store information about a line made of segments. */
typedef struct short_line_ short_line_t;
struct short_line_ {
  double sh, sx, sy, sxx, sxy;
  long length;
  segment_t **list;
};
static void short_line_init(short_line_t *line,
                            segment_t **list,
                            long length);
static int short_line_accept(const short_line_t *line,
                             const segment_t *sgm,
                             double slope,
                             double aatol,
                             double artol);
static int fit_vertical_shear(chain_t *chain, double prec);
static int fit_horizontal_shear(chain_t *chain, double prec);
static int fit_line(double sw,
                    double swx,
                    double swy,
                    double swxx,
                    double swxy,
                    double *xm,
                    double *ym,
                    double *alpha);

static void sort_segments(segment_t *obj[], size_t n);

#define HEAPSORT_SCOPE          static
#define HEAPSORT_FUNCTION       sort_segments
#define HEAPSORT_OBJ_TYPE       segment_t *
#define HEAPSORT_KEY_TYPE       double
#define HEAPSORT_GET_KEY(obj)   ((obj)->xcen)
#include "heapsort.h"

static int chainlink_insert(chainlink_t **list,
                            itempool_t   *pool,
                            chainable_t  *left,
                            chainable_t  *right);

/**
 * @brief Build chains of image segments.
 *
 * @param sgm     The address of the image segmentation,
 * @param satol   The absolute tolerance for size of characters.
 * @param srtol   The relative tolerance for size of characters.
 * @param drmin   The minimun relative distance between characters.
 * @param drmax   The maximun relative distance between characters.
 * @param slope   The maximum slope of a chain with respect to horizontal
 *                direction.
 * @param aatol   The absolute alignement tolerance of segments.
 * @param artol   The alignement tolerance of segments relative to their
 *                heights.
 * @param prec    The precision for estimating the shears of the chains.
 * @param lmin    The minimum length of the chains.
 * @param lmax    The maximum length of the chains.
 *
 * @return The address of a new pool of chains of image segments; \c NULL in
 *         case of error.
 *
 * @see img_chainpool_destroy(), img_chainpool_get_segmentation(),
 *      img_chainpool_get_number().
 */
img_chainpool_t *img_chainpool_new(img_segmentation_t *sgm,
                                   double satol,
                                   double srtol,
                                   double drmin,
                                   double drmax,
                                   double slope,
                                   double aatol,
                                   double artol,
                                   double prec,
                                   long lmin,
                                   long lmax)
{
  double sa, sq, sr, rmin, rmax;
  long j, jleft, jright;
  long count, length, level, nsegments, nchains;
  chainlink_t* top;
  segment_t** segment_list;
  img_chainpool_t* chainpool;
  chainlink_t* first;
  itempool_t* itempool;
  size_t nbytes;
  int pass;

  /* Check/fix arguments. */
  if (sgm == NULL) {
    errno = EFAULT;
    return NULL;
  }
  if (srtol < 0.0) {
    srtol = 0.0;
  }
  if (srtol > 1.0) {
    srtol = 1.0;
  }
  if (satol < 0.0) {
    satol = 0.0;
  }
  if (drmin < 0.0) {
    drmin = 0.0;
  }
  if (drmax < 0.0) {
    drmax = 0.0;
  }
  if (drmax < drmin) {
    double temp = drmax;
    drmax = drmin;
    drmin = temp;
  }
  if (slope < 0.0) {
    slope = 0.0;
  }
  if (aatol < 0.0) {
    aatol = 0.0;
  }
  if (artol < 0.0) {
    artol = 0.0;
  }
  if (prec < 0.0) {
    prec = 0.0;
  }
  sa = 1.0 + 2.0*satol;
  sq = 2.0 - srtol;
  sr = 2.0 + srtol;
  rmin = 0.5*drmin;
  rmax = 0.5*drmax;

  /* Setup memory managment. */
  SETUP_STACK(NULL);
  first = NULL;
  chainpool = NULL;
  itempool = itempool_new(sizeof(chainlink_t), 20);
  if ((itempool == NULL) ||
      (PUSH_ITEM((void *)itempool,
                 (destroy_t *)itempool_destroy) != ITEMSTACK_SUCCESS)) {
    DEBUG_INFO("failure");
    goto failure;
  }

  /* Allocate an array of segment pointers and sort segments by increasing X
     coordinate. */
  nsegments = sgm->number;
  segment_list = PUSH_NEW_ARRAY(segment_t *, nsegments);
  if (segment_list == NULL) {
    DEBUG_INFO("failure");
    goto failure;
  }
  for (j = 0; j < nsegments; ++j) {
    segment_list[j] = &sgm->segment[j];
  }
  sort_segments(segment_list, nsegments);

  /* Create the 1st level links between pairs of segments. */
  count = 0;
  for (jleft = 0; jleft < nsegments; ++jleft) {
    segment_t *left = segment_list[jleft];
    double h0 = left->height;
    double w0 = left->width;
    double x0 = left->xcen;
    double y0 = left->ycen;
    double hmin = (sq*h0 - sa)/sr;
    double hmax = (sr*h0 + sa)/sq;
    double xmax = x0 + rmax*(h0 + hmax);

    for (jright = jleft + 1; jright < nsegments; ++jright) {
      /* Select all potential next segments.  To speed-up the search, the most
         selective tests and which do involve the least computations are tried
         first. */
      segment_t *right = segment_list[jright];
      double x1, y1, h1, w1, delta_x;

      /* Check whether the next character is not too far. */
      x1 = right->xcen;
      if (x1 >= xmax) {
        /* No other character is allowed beyond this limit since the segments
           are ordered with ascending abscissa. */
        break;
      }

      /* Check whether the height is in the (exclusive) range. */
      h1 = right->height;
      if (h1 <= hmin || h1 >= hmax) {
        continue;
      }

      /* Check whether the slope is not too important. */
      y1 = right->ycen;
      if (fabs(y1 - y0) > slope*fabs(x1 - x0)) {
        continue;
      }

      /* Check whether the abscissa of the next character is in the allowed
         range. */
      w1 = right->width;
      delta_x = x1 - x0;
      if (delta_x < 1.0 + rmin*(w0 + w1) || delta_x > rmax*(h0 + h1)) {
        continue;
      }

      /* The potential RIGHT character must not be aligned with any of the
         existing successors of the LEFT character.  This is to avoid
         "jumping" over one character unless really needed.  The algorithm
         works because segments are sorted in ascending abscissa order; hence
         the closest characters are tried first. */
      if (left->first_link != NULL) {
        short_line_t line;
        segment_t *list[2];
        chainlink_t *link;
        int skip;

        list[0] = left;
        list[1] = right;
        short_line_init(&line, list, 2);
        skip = FALSE;
        for (link = left->first_link; link != NULL; link = link->next_link) {
          if (short_line_accept(&line, link->last, slope, aatol, artol)) {
            skip = TRUE;
            break;
          }
        }
        if (skip) {
          continue;
        }
      }

      /* Create a new chainlink between the two segments. */
      if (chainlink_insert(&first, itempool, (chainable_t *)left,
                           (chainable_t *)right) != SUCCESS) {
        DEBUG_INFO("failure");
        goto failure;
      }
      ++count;
    }
  }

  /* Try to build longer chains by appending segments to the longest ones. */
  while (count > 0) {
    level = first->level;
    length = level + 1;
    ASSERT(length <= nsegments, goto failure);
    count = 0;

    for (top = first;
         top != NULL && top->level == level;
         top = top->next) {

      if (top->right_child->first_link != NULL) {

        short_line_t line;
        chainlink_t *link;
        chainable_t *chainable;
        long k;

        /* Get vector of segments that make the chain defined by the TOP
           chain-link. */
        chainable = (chainable_t *)top;
        k = 0;
        while (chainable->level > 0) {
          chainlink_t *chainlink = (chainlink_t *)chainable;
          segment_list[k++] = chainlink->first;
          chainable = chainlink->right_child;
        }
        segment_list[k++] = (segment_t *)chainable;
        ASSERT(k == length, goto failure);

        /* Check whether potential successors are aligned with the chain
           defined by the TOP link. */
        short_line_init(&line, segment_list, length);
        for (link = top->right_child->first_link;
             link != NULL;
             link = link->next_link) {
          if (short_line_accept(&line, link->last, slope, aatol, artol)) {
            if (chainlink_insert(&first, itempool, (chainable_t *)top,
                                 (chainable_t *)link) != SUCCESS) {
              DEBUG_INFO("failure");
              goto failure;
            }
            ++count;
          }
        }
      }
    }
  }

  /* Save the longest chains (1st pass is to count the number of such chains,
     2nd pass is to register them). */
  nchains = 0;
  for (pass = 1; pass <= 2; ++pass) {

    if (pass == 2) {
      /* Allocate the chain-pool object. */
      if (nchains <= 0) {
        goto failure;
      }
      nbytes = OFFSET_OF(img_chainpool_t, chain) + nchains*sizeof(void *);
      chainpool = malloc(nbytes);
      if (chainpool == NULL) {
        DEBUG_INFO("not enough memory");
        goto failure;
      }
      memset(chainpool, 0, nbytes);
      if (PUSH_ITEM((void *)chainpool,
                    (destroy_t *)img_chainpool_destroy) != ITEMSTACK_SUCCESS) {
        DEBUG_INFO("failed to push item");
        goto failure;
      }
      chainpool->segmentation = img_segmentation_link(sgm);
    }

    /* Loop over all the chains. */
    for (top = first;
         top != NULL && (length = top->level + 1) >= lmin;
         top = top->next) {

      /* Only consider chains that are not part of a longer chain. */
      if (top->nparents == 0) {
        if (pass == 1) {
          ++nchains;
        } else {
          chainable_t *chainable;
          chain_t *chain;
          long k;

          /* Allocate memory for the chain of segments. */
          nbytes = OFFSET_OF(chain_t, segment) + length*sizeof(void *);
          chain = malloc(nbytes);
          if (chain == NULL) {
            DEBUG_INFO("not enough memory");
            goto failure;
          }
          chainpool->chain[chainpool->nchains++] = memset(chain, 0, nbytes);

          /* Get the list of segments in the chain. */
          segment_list = chain->segment;
          chainable = (chainable_t *)top;
          k = 0;
          while (chainable->level > 0) {
            chainlink_t *chainlink = (chainlink_t *)chainable;
            segment_list[k++] = chainlink->first;
            chainable = chainlink->right_child;
          }
          segment_list[k++] = (segment_t *)chainable;
          ASSERT(k == length, goto failure);
          chain->length = length;

          /* Iteratively estimate the slopes and the bounding box of the
             chain. FIXME: use the mean slope of the chain to initialize the
             fit */
          chain->a[0] = 1.0;
          chain->a[1] = 0.0;
          chain->a[2] = 0.0;
          chain->a[3] = 1.0;
          chain->vertical_shear = 0.0;
          chain->horizontal_shear = 0.0;
          if (fit_vertical_shear(chain, prec) != SUCCESS ||
              fit_horizontal_shear(chain, prec) != SUCCESS) {
            /* Discard the chain. */
            --chainpool->nchains;
            free(chain);
            continue;
          }
        }
      }
    }
  }

  /* Recover result from the top of the stack. */
  return POP_STACK();

  /* Free all stacked items and return the result. */
 failure:
  CLEAR_STACK();
  return NULL;
}

/**
 * @brief Pop topmost item out of the stack and clear the stack.
 *
 * @param stack   The address of the stack.
 *
 * @return The value of the \c data member of the topmost item of the stack.
 *         In case of error, \c NULL is returned and \c errno is set.
 */
static void *clear_and_pop(itemstack_t *stack)
{
  itemstack_item_t item;
  long n;

  /* errno = 0; */
  n = itemstack_get_number(stack);
  if (n <= 0) {
    return NULL;
  }
  if (n > 1) {
    if (itemstack_swap(stack, 0, n - 1) != ITEMSTACK_SUCCESS) {
      return NULL;
    }
    itemstack_drop(stack, n - 1);
  }
  if (itemstack_pop(stack, &item) != ITEMSTACK_SUCCESS) {
    return NULL;
  }
  return item.data;
}

/**
 * @brief Create a new chain-link object.
 *
 * This function creates a new chain-link object and insert it at
 * \c first_chainlink consistently.
 *
 * @param list    The address of a pointer to the first chain-link of the list.
 * @param pool    The memory pool to use for allocation of chain-links.
 * @param left    The left child of the chain-link.
 * @param right   The right child of the chain-link.
 *
 * @return \c SUCCESS or \c FAILURE (and \c errno set appropriately).
 */
static int chainlink_insert(chainlink_t **list,
                            itempool_t   *pool,
                            chainable_t  *left,
                            chainable_t  *right)
{
  chainlink_t *link;

  /* Check consistency. */
  ASSERT(left->level == right->level, return failure(EINVAL));
  ASSERT((left->level < 1) || (((chainlink_t *)left)->right_child == ((chainlink_t *)right)->left_child), return failure(EINVAL));

  /* Allocate a new chain-link item. */
  link = (chainlink_t *)itempool_new_item(pool);
  if (link == NULL) {
    return FAILURE;
  }
  memset(link, 0, sizeof(*link));

  /* Instanciate chain-link. */
  link->level = left->level + 1;
  link->nparents = 0;
  link->next = *list;
  *list = link;
  link->first_link = NULL;
  link->next_link = left->first_link;
  left->first_link = link;
  link->left_child = left;
  ++left->nparents;
  link->right_child = right;
  ++right->nparents;
# define ENDPOINT(e,m) ((e)->level > 0 ? ((chainlink_t *)(e))->m \
                                       : ((segment_t *)(e)))
  link->first = ENDPOINT(left,  first);
  link->last  = ENDPOINT(right, last);
# undef ENDPOINT
  return SUCCESS;
}

static void short_line_init(short_line_t *line,
                            segment_t **list,
                            long length)
{
  double h, x, y, sh, sx, sy, sxx, sxy;
  long k;

  sh = sx = sy = sxx = sxy = 0.0;
  for (k = 0; k < length; ++k) {
    x = list[k]->xcen;
    y = list[k]->ycen;
    h = list[k]->height;
    sh += h;
    sx += x;
    sy += y;
    sxx += x*x;
    sxy += x*y;
  }
  line->sh = sh;
  line->sx = sx;
  line->sy = sy;
  line->sxx = sxx;
  line->sxy = sxy;
  line->length = length;
  line->list = list;
}

static int short_line_accept(const short_line_t *line,
                             const segment_t *sgm,
                             double slope,
                             double aatol,
                             double artol)
{
  double np1, h, x, y, hm, xm, ym, a, u, v, e, threshold;
  segment_t **list;
  long k, length;

  /* Compute the parameters of the line that fits all the segments (those in
     the short-line plus the new one). */
  x = sgm->xcen;
  y = sgm->ycen;
  h = sgm->height;
  length = line->length;
  np1 = length + 1.0;
  if (fit_line(np1, line->sx + x, line->sy + y,
               line->sxx + x*x, line->sxy + x*y, &xm, &ym, &a) != 0
      || fabs(a) > slope) {
    return FALSE;
  }

  /* Average height and maximum vertical distance. */
  hm = (line->sh + h)/np1;
  threshold = aatol + artol*hm;

  /* Find the worst (vertical) distance of the line to the segment positions.
     The coordinates are taken relatively to the mean position to minimize
     rounding errors.  FIXME: also check the height. */
  u = x - xm;
  v = y - ym;
  e = fabs(a*u - v);
  if (e > threshold) {
    return FALSE;
  }
  list = line->list;
  for (k = 0; k < length; ++k) {
    u = list[k]->xcen - xm;
    v = list[k]->ycen - ym;
    e = fabs(a*u - v);
    if (e > threshold) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * @brief Compute equation of regression line.
 *
 * Compute the coefficients of the line which minimizes the weighted vertical
 * distance to a list of points.  The equation of the line is:
 *  \f$ y = ym + alpha*(x - xm)\f$
 *
 * @param sw     The sum of weights.
 * @param swx    The sum of weights times abscissaes.
 * @param swy    The sum of weights times ordinates.
 * @param swxx   The sum of weights times squared abscissaes.
 * @param swxy   The sum of weights times abscissaes times ordinates.
 * @param xm     The address to store the mean abscissa.
 * @param ym     The address to store the mean ordinate.
 * @param alpha  The address to store the slope.
 *
 * @return 0 on success, -1 if the slope is infinite (which means that the
 *         line is vertical and may be due to rounding errors), -2 if there is
 *         not enough points (or no spread in the sample).
 */
static int fit_line(double sw,
                    double swx,
                    double swy,
                    double swxx,
                    double swxy,
                    double *xm,
                    double *ym,
                    double *alpha)
{
  double q, x, y, r;

  if (sw <= 0.0) {
    return -2;
  }
  q = 1.0/sw;
  *xm = (x = swx*q);
  *ym = (y = swy*q);
  r = swxx*q - x*x;
  if (r <= 0.0) {
    /* vertical line (may be due to rounding errors) */
    *alpha = DBL_MAX;
    return -1;
  } else {
    *alpha = (swxy*q - x*y)/r;
    return 0;
  }
}

/**
 * @brief Fit the vertical shear of a chain.
 *
 * Adjust the vertical shear so as to optimize the alignement of the
 * bounding-box of chained segments.
 *
 * @param chain The chain to fix.
 *
 * @return \c SUCCESS on success; \c FAILURE on failure.
 */
static int fit_vertical_shear(chain_t *chain, double prec)
{
  const long maxiter = 10;
  bbox_t bbox;
  double x, y, sx, sy, sxx, sxy, xm, ym, slope, tol;
  double xmin, xmax, ymin, ymax, temp;
  long k, length, iter;
  segment_t **segment_list;
  int convergence = FALSE;

  length = chain->length;
  segment_list = chain->segment;
  xmin = xmax = ymin = ymax = 0.0; /* avoid compiler warnings */
  iter = 0;
  while (TRUE) {
    sx = sy = sxx = sxy = 0.0;
    if (iter == 0) {
      for (k = 0; k < length; ++k) {
        if (k == 0) {
          xmin = segment_list[k]->xmin;
          xmax = segment_list[k]->xmax;
          ymin = segment_list[k]->ymin;
          ymax = segment_list[k]->ymax;
        } else {
          if ((temp = segment_list[k]->xmin) < xmin) xmin = temp;
          if ((temp = segment_list[k]->xmax) > xmax) xmax = temp;
          if ((temp = segment_list[k]->ymin) < ymin) ymin = temp;
          if ((temp = segment_list[k]->ymax) > ymax) ymax = temp;
        }
        x = segment_list[k]->xcen;
        y = segment_list[k]->ycen;
        sx += x;
        sy += y;
        sxx += x*x;
        sxy += x*y;
      }
    } else {
      for (k = 0; k < length; ++k) {
        get_bbox(&bbox, segment_list[k], chain->a);
        if (k == 0) {
          xmin = bbox.xmin;
          xmax = bbox.xmax;
          ymin = bbox.ymin;
          ymax = bbox.ymax;
        } else {
          if (bbox.xmin < xmin) xmin = bbox.xmin;
          if (bbox.xmax > xmax) xmax = bbox.xmax;
          if (bbox.ymin < ymin) ymin = bbox.ymin;
          if (bbox.ymax > ymax) ymax = bbox.ymax;
        }
        x = 0.5*(bbox.xmax + bbox.xmin);
        y = 0.5*(bbox.ymax + bbox.ymin);
        sx += x;
        sy += y;
        sxx += x*x;
        sxy += x*y;
      }
    }
    if (fit_line(length, sx, sy, sxx, sxy, &xm, &ym, &slope) != 0) {
      /* Vertical line or slope too steep. */
      return FAILURE;
    }
    /* Check for convergence (at least one iteration is required, the
       tolerance is the precision in pixels divided by the lever arm
       which is the total width of the chain). */
    tol = prec/(1.0 + xmax - xmin);
    convergence = (iter >= 1 && fabs(slope) <= tol);
    chain->vertical_shear += slope; /* update the slope */
    chain->a[2] = -chain->vertical_shear; /* to fix the shear */
#if 0
    fprintf(stderr, "vertical_shear (%ld) = %12.9f  (tolerance = %.2e)\n",
            iter, chain->vertical_shear, tol);
#endif
    if (convergence) {
      chain->xmin = xmin;
      chain->xmax = xmax;
      chain->ymin = ymin;
      chain->ymax = ymax;
      return SUCCESS;
    }
    if (++iter > maxiter) {
      return FAILURE;
    }
  }
}

/**
 * @brief Fit the horizontal shear of a chain.
 *
 * Adjust the horizontal shear so as to maximize the spacing of the segments.
 *
 * @param chain The chain to fix.
 *
 * @return \c SUCCESS on success; \c FAILURE on failure.
 */
static int fit_horizontal_shear(chain_t *chain, double prec)
{
  double shear, best_shear, spacing, best_spacing;
  double width, height, step, bound, a[4];
  double xmin, xmax, ymin, ymax, prev_xmax;
  bbox_t bbox;
  long k, length, iter, maxiter;
  segment_t **segment_list;

  length = chain->length;
  segment_list = chain->segment;
  a[0] = chain->a[0];
  a[1] = chain->a[1];
  a[2] = chain->a[2];
  a[3] = chain->a[3];

  /* Compute a shear step corresponding to a displacement of 1/4 of pixel and
     a bound corresponding to +/- 1/2 of the mean width of the segments.  In a
     first stage, the best shear is found (favoring small, in magnitude,
     corrections). In a second stage, the global bounding box of the chain is
     updated. */
  /* FIXME: use a golden search method. */
  width = (1.0 + chain->xmax - chain->xmin)/length;
  height = 1.0 + chain->ymax - chain->ymin;
  step = 0.25/height;
  bound = 0.5*width/height;
  maxiter = 2*(long)ceil(bound/step);
  best_spacing = 0.0;
  best_shear = 0.0;
  for (iter = 0; iter <= maxiter; ++iter) {
    if (iter % 2 == 0) {
      shear = +step*(iter/2);
    } else {
      shear = -step*((iter + 1)/2);
    }
    a[1] = -shear; /* to fix the shear */
    spacing = 0.0;
    for (k = 0; k < length; ++k) {
      get_bbox(&bbox, segment_list[k], a);
      if (k != 0) {
        spacing += (bbox.xmin - prev_xmax);
      }
      prev_xmax = bbox.xmax;
    }
#if 0
    fprintf(stderr, "vertical_shear (%ld) = %12.9f  --> spacing = %12.9f\n",
            iter, shear, spacing);
#endif
    if (iter == 0 || spacing > best_spacing) {
      best_shear = shear;
      best_spacing = spacing;
    }
  }
  chain->horizontal_shear = best_shear;
  chain->a[1] = a[1] = -chain->horizontal_shear; /* to fix the shear */
  xmin = xmax = ymin = ymax = 0.0; /* avoid compiler warnings */
  xmin = xmax = ymin = ymax = 0.0; /* avoid compiler warnings */
  for (k = 0; k < length; ++k) {
    get_bbox(&bbox, segment_list[k], a);
    if (k == 0) {
      xmin = bbox.xmin;
      xmax = bbox.xmax;
      ymin = bbox.ymin;
      ymax = bbox.ymax;
    } else {
      if (bbox.xmin < xmin) xmin = bbox.xmin;
      if (bbox.xmax > xmax) xmax = bbox.xmax;
      if (bbox.ymin < ymin) ymin = bbox.ymin;
      if (bbox.ymax > ymax) ymax = bbox.ymax;
    }
  }
  chain->xmin = xmin;
  chain->xmax = xmax;
  chain->ymin = ymin;
  chain->ymax = ymax;
  return SUCCESS;
}

/**
 * @brief Get the bounding box of a segment after linear geometrical
 *        transform.  The segment must have at least one point.
 *
 * @param bbox   The address of the result.
 * @param s      The input segment.
 * @param a      The coefficients of the linear transform.
 */
static void get_bbox(bbox_t *bbox,
		     const segment_t *s,
		     const double a[])
{
  double px, x, xmin, xmax;
  double py, y, ymin, ymax;
  double axx, axy, ayx, ayy;
  const point_t *point;
  long i, number;
  const link_t mask = (IMG_LINK_EAST  | IMG_LINK_WEST |
                       IMG_LINK_NORTH | IMG_LINK_SOUTH);

  if ((s == NULL) || ((number = s->count) < 1)) {
    xmin = xmax = ymin = ymax = 0.0;
  } else {
    axx = a[0];
    axy = a[1];
    ayx = a[2];
    ayy = a[3];
    point = s->point;
    i = 0;
    px = point[i].x;
    py = point[i].y;
    xmin = xmax = axx*px + axy*py;
    ymin = ymax = ayx*px + ayy*py;
    while (++i < number) {
      if ((point[i].link & mask) != mask) {
        px = point[i].x;
        py = point[i].y;
        x = axx*px + axy*py;
        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
        y = ayx*px + ayy*py;
        if (y < ymin) ymin = y;
      if (y > ymax) ymax = y;
      }
    }
  }
  bbox->xmin = xmin;
  bbox->xmax = xmax;
  bbox->ymin = ymin;
  bbox->ymax = ymax;
}

#define OWNED  IMG_LINK_OWNED

long img_segment_from_links(link_t link[], const long width,
                            const long height, long index[])
{
  long i, j, k, number, size, nsegments;
  long *region;
  int s;

  nsegments = 0;
  number = width*height;
  region = index;
  for (i = 0; i < number; ++i) {
    /* Check whether current pixel is not already owned by a segment. */
    if ((link[i] & OWNED) == 0) {
      link[i] |= OWNED; /* mark pixel as being owned by a segment */
      size = 1; /* number of pixels in the current region */
      region[size] = i; /* i-th pixel belongs to current region */
      for (j = 1; j <= size; ++j) {
        k = region[j];
        s = link[k];
#define CHECK_LINK(DIR, IDX)                    \
        do {                                    \
          if ((s & (DIR)) != 0) {               \
            long l = (IDX);                     \
            if ((link[l] & OWNED) == 0) {       \
              link[l] |= OWNED;                 \
              region[++size] = l;               \
            }                                   \
          }                                     \
        } while (0)
        CHECK_LINK(IMG_LINK_WEST,  k - 1);
        CHECK_LINK(IMG_LINK_EAST,  k + 1);
        CHECK_LINK(IMG_LINK_SOUTH, k - width);
        CHECK_LINK(IMG_LINK_NORTH, k + width);
#undef CHECK_LINK
      }
      region[0] = size; /* store the number of pixels in the region */
      region += (size + 1); /* move to base of next region */
      ++nsegments;
    }
  }
  return nsegments;
}

#undef OWNED

/*---------------------------------------------------------------------------*/
/* UTILITIES */

#if 0
static int mem_realloc(void **ptr, size_t size)
{
  void *addr; /* to store initial address */

  if ((addr = *ptr) == NULL) {
    if ((*ptr = malloc(size)) != NULL) {
      return SUCCESS;
    }
  } else {
    if ((*ptr = realloc(addr, size)) != NULL) {
      return SUCCESS;
    }
    *ptr = addr; /* restore initial address */
  }
  return FAILURE;
}
#endif

/*---------------------------------------------------------------------------*/

#else /* _IMG_SEGMENT_C defined */

/* Computes absolute difference between A and B. */
#undef ABS_DIFF
#if CPT_IS_DOUBLE(TYPE)
# define ABS_DIFF(a,b)  (fabs((a) - (b)))
#elif CPT_IS_FLOAT(TYPE)
# define ABS_DIFF(a,b)  (fabsf((a) - (b)))
#else
# define ABS_DIFF(a,b)  (((a) >= (b)) ? ((a) - (b)) :  ((b) - (a)))
#endif

/* Check whether absolute difference between A and B is less or equal
   threshold T. */
#undef SIMILAR
#define SIMILAR(a,b,t)   (ABS_DIFF(a,b) <= (t))

#if (! CPT_IS_COMPLEX(TYPE) && ! CPT_IS_COLOR(TYPE))

 int BUILD_LINKS(TYPE)(const pixel_t img[],
                             const long img_offset,
                             const long img_pitch,
                             link_t lnk[],
                             const long lnk_offset,
                             const long lnk_pitch,
                             const long width,
                             const long height,
                             const pixel_t threshold)
{
  link_t *lnk0, *lnk2, bit0, bit2;
  const pixel_t *img0, *img2;
  long y, x;
  pixel_t pix0, pix1, pix2;

  /* Change macro if image(s) may have a pitch different from their width. */
#undef img_pitch
  /*#define img_pitch width*/
#undef lnk_pitch
  /*#define lnk_pitch width*/

  if (img == NULL || lnk == NULL) {
    errno = EFAULT;
    return IMG_FAILURE;
  }
  if (img_pitch < width || lnk_pitch < width) {
    errno = EINVAL;
    return IMG_FAILURE;
  }
  if (width <= 0 || height <= 0) {
    if (width < 0 || height < 0) {
      errno = EINVAL;
    }
    return IMG_FAILURE;
  }


  /*
   *       +---+---+
   *    y  | 1 | 0 |    <-- img0, lnk0
   *       +---+---+
   *   y-1 |   | 2 |    <-- img2, lnk2
   *       +---+---+
   *        x-1  x
   */

  /* Initialization. */
  img0 = img + img_offset;
  pix0 = img0[0];
  lnk0 = lnk + lnk_offset;
  lnk0[0] = (link_t)IMG_LINK_NONE;

  /* For every pixel, set "link" bits indicating if the neighbor in a given
     direction belongs to the same region. */
  if (threshold != (pixel_t)0) {
    /* Neighbor pixels belongs to the same region if their absolute
       difference is less or equal THRESHOLD. */

    /* First row (Y = 0). */
    for (x = 1; x < width; ++x) {
      pix1 = pix0;     /* value of previous pixel */
      pix0 = img0[x];  /* value of current pixel */
      if (SIMILAR(pix0, pix1, threshold)) {
        lnk0[x - 1] |= (link_t)IMG_LINK_EAST;
        lnk0[x] = (link_t)IMG_LINK_WEST;
      } else {
        lnk0[x] = (link_t)IMG_LINK_NONE;
      }
    }

    /* Other rows (Y > 0). */
    for (y = 1; y < height; ++y) {
      img2 = img0;
      img0 += img_pitch;
      pix0 = img0[0];
      lnk2 = lnk0;
      lnk0 += lnk_pitch;
      lnk0[0] = (link_t)IMG_LINK_NONE;
      for (x = 1; x < width; ++x) {
        pix1 = pix0;     /* value of previous pixel */
	pix0 = img0[x];  /* value of current pixel */
        pix2 = img2[x];  /* value of pixel below current one */
        if (SIMILAR(pix0, pix1, threshold)) {
          lnk0[x - 1] |= (link_t)IMG_LINK_EAST;
          bit0 = (link_t)IMG_LINK_WEST;
        } else {
          bit0 = (link_t)IMG_LINK_NONE;
        }
        if (SIMILAR(pix0, pix2, threshold)) {
          lnk2[x] |= (link_t)IMG_LINK_NORTH;
          bit2 = (link_t)IMG_LINK_SOUTH;
        } else {
          bit2 = (link_t)IMG_LINK_NONE;
        }
        lnk0[x] = bit0 | bit2;
      }
    }

  } else {
    /* Neighbor pixels belongs to the same region if they have the
       same value. */

    /* First row (Y = 0). */
    for (x = 1; x < width; ++x) {
      pix1 = pix0;     /* value of previous pixel */
      pix0 = img0[x];  /* value of current pixel */
      if (pix0 == pix1) {
        lnk0[x - 1] |= (link_t)IMG_LINK_EAST;
        lnk0[x] = (link_t)IMG_LINK_WEST;
      } else {
        lnk0[x] = (link_t)IMG_LINK_NONE;
      }
    }

    /* Other rows (Y > 0). */
    for (y = 1; y < height; ++y) {
      img2 = img0;
      img0 += img_pitch;
      pix0 = img0[0];
      lnk2 = lnk0;
      lnk0 += lnk_pitch;
      lnk0[0] = (link_t)IMG_LINK_NONE;
      for (x = 1; x < width; ++x) {
        pix1 = pix0;     /* value of previous pixel */
	pix0 = img0[x];  /* value of current pixel */
        pix2 = img2[x];  /* value of pixel below current one */
        if (pix0 == pix1) {
          lnk0[x - 1] |= (link_t)IMG_LINK_EAST;
          bit0 = (link_t)IMG_LINK_WEST;
        } else {
          bit0 = (link_t)IMG_LINK_NONE;
        }
        if (pix0 == pix2) {
          lnk2[x] |= (link_t)IMG_LINK_NORTH;
          bit2 = (link_t)IMG_LINK_SOUTH;
        } else {
          bit2 = (link_t)IMG_LINK_NONE;
        }
        lnk0[x] = (bit0 | bit2);
      }
    }
  }
  return IMG_SUCCESS;
}
#endif /* not COMPLEX and not COLOR */

#undef ABS_DIFF
#undef SIMILAR

/*---------------------------------------------------------------------------*/

#undef TYPE

#endif /* _IMG_SEGMENT_C defined */

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
