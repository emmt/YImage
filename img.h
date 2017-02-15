/*
 * img.h --
 *
 * Definitions for low-level (and simple) image routines.
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

#ifndef _IMG_H
#define _IMG_H 1

#include "c_pseudo_template.h"

/*---------------------------------------------------------------------------*/
/* Definitions for exporting/importing the public API to/from a shared library.
   See http://gcc.gnu.org/wiki/Visibility for explanations. */

/* Generic helper definitions for shared library support. */
#if defined _WIN32 || defined __CYGWIN__
# define IMG_HELPER_DLL_IMPORT __declspec(dllimport)
# define IMG_HELPER_DLL_EXPORT __declspec(dllexport)
# define IMG_HELPER_DLL_LOCAL
#elif __GNUC__ >= 4
# define IMG_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
# define IMG_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
# define IMG_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
# define IMG_HELPER_DLL_IMPORT
# define IMG_HELPER_DLL_EXPORT
# define IMG_HELPER_DLL_LOCAL
#endif

/* Now we use the generic helper definitions above to define IMG_API and
   IMG_LOCAL.  IMG_API is used for the public API symbols.  It either DLL
   imports or DLL exports (or does nothing for static build) IMG_LOCAL is
   used for non-api symbols. */
#ifdef IMG_DLL /* defined if IMG library is compiled as a DLL */
# define IMG_LOCAL IMG_HELPER_DLL_LOCAL
# ifdef IMG_DLL_EXPORTS /* defined if we are building the IMG DLL
                           (instead of using it) */
#  define IMG_API IMG_HELPER_DLL_EXPORT
# else
#  define IMG_API IMG_HELPER_DLL_IMPORT
# endif /* IMG_DLL_EXPORTS */
#else /* IMG_DLL is not defined: this means IMG is a static lib. */
# define IMG_LOCAL
# define IMG_API
#endif /* IMG_DLL */

/*---------------------------------------------------------------------------*/

#define IMG_SUCCESS   0
#define IMG_FAILURE (-1)

/* Image pixel types (defined as macros to allow pre-processor
   processing in the code). */
#define IMG_TYPE_NONE       0
#define IMG_TYPE_INT8       1
#define IMG_TYPE_UINT8      2
#define IMG_TYPE_INT16      3
#define IMG_TYPE_UINT16     4
#define IMG_TYPE_INT32      5
#define IMG_TYPE_UINT32     6
#define IMG_TYPE_INT64      7
#define IMG_TYPE_UINT64     8
#define IMG_TYPE_FLOAT      9
#define IMG_TYPE_DOUBLE    10
#define IMG_TYPE_SCOMPLEX  11
#define IMG_TYPE_DCOMPLEX  12
#define IMG_TYPE_RGB       13
#define IMG_TYPE_RGBA      14

#define IMG_TYPE_MIN IMG_TYPE_NONE
#define IMG_TYPE_MAX IMG_TYPE_RGBA

/* Use constants in c_pseudo_template.h to setup macro definitions for basic
   integer types. */

#ifndef DOXYGEN

#ifdef CPT_CHAR
# if (CPT_CHAR == CPT_INT8)
#   define IMG_TYPE_CHAR  IMG_TYPE_INT8
# elif (CPT_CHAR == CPT_UINT8)
#   define IMG_TYPE_CHAR  IMG_TYPE_UINT8
# endif
#endif

#ifdef CPT_UCHAR
# if (CPT_UCHAR == CPT_UINT8)
#  define IMG_TYPE_UCHAR  IMG_TYPE_UINT8
# endif
#endif

#ifdef CPT_SHORT
# if (CPT_SHORT == CPT_INT16)
#  define IMG_TYPE_SHORT       IMG_TYPE_INT16
#  define IMG_TYPE_USHORT      IMG_TYPE_UINT16
# elif (CPT_SHORT == CPT_INT32)
#  define IMG_TYPE_SHORT       IMG_TYPE_INT32
#  define IMG_TYPE_USHORT      IMG_TYPE_UINT32
# elif (CPT_SHORT == CPT_INT64)
#  define IMG_TYPE_SHORT       IMG_TYPE_INT64
#  define IMG_TYPE_USHORT      IMG_TYPE_UINT64
# endif
#endif

#ifdef CPT_INT
# if (CPT_INT == CPT_INT16)
#  define IMG_TYPE_INT       IMG_TYPE_INT16
#  define IMG_TYPE_UINT      IMG_TYPE_UINT16
# elif (CPT_INT == CPT_INT32)
#  define IMG_TYPE_INT       IMG_TYPE_INT32
#  define IMG_TYPE_UINT      IMG_TYPE_UINT32
# elif (CPT_INT == CPT_INT64)
#  define IMG_TYPE_INT       IMG_TYPE_INT64
#  define IMG_TYPE_UINT      IMG_TYPE_UINT64
# endif
#endif

#ifdef CPT_LONG
# if (CPT_LONG == CPT_INT32)
#  define IMG_TYPE_LONG       IMG_TYPE_INT32
#  define IMG_TYPE_ULONG      IMG_TYPE_UINT32
# elif (CPT_LONG == CPT_INT64)
#  define IMG_TYPE_LONG       IMG_TYPE_INT64
#  define IMG_TYPE_ULONG      IMG_TYPE_UINT64
# endif
#endif

#ifdef CPT_LLONG
# if (CPT_LLONG == CPT_INT64)
#  define IMG_TYPE_LLONG       IMG_TYPE_INT64
#  define IMG_TYPE_ULLONG      IMG_TYPE_UINT64
# endif
#endif

#endif /* DOXYGEN */

#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* COPY AND CONVERSION */

int img_copy(const long  width, const long  height,
             const void *src_addr, const int src_type,
             const long src_offset, const long src_pitch,
             void *dst_addr, const int dst_type,
             const long dst_offset, const long dst_pitch);

/*---------------------------------------------------------------------------*/
/* MORPHO-MATH OPERATIONS */

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

/*---------------------------------------------------------------------------*/
/* LINEAR TRANSFORM */

extern int img_extract_rectangle(const void *src,
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
                                 int inverse);

extern int img_inverse_linear_transform(const double a[], long ncoefs,
                                        double b[]);

/*---------------------------------------------------------------------------*/
/* IMAGE NOISE */

extern double img_estimate_noise(const int type, const void *img,
                                 const long offset, const long width,
                                 const long height, const long stride,
                                 const int method);

/*---------------------------------------------------------------------------*/
/* IMAGE COMPARISON */

extern double img_cost_l2(const int type,
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
                          double scale);

/* Image segmentation functions. */

typedef unsigned char img_link_t;
typedef unsigned char img_byte_t;

#define IMG_LINK_NONE    0
#define IMG_LINK_EAST    1  /* right */
#define IMG_LINK_WEST    2  /* left */
#define IMG_LINK_NORTH   4  /* above */
#define IMG_LINK_SOUTH   8  /* below */
#define IMG_LINK_OWNED  16  /* pixel already taken into account */

extern int img_build_links(const void *img, const int type,
                           const long img_offset, const long img_pitch,
                           img_link_t lnk[], const long lnk_offset,
                           const long lnk_pitch, const long width,
                           const long height, const double threshold);

/* Opaque structure used to collect segments extracted from an image. */
typedef struct _img_segmentation img_segmentation_t;

extern img_segmentation_t *img_segmentation_new(const void *img,
						const int type,
						const long offset,
						const long width,
						const long height,
						const long stride,
						const double threshold);
extern void img_segmentation_unlink(img_segmentation_t *ws);
extern img_segmentation_t *img_segmentation_link(img_segmentation_t *ws);
extern img_segmentation_t *img_segmentation_select(const img_segmentation_t *src,
						   const long list[],
						   const long number);
extern int img_segmentation_get_nrefs(img_segmentation_t *ws);
extern long img_segmentation_get_number(img_segmentation_t *ws);
extern long img_segmentation_get_image_width(img_segmentation_t *sgm);
extern long img_segmentation_get_image_height(img_segmentation_t *sgm);
extern int img_segmentation_get_xcens(img_segmentation_t *ws,
				      double x[], long number);
extern int img_segmentation_get_ycens(img_segmentation_t *ws,
				      double y[], long number);
extern int img_segmentation_get_counts(img_segmentation_t *ws,
				       long count[], long number);
extern int img_segmentation_get_xmins(img_segmentation_t *ws,
				      long xmin[], long number);
extern int img_segmentation_get_xmaxs(img_segmentation_t *ws,
				      long xmax[], long number);
extern int img_segmentation_get_ymins(img_segmentation_t *ws,
				      long ymin[], long number);
extern int img_segmentation_get_ymaxs(img_segmentation_t *ws,
				      long ymax[], long number);
extern int img_segmentation_get_widths(img_segmentation_t *ws,
				       long width[], long number);
extern int img_segmentation_get_heights(img_segmentation_t *ws,
					long height[], long number);

extern double img_segmentation_get_xcen(img_segmentation_t *ws, long j);
extern double img_segmentation_get_ycen(img_segmentation_t *ws, long j);
extern long img_segmentation_get_count(img_segmentation_t *ws, long j);
extern long img_segmentation_get_xmin(img_segmentation_t *ws, long j);
extern long img_segmentation_get_xmax(img_segmentation_t *ws, long j);
extern long img_segmentation_get_ymin(img_segmentation_t *ws, long j);
extern long img_segmentation_get_ymax(img_segmentation_t *ws, long j);
extern long img_segmentation_get_width(img_segmentation_t *ws, long j);
extern long img_segmentation_get_height(img_segmentation_t *ws, long j);

extern int img_segmentation_get_x(img_segmentation_t *ws, const long i,
                                  long what[], const long number);
extern int img_segmentation_get_y(img_segmentation_t *ws, const long i,
                                  long what[], const long number);
extern int img_segmentation_get_link(img_segmentation_t *ws, const long i,
                                     long what[], const long number);


/* Opaque structure to store a pool of chains of image segments. */
typedef struct _img_chainpool img_chainpool_t;

extern img_chainpool_t *img_chainpool_new(img_segmentation_t *sgm,
                                          double satol,
                                          double srtol,
                                          double drmin,
                                          double drmax,
                                          double slope,
                                          double aatol,
                                          double artol,
                                          double prec,
                                          long lmin,
                                          long lmax);
extern void img_chainpool_destroy(img_chainpool_t *self);
extern long img_chainpool_get_number(img_chainpool_t *self);
extern img_segmentation_t *img_chainpool_get_segmentation(img_chainpool_t *self);
extern long img_chainpool_get_image_width(img_chainpool_t *chn);
extern long img_chainpool_get_image_height(img_chainpool_t *chn);
extern int img_chainpool_get_segments(img_chainpool_t *chn,
                                      long j, long list[], long n);

extern int img_chainpool_get_vertical_shears(img_chainpool_t *chn,
                                             double shear[], long n);
extern int img_chainpool_get_horizontal_shears(img_chainpool_t *chn,
                                               double shear[], long n);
extern int img_chainpool_get_xmins(img_chainpool_t *chn,
                                   double xmin[], long n);
extern int img_chainpool_get_xmaxs(img_chainpool_t *chn,
                                   double xmax[], long n);
extern int img_chainpool_get_ymins(img_chainpool_t *chn,
                                   double ymin[], long n);
extern int img_chainpool_get_ymaxs(img_chainpool_t *chn,
                                   double ymax[], long n);
extern int img_chainpool_get_lengths(img_chainpool_t *chn,
                                     long length[], long n);

extern double img_chainpool_get_vertical_shear(img_chainpool_t *chn, long j);
extern double img_chainpool_get_horizontal_shear(img_chainpool_t *chn, long j);
extern double img_chainpool_get_xmin(img_chainpool_t *chn, long j);
extern double img_chainpool_get_xmax(img_chainpool_t *chn, long j);
extern double img_chainpool_get_ymin(img_chainpool_t *chn, long j);
extern double img_chainpool_get_ymax(img_chainpool_t *chn, long j);
extern long img_chainpool_get_length(img_chainpool_t *chn, long j);


/**
 * @brief Get image segments given a map of pixel links.
 *
 * Given a map of pixel links, this function retrieve the offsets of the
 * pixels in the same segments.
 *
 * The output is build as follows: \a index[0] = n1, the size of the first
 * segment, \a index[1] ... \a index[n1] are the offsets (in a \a width by \a
 * height array) of the elements in the first segment; \a index[n1+1] = n2,
 * the size of the second segment, etc.
 *
 * The returned value is the number N of segments, hence \a width*\a height +
 * N elements of \a index are set by the function (the others are left
 * unchanged).

 * @param link    The input \a width by \a height array of links.
 * @param width   The width of the image.
 * @param height  The height of the image.
 * @param index   The output array.  Must have, at least, 2*\a width*\a height
 *                elements.
 * @return The number of segments.
 */
extern long img_segment_from_links(img_link_t link[], const long width,
                                   const long height, long index[]);


/*---------------------------------------------------------------------------*/
/* SPOT DETECTION */

extern int img_detect_spot(const void* src, const int type,
                           const int width, const long height,
                           const double c0, const double c1, const double c2,
                           const double t0, const double t1, const double t2,
                           int dst[], long* count_ptr, double* ws);

/*---------------------------------------------------------------------------*/

#ifdef  __cplusplus
}
#endif

#endif /* _IMG_H */

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
