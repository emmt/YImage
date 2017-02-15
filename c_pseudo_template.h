/*
 * c_pseudo_template.h --
 *
 * Definitions for pseudo-template programming in C.
 *
 *-----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 Eric Thiébaut <thiebaut@obs.univ-lyon1.fr>
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
 * $Id: c_pseudo_template.h,v 1.3 2009/12/10 08:57:50 eric Exp $
 * $Log: c_pseudo_template.h,v $
 * Revision 1.3  2009/12/10 08:57:50  eric
 *  - New macros for min/max values.
 *
 * Revision 1.2  2009/11/23 16:24:24  eric
 *  - Some minor bugs fixed.
 *  - New types for color images: RGB and RGBA.
 *
 * Revision 1.1  2009/11/15 07:39:54  eric
 * Initial revision
 *
 *-----------------------------------------------------------------------------
 */

#ifndef _CPT_H
#define _CPT_H 1

#include <stdint.h>
#include <limits.h>
#include <float.h>

/** 
 * @author Eric Thiébaut <thiebaut\@obs.univ-lyon1.fr>
 * @date   Wed Mar 25 15:28:39 2009
 */

/**
 * @addtogroup cpt Pseudo-Templates in C
 *
 * @brief  Definitions for pseudo-template programming in C.
 *
 * The file "c_pseudo_template.h" defines macros that can be used to achieve
 * template programming in C.
 *
 * @code
 * #include "c_pseudo_template.h"
 * @endcode
 *
 * All the definitions shall be expanded by the pre-processor either as
 * integer constants (so that they can be part of preprocessor \#if and \#elif
 * test directives) or C code.  The advantage of such macros is that they are
 * all evaluated by the preprocessor and can be used for conditional code at
 * compilation time.  Another advantage is that compiler errors will give line
 * numbers in the source and make the debugging easier (unlike multi-statement
 * macros).
 *
 * Most macros take a type identifier as argument.  The argument is expanded
 * by the pre-processor, hence for the macros to properly work, the recognized
 * tokens (see table below) should not be defined as macros (otherwise they
 * don't work as shortcuts).  However, at least the prefixed name should
 * always work, it is thus the token of choice.
 *
 * @code
 * C type       Integer   Abbreviation   Name         Prefixed name
 * ----------------------------------------------------------------
 * void             0         x          VOID         CPT_VOID
 * int8_t           1         i8         INT8         CPT_INT8
 * uint8_t          2         u8         UINT8        CPT_UINT8
 * int16_t          3         i16        INT16        CPT_INT16
 * uint16_t         4         u16        UINT16       CPT_UINT16
 * int32_t          5         i32        INT32        CPT_INT32
 * uint32_t         6         u32        UINT32       CPT_UINT32
 * int64_t          7         i64        INT64        CPT_INT64
 * uint64_t         8         u64        UINT64       CPT_UINT64
 * float            9         f          FLOAT        CPT_FLOAT
 * double          10         d          DOUBLE       CPT_DOUBLE
 * cpt_scomplex_t  11         c          SCOMPLEX     CPT_SCOMPLEX
 * cpt_dcomplex_t  12         z          DCOMPLEX     CPT_DCOMPLEX
 * cpt_rgb_t       13         rgb        RGB          CPT_RGB
 * cpt_rgba_t      14         rgba       RGBA         CPT_RGBA
 * pointer         15         p          POINTER      CPT_POINTER
 * @endcode
 *
 * In addition, and based on file @c <limits.h>, basic C integer types
 * may also be used: @c char, @c uchar, @c short, @c ushort, etc.
 *
 * @par Example
 *
 * The following source code shows how to implement the code for functions to
 * add two arrays of values.  The pre-processor will output function code for
 * every different types (here UINT8, INT16 and FLOAT to produce functions @c
 * add_u8, @c add_i16 and @c add_s respectively).
 *
 * @code
 * #ifndef _MY_PSEUDO_CODE_C
 * // This is the first time that this file is seen by the pre-processor.
 *
 * // Avoid multiple inclusions of this part.
 * #define _MY_PSEUDO_CODE_C 1
 *
 * // Include definitions.
 * #include <stdint.h>
 * #include "c_pseudo_template.h"
 *
 * // Some definitions that will be expanded by the template code.
 * #define FUNC(prefix) CPT_JOIN3(prefix, _, CPT_ABBREV(TYPE))
 * #define CTYPE        CPT_CTYPE(TYPE)
 *
 * // Manage to include this file with a different data type each time.
 * #define TYPE CPT_UINT8
 * #include __FILE__
 * #define TYPE CPT_INT16
 * #include __FILE__
 * #define TYPE CPT_FLOAT
 * #include __FILE__
 *
 * #else // _MY_PSEUDO_CODE_C
 * // This is not the first time that this file is seen by the pre-processor.
 * // This part consist in pseudo-template code.
 *
 * void FUNC(add)(const long number, const CTYPE *arg1,
 *                const CTYPE *arg2, CTYPE *res)
 * {
 *   long j;
 *
 *   for (j = 0; j < number; ++j) {
 *     res[j] = arg1[j] + arg2[j];
 *   }
 * }
 *
 * // Undefine macro(s) that may be re-defined to avoid warnings.
 * #undef TYPE
 *
 * #endif // _MY_PSEUDO_CODE_C
 * @endcode
 *
 * @{
 */

typedef struct { float re, im; } cpt_scomplex_t;
typedef struct { double re, im; } cpt_dcomplex_t;
typedef struct { uint8_t r, g, b; } cpt_rgb_t;
typedef struct { uint8_t r, g, b, a; } cpt_rgba_t;

/*---------------------------------------------------------------------------*/

/** @brief Yield numerical identifier of type.
 *
 *  Expand accepted type identifier to unique integer numerical value.
 *
 *  @par Examples:
 *  @code
 *  CPT_TYPE(int64_t)   // yields 7
 *  #define my_real double
 *  CPT_TYPE(my_real)   // yields 10
 *  @endcode
 *
 *  @param id   Type identifier, must expand to one of:
 *               - integer in the range [0,11]
 *               - abbreviated type: i8, u8, i16, u16, ..., s, d, p
 *               - standard C type: int8_t, uint8_t, ..., float, double, pointer
 *               - uppercase name: INT8, UINT8, ..., FLOAT, DOUBLE
 *               - macro type name: CPT_INT8, CPT_UINT8, ..., CPT_FLOAT, ...
 */
#define CPT_TYPE(id)  CPT_JOIN2(_CPT_TYPE,_CPT_IDENT(id))

#ifndef DOXYGEN
# define _CPT_TYPE_CPT_VOID     CPT_VOID
# define _CPT_TYPE_CPT_INT8     CPT_INT8
# define _CPT_TYPE_CPT_UINT8    CPT_UINT8
# define _CPT_TYPE_CPT_INT16    CPT_INT16
# define _CPT_TYPE_CPT_UINT16   CPT_UINT16
# define _CPT_TYPE_CPT_INT32    CPT_INT32
# define _CPT_TYPE_CPT_UINT32   CPT_UINT32
# define _CPT_TYPE_CPT_INT64    CPT_INT64
# define _CPT_TYPE_CPT_UINT64   CPT_UINT64
# define _CPT_TYPE_CPT_FLOAT    CPT_FLOAT
# define _CPT_TYPE_CPT_DOUBLE   CPT_DOUBLE
# define _CPT_TYPE_CPT_SCOMPLEX CPT_SCOMPLEX
# define _CPT_TYPE_CPT_DCOMPLEX CPT_DCOMPLEX
# define _CPT_TYPE_CPT_RGB      CPT_RGB
# define _CPT_TYPE_CPT_RGBA     CPT_RGBA
# define _CPT_TYPE_CPT_POINTER  CPT_POINTER
#endif /* DOXYGEN */

/** @name Type Identifiers
 *
 * @brief These constants define the basic types that can be used to
 *        write pseudo-template code.
 */
/*@{*/

/** @brief Void type. */
#define CPT_VOID      0

/** @brief 8-bit signed integer. */
#define CPT_INT8      1

/** @brief 8-bit unsigned integer. */
#define CPT_UINT8     2

/** @brief 16-bit signed integer. */
#define CPT_INT16     3

/** @brief 16-bit unsigned integer. */
#define CPT_UINT16    4

/** @brief 32-bit signed integer. */
#define CPT_INT32     5

/** @brief 32-bit unsigned integer. */
#define CPT_UINT32    6

/** @brief 64-bit signed integer. */
#define CPT_INT64     7

/** @brief 64-bit unsigned integer. */
#define CPT_UINT64    8

/** @brief Single precision floating-point. */
#define CPT_FLOAT     9

/** @brief Double precision floating-point. */
#define CPT_DOUBLE   10

/** @brief Single precision complex. */
#define CPT_SCOMPLEX 11

/** @brief Double precision complex. */
#define CPT_DCOMPLEX 12

/** @brief Red, green, and blue triplet. */
#define CPT_RGB      13

/** @brief Red, green, blue and alpha quadruplet. */
#define CPT_RGBA     14

/** @brief Pointer. */
#define CPT_POINTER  15

/*@}*/

/*---------------------------------------------------------------------------*/

/** @brief Expand accepted type identifier to standard abbreviation.
 *
 *  @param id   Type identifier. See CPT_TYPE() for a list of accepted
 *              identifiers and the resulting abbreviations.
 */
#define CPT_ABBREV(id)   CPT_JOIN2(_CPT_ABBREV,_CPT_IDENT(id))

#ifndef DOXYGEN
# define _CPT_ABBREV_CPT_VOID      x
# define _CPT_ABBREV_CPT_INT8      i8
# define _CPT_ABBREV_CPT_UINT8     u8
# define _CPT_ABBREV_CPT_INT16     i16
# define _CPT_ABBREV_CPT_UINT16    u16
# define _CPT_ABBREV_CPT_INT32     i32
# define _CPT_ABBREV_CPT_UINT32    u32
# define _CPT_ABBREV_CPT_INT64     i64
# define _CPT_ABBREV_CPT_UINT64    u64
# define _CPT_ABBREV_CPT_FLOAT     f
# define _CPT_ABBREV_CPT_DOUBLE    d
# define _CPT_ABBREV_CPT_SCOMPLEX  c
# define _CPT_ABBREV_CPT_DCOMPLEX  z
# define _CPT_ABBREV_CPT_RGB       rgb
# define _CPT_ABBREV_CPT_RGBA      rgba
# define _CPT_ABBREV_CPT_POINTER   p
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/**
 * @brief Expand type identifier to C-type.
 *
 * @par Examples:
 * @code
 * #define my_real double
 * CPT_CTYPE(CPT_INT64)    // yields: int64_t
 * CPT_CTYPE(my_real)      // yields: double
 * CPT_CTYPE(11)           // yields: void *
 * @endcode
 *
 * @param id   Type identifier. See CPT_TYPE() for a list of accepted
 *             identifiers and the resulting C-type.
 */
#define CPT_CTYPE(id)      CPT_JOIN2(_CPT_CTYPE,_CPT_IDENT(id))

#ifndef DOXYGEN
# define _CPT_CTYPE_CPT_VOID       void
# define _CPT_CTYPE_CPT_INT8       int8_t
# define _CPT_CTYPE_CPT_UINT8      uint8_t
# define _CPT_CTYPE_CPT_INT16      int16_t
# define _CPT_CTYPE_CPT_UINT16     uint16_t
# define _CPT_CTYPE_CPT_INT32      int32_t
# define _CPT_CTYPE_CPT_UINT32     uint32_t
# define _CPT_CTYPE_CPT_INT64      int64_t
# define _CPT_CTYPE_CPT_UINT64     uint64_t
# define _CPT_CTYPE_CPT_FLOAT      float
# define _CPT_CTYPE_CPT_DOUBLE     double
# define _CPT_CTYPE_CPT_SCOMPLEX   cpt_scomplex_t
# define _CPT_CTYPE_CPT_DCOMPLEX   cpt_dcomplex_t
# define _CPT_CTYPE_CPT_RGB        cpt_rgb_t
# define _CPT_CTYPE_CPT_RGBA       cpt_rgba_t
# define _CPT_CTYPE_CPT_POINTER    void *
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/**
 * @brief Expand to C-type of a binary arithmetic operation.
 *
 * Expand to the C-type corresponding of that of the result of a binary
 * arithmetic operation of operands of given types. See CPT_TYPE() for a list
 * of accepted identifiers and the resulting abbreviation.
 *
 * @param id1  Type identifier of left operand.
 * @param id2  Type identifier of right operand.
 */
#define CPT_CTYPE2(id1,id2)  CPT_JOIN3(_CPT_CTYPE2,_CPT_IDENT(id1), _CPT_IDENT(id2))

#ifndef DOXYGEN

# define _CPT_CTYPE2_CPT_INT8_CPT_INT8           int8_t
# define _CPT_CTYPE2_CPT_INT8_CPT_UINT8          uint8_t
# define _CPT_CTYPE2_CPT_INT8_CPT_INT16          int16_t
# define _CPT_CTYPE2_CPT_INT8_CPT_UINT16         uint16_t
# define _CPT_CTYPE2_CPT_INT8_CPT_INT32          int32_t
# define _CPT_CTYPE2_CPT_INT8_CPT_UINT32         uint32_t
# define _CPT_CTYPE2_CPT_INT8_CPT_INT64          int64_t
# define _CPT_CTYPE2_CPT_INT8_CPT_UINT64         uint64_t
# define _CPT_CTYPE2_CPT_INT8_CPT_FLOAT          float
# define _CPT_CTYPE2_CPT_INT8_CPT_DOUBLE         double
# define _CPT_CTYPE2_CPT_INT8_CPT_SCOMPLEX       cpt_scomplex_t
# define _CPT_CTYPE2_CPT_INT8_CPT_DCOMPLEX       cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_UINT8_CPT_INT8          int8_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_UINT8         uint8_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_INT16         int16_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_UINT16        uint16_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_INT32         int32_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_UINT32        uint32_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_INT64         int64_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_UINT64        uint64_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_FLOAT         float
# define _CPT_CTYPE2_CPT_UINT8_CPT_DOUBLE        double
# define _CPT_CTYPE2_CPT_UINT8_CPT_SCOMPLEX      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_UINT8_CPT_DCOMPLEX      cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_INT16_CPT_INT8          int16_t
# define _CPT_CTYPE2_CPT_INT16_CPT_UINT8         int16_t
# define _CPT_CTYPE2_CPT_INT16_CPT_INT16         int16_t
# define _CPT_CTYPE2_CPT_INT16_CPT_UINT16        int16_t
# define _CPT_CTYPE2_CPT_INT16_CPT_INT32         int32_t
# define _CPT_CTYPE2_CPT_INT16_CPT_UINT32        int32_t
# define _CPT_CTYPE2_CPT_INT16_CPT_INT64         int64_t
# define _CPT_CTYPE2_CPT_INT16_CPT_UINT64        int64_t
# define _CPT_CTYPE2_CPT_INT16_CPT_FLOAT         float
# define _CPT_CTYPE2_CPT_INT16_CPT_DOUBLE        double
# define _CPT_CTYPE2_CPT_INT16_CPT_SCOMPLEX      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_INT16_CPT_DCOMPLEX      cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_UINT16_CPT_INT8         int16_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_UINT8        uint16_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_INT16        int16_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_UINT16       uint16_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_INT32        int32_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_UINT32       uint32_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_INT64        int64_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_UINT64       uint64_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_FLOAT        float
# define _CPT_CTYPE2_CPT_UINT16_CPT_DOUBLE       double
# define _CPT_CTYPE2_CPT_UINT16_CPT_SCOMPLEX     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_UINT16_CPT_DCOMPLEX     cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_INT32_CPT_INT8          int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_UINT8         int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_INT16         int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_UINT16        int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_INT32         int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_UINT32        int32_t
# define _CPT_CTYPE2_CPT_INT32_CPT_INT64         int64_t
# define _CPT_CTYPE2_CPT_INT32_CPT_UINT64        int64_t
# define _CPT_CTYPE2_CPT_INT32_CPT_FLOAT         float
# define _CPT_CTYPE2_CPT_INT32_CPT_DOUBLE        double
# define _CPT_CTYPE2_CPT_INT32_CPT_SCOMPLEX      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_INT32_CPT_DCOMPLEX      cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_UINT32_CPT_INT8         int32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_UINT8        uint32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_INT16        int32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_UINT16       uint32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_INT32        int32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_UINT32       uint32_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_INT64        int64_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_UINT64       uint64_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_FLOAT        float
# define _CPT_CTYPE2_CPT_UINT32_CPT_DOUBLE       double
# define _CPT_CTYPE2_CPT_UINT32_CPT_SCOMPLEX     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_UINT32_CPT_DCOMPLEX     cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_INT64_CPT_INT8          int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_UINT8         int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_INT16         int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_UINT16        int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_INT32         int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_UINT32        int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_INT64         int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_UINT64        int64_t
# define _CPT_CTYPE2_CPT_INT64_CPT_FLOAT         float
# define _CPT_CTYPE2_CPT_INT64_CPT_DOUBLE        double
# define _CPT_CTYPE2_CPT_INT64_CPT_SCOMPLEX      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_INT64_CPT_DCOMPLEX      cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_UINT64_CPT_INT8         int64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_UINT8        uint64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_INT16        int64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_UINT16       uint64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_INT32        int64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_UINT32       uint64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_INT64        int64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_UINT64       uint64_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_FLOAT        float
# define _CPT_CTYPE2_CPT_UINT64_CPT_DOUBLE       double
# define _CPT_CTYPE2_CPT_UINT64_CPT_SCOMPLEX     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_UINT64_CPT_DCOMPLEX     cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_FLOAT_CPT_INT8          float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_UINT8         float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_INT16         float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_UINT16        float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_INT32         float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_UINT32        float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_INT64         float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_UINT64        float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_FLOAT         float
# define _CPT_CTYPE2_CPT_FLOAT_CPT_DOUBLE        double
# define _CPT_CTYPE2_CPT_FLOAT_CPT_SCOMPLEX      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_FLOAT_CPT_DCOMPLEX      cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_DOUBLE_CPT_INT8         double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_UINT8        double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_INT16        double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_UINT16       double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_INT32        double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_UINT32       double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_INT64        double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_UINT64       double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_FLOAT        double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_DOUBLE       double
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_SCOMPLEX     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_DOUBLE_CPT_DCOMPLEX     cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_INT8       cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_UINT8      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_INT16      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_UINT16     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_INT32      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_UINT32     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_INT64      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_UINT64     cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_FLOAT      cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_DOUBLE     cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_SCOMPLEX   cpt_scomplex_t
# define _CPT_CTYPE2_CPT_SCOMPLEX_CPT_DCOMPLEX   cpt_dcomplex_t

# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_INT8       cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_UINT8      cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_INT16      cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_UINT16     cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_INT32      cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_UINT32     cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_INT64      cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_UINT64     cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_FLOAT      cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_DOUBLE     cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_SCOMPLEX   cpt_dcomplex_t
# define _CPT_CTYPE2_CPT_DCOMPLEX_CPT_DCOMPLEX   cpt_dcomplex_t

#endif /* DOXYGEN */

/*--------------------------------------------------------------------------*/

/** @brief Check whether @a id corresponds to an integer type. */
#define CPT_IS_INTEGER(id)   _CPT_IS_INTEGER(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a floating-point type. */
#define CPT_IS_REAL(id)      _CPT_IS_REAL(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a complex type. */
#define CPT_IS_COMPLEX(id)   _CPT_IS_COMPLEX(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to an unsigned integer type. */
#define CPT_IS_UNSIGNED(id)  _CPT_IS_UNSIGNED(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a signed integer type. */
#define CPT_IS_SIGNED(id)    _CPT_IS_SIGNED(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a color (RGN or RGBA) type. */
#define CPT_IS_COLOR(id)     _CPT_IS_COLOR(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a void type. */
#define CPT_IS_VOID(id)      _CPT_IS_VOID(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 8-bit signed integer. */
#define CPT_IS_INT8(id)      _CPT_IS_INT8(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 8-bit unsigned integer. */
#define CPT_IS_UINT8(id)     _CPT_IS_UINT8(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 16-bit signed integer. */
#define CPT_IS_INT16(id)     _CPT_IS_INT16(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 16-bit unsigned integer. */
#define CPT_IS_UINT16(id)    _CPT_IS_UINT16(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 32-bit signed integer. */
#define CPT_IS_INT32(id)     _CPT_IS_INT32(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 32-bit unsigned integer. */
#define CPT_IS_UINT32(id)    _CPT_IS_UINT32(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 64-bit signed integer. */
#define CPT_IS_INT64(id)     _CPT_IS_INT64(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a 64-bit unsigned integer. */
#define CPT_IS_UINT64(id)    _CPT_IS_UINT64(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a single precision floating-point. */
#define CPT_IS_FLOAT(id)     _CPT_IS_FLOAT(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a double precision floating-point. */
#define CPT_IS_DOUBLE(id)    _CPT_IS_DOUBLE(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a single precision complex. */
#define CPT_IS_SCOMPLEX(id)  _CPT_IS_SCOMPLEX(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a double precision complex. */
#define CPT_IS_DCOMPLEX(id)  _CPT_IS_DCOMPLEX(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to (red, green, blue) triplet. */
#define CPT_IS_RGB(id)       _CPT_IS_RGB(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to (red, green, blue, alpha) quadruplet. */
#define CPT_IS_RGBA(id)      _CPT_IS_RGBA(CPT_TYPE(id))

/** @brief Check whether @a id corresponds to a pointer type. */
#define CPT_IS_POINTER(id)   _CPT_IS_POINTER(CPT_TYPE(id))

#ifndef DOXYGEN
# define _CPT_IS_INTEGER(n)   ((n) > CPT_VOID && (n) < CPT_FLOAT)
# define _CPT_IS_REAL(n)      ((n) == CPT_FLOAT || (n) == CPT_DOUBLE)
# define _CPT_IS_COMPLEX(n)   ((n) == CPT_SCOMPLEX || (n) == CPT_DCOMPLEX)
# define _CPT_IS_UNSIGNED(n)  (_CPT_IS_INTEGER(n) && (n)%2 == 0)
# define _CPT_IS_SIGNED(n)    (_CPT_IS_INTEGER(n) && (n)%2 == 1)
# define _CPT_IS_VOID(n)      ((n) == CPT_VOID)
# define _CPT_IS_COLOR(n)     ((n) == CPT_RGB || (n) == CPT_RGBA)
# define _CPT_IS_INT8(n)      ((n) == CPT_INT8)
# define _CPT_IS_UINT8(n)     ((n) == CPT_UINT8)
# define _CPT_IS_INT16(n)     ((n) == CPT_INT16)
# define _CPT_IS_UINT16(n)    ((n) == CPT_UINT16)
# define _CPT_IS_INT32(n)     ((n) == CPT_INT32)
# define _CPT_IS_UINT32(n)    ((n) == CPT_UINT32)
# define _CPT_IS_INT64(n)     ((n) == CPT_INT64)
# define _CPT_IS_UINT64(n)    ((n) == CPT_UINT64)
# define _CPT_IS_FLOAT(n)     ((n) == CPT_FLOAT)
# define _CPT_IS_DOUBLE(n)    ((n) == CPT_DOUBLE)
# define _CPT_IS_SCOMPLEX(n)  ((n) == CPT_SCOMPLEX)
# define _CPT_IS_DCOMPLEX(n)  ((n) == CPT_DCOMPLEX)
# define _CPT_IS_RGB(n)       ((n) == CPT_RGB)
# define _CPT_IS_RGBA(n)      ((n) == CPT_RGBA)
# define _CPT_IS_POINTER(n)   ((n) == CPT_POINTER)
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/** @brief Yield size of basic data type.
 *
 *  This macro yields size (in bytes) of data type according to its
 *  argument (see CPT_TYPE()).
 */
#define CPT_SIZEOF(id)  CPT_JOIN2(_CPT_SIZEOF,_CPT_IDENT(id))

/** @brief Size in bytes of a 8-bit signed integer. */
#define CPT_SIZEOF_INT8     1

/** @brief Size in bytes of a 8-bit unsigned integer. */
#define CPT_SIZEOF_UINT8    1

/** @brief Size in bytes of a 16-bit signed integer. */
#define CPT_SIZEOF_INT16    2

/** @brief Size in bytes of a 16-bit unsigned integer. */
#define CPT_SIZEOF_UINT16   2

/** @brief Size in bytes of a 32-bit signed integer. */
#define CPT_SIZEOF_INT32    4

/** @brief Size in bytes of a 32-bit unsigned integer. */
#define CPT_SIZEOF_UINT32   4

/** @brief Size in bytes of a 64-bit signed integer. */
#define CPT_SIZEOF_INT64    8

/** @brief Size in bytes of a 64-bit unsigned integer. */
#define CPT_SIZEOF_UINT64   8

/* FIXME: the following should be constants */

/** @brief Size in bytes of a single precision floating-point. */
#define CPT_SIZEOF_FLOAT    sizeof(float)

/** @brief Size in bytes of a double precision floating-point. */
#define CPT_SIZEOF_DOUBLE   sizeof(double)

/** @brief Size in bytes of a single precision complex. */
#define CPT_SIZEOF_SCOMPLEX (2*CPT_SIZEOF_FLOAT)

/** @brief Size in bytes of a double precision complex. */
#define CPT_SIZEOF_DCOMPLEX (2*CPT_SIZEOF_double)

/** @brief Size in bytes of an (red, green, blue) triplet. */
#define CPT_SIZEOF_RGB      3

/** @brief Size in bytes of an (red, green, blue, alpha) quadruplet. */
#define CPT_SIZEOF_RGBA     4

/** @brief Size in bytes of a pointer. */
#define CPT_SIZEOF_POINTER  sizeof(void *)

#ifndef DOXYGEN
# define _CPT_SIZEOF_CPT_INT8     CPT_SIZEOF_INT8
# define _CPT_SIZEOF_CPT_UINT8    CPT_SIZEOF_UINT8
# define _CPT_SIZEOF_CPT_INT16    CPT_SIZEOF_INT16
# define _CPT_SIZEOF_CPT_UINT16   CPT_SIZEOF_UINT16
# define _CPT_SIZEOF_CPT_INT32    CPT_SIZEOF_INT32
# define _CPT_SIZEOF_CPT_UINT32   CPT_SIZEOF_UINT32
# define _CPT_SIZEOF_CPT_INT64    CPT_SIZEOF_INT64
# define _CPT_SIZEOF_CPT_UINT64   CPT_SIZEOF_UINT64
# define _CPT_SIZEOF_CPT_FLOAT    CPT_SIZEOF_FLOAT
# define _CPT_SIZEOF_CPT_DOUBLE   CPT_SIZEOF_DOUBLE
# define _CPT_SIZEOF_CPT_SCOMPLEX CPT_SIZEOF_SCOMPLEX
# define _CPT_SIZEOF_CPT_DCOMPLEX CPT_SIZEOF_DCOMPLEX
# define _CPT_SIZEOF_CPT_RGB      CPT_SIZEOF_RGB
# define _CPT_SIZEOF_CPT_RGBA     CPT_SIZEOF_RGBA
# define _CPT_SIZEOF_CPT_POINTER  CPT_SIZEOF_POINTER
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/** @brief Yield minimum value of basic data type.
 *
 *  This macro yields the minimum value of data type according to its
 *  argument (see CPT_TYPE()).
 */
#define CPT_MIN_VALUE(id)  CPT_JOIN2(_CPT_MIN,_CPT_IDENT(id))

/** @brief Yield maximum value of basic data type.
 *
 *  This macro yields the maximum value of data type according to its
 *  argument (see CPT_TYPE()).
 */
#define CPT_MAX_VALUE(id)  CPT_JOIN2(_CPT_MAX,_CPT_IDENT(id))

/** @brief Minimum value of a 8-bit signed integer. */
#define CPT_MIN_INT8  (-128)

/** @brief Maximum value of a 8-bit signed integer. */
#define CPT_MAX_INT8    127

/** @brief Minimum value of a 8-bit unsigned integer. */
#define CPT_MIN_UINT8     0

/** @brief Maximum value of a 8-bit unsigned integer. */
#define CPT_MAX_UINT8   255

/** @brief Minimum value of a 16-bit signed integer. */
#define CPT_MIN_INT16  (-32768)

/** @brief Maximum value of a 16-bit signed integer. */
#define CPT_MAX_INT16    32767

/** @brief Minimum value of a 16-bit unsigned integer. */
#define CPT_MIN_UINT16       0

/** @brief Maximum value of a 16-bit unsigned integer. */
#define CPT_MAX_UINT16   65535

/** @brief Minimum value of a 32-bit signed integer. */
#define CPT_MIN_INT32  (-CPT_MAX_INT32 - 1)

/** @brief Maximum value of a 32-bit signed integer. */
#define CPT_MAX_INT32    2147483647L

/** @brief Minimum value of a 32-bit unsigned integer. */
#define CPT_MIN_UINT32     0

/** @brief Maximum value of a 32-bit unsigned integer. */
#define CPT_MAX_UINT32  (CPT_MAX_INT32*2UL + 1)

/** @brief Minimum value of a 64-bit signed integer. */
#define CPT_MIN_INT64  (-CPT_MAX_INT64 - 1)

/** @brief Maximum value of a 64-bit signed integer. */
#define CPT_MAX_INT64    9223372036854775807LL

/** @brief Minimum value of a 64-bit unsigned integer. */
#define CPT_MIN_UINT64     0

/** @brief Maximum value of a 64-bit unsigned integer. */
#define CPT_MAX_UINT64   (CPT_MAX_INT64*2ULL + 1)

/** @brief Minimum value of a single precision floating-point. */
#define CPT_MIN_FLOAT     (-FLT_MAX)

/** @brief Maximum value of a single precision floating-point. */
#define CPT_MAX_FLOAT       FLT_MAX

/** @brief Minimum value of a double precision floating-point. */
#define CPT_MIN_DOUBLE    (-DBL_MAX)

/** @brief Maximum value of a double precision floating-point. */
#define CPT_MAX_DOUBLE      DBL_MAX

#ifndef DOXYGEN
# define _CPT_MIN_CPT_INT8     CPT_MIN_INT8
# define _CPT_MIN_CPT_UINT8    CPT_MIN_UINT8
# define _CPT_MIN_CPT_INT16    CPT_MIN_INT16
# define _CPT_MIN_CPT_UINT16   CPT_MIN_UINT16
# define _CPT_MIN_CPT_INT32    CPT_MIN_INT32
# define _CPT_MIN_CPT_UINT32   CPT_MIN_UINT32
# define _CPT_MIN_CPT_INT64    CPT_MIN_INT64
# define _CPT_MIN_CPT_UINT64   CPT_MIN_UINT64
# define _CPT_MIN_CPT_FLOAT    CPT_MIN_FLOAT
# define _CPT_MIN_CPT_DOUBLE   CPT_MIN_DOUBLE
# define _CPT_MAX_CPT_INT8     CPT_MAX_INT8
# define _CPT_MAX_CPT_UINT8    CPT_MAX_UINT8
# define _CPT_MAX_CPT_INT16    CPT_MAX_INT16
# define _CPT_MAX_CPT_UINT16   CPT_MAX_UINT16
# define _CPT_MAX_CPT_INT32    CPT_MAX_INT32
# define _CPT_MAX_CPT_UINT32   CPT_MAX_UINT32
# define _CPT_MAX_CPT_INT64    CPT_MAX_INT64
# define _CPT_MAX_CPT_UINT64   CPT_MAX_UINT64
# define _CPT_MAX_CPT_FLOAT    CPT_MAX_FLOAT
# define _CPT_MAX_CPT_DOUBLE   CPT_MAX_DOUBLE
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/** 
 * @brief Expand known type identifier into internal suffix.
 *
 * Expand known (see CPT_TYPE) type identifier to unique internal suffix.
 *
 * @param id   Type identifier, must expand to one of:
 *               - integer in the range [0,10]
 *               - abbreviated type: i8, u8, i16, u16, ..., s, d
 *               - standard C type: int8_t, uint8_t, ..., float, double
 *               - uppercase name: INT8, UINT8, ..., FLOAT, DOUBLE
 *               - macro type name: CPT_INT8, CPT_UINT8, ..., CPT_FLOAT, ...
 */
#define _CPT_IDENT(id)   _CPT_JOIN2(_CPT_IDENT_,id) /* do macro expansion */

#ifndef DOXYGEN

# define _CPT_IDENT_VOID      _CPT_VOID
# define _CPT_IDENT_INT8      _CPT_INT8
# define _CPT_IDENT_UINT8     _CPT_UINT8
# define _CPT_IDENT_INT16     _CPT_INT16
# define _CPT_IDENT_UINT16    _CPT_UINT16
# define _CPT_IDENT_INT32     _CPT_INT32
# define _CPT_IDENT_UINT32    _CPT_UINT32
# define _CPT_IDENT_INT64     _CPT_INT64
# define _CPT_IDENT_UINT64    _CPT_UINT64
# define _CPT_IDENT_FLOAT     _CPT_FLOAT
# define _CPT_IDENT_DOUBLE    _CPT_DOUBLE
# define _CPT_IDENT_SCOMPLEX  _CPT_SCOMPLEX
# define _CPT_IDENT_DCOMPLEX  _CPT_DCOMPLEX
# define _CPT_IDENT_RGB       _CPT_RGB
# define _CPT_IDENT_RGBA      _CPT_RGBA
# define _CPT_IDENT_POINTER   _CPT_POINTER

# define _CPT_IDENT_0            _CPT_IDENT_VOID
# define _CPT_IDENT_1            _CPT_IDENT_INT8
# define _CPT_IDENT_2            _CPT_IDENT_UINT8
# define _CPT_IDENT_3            _CPT_IDENT_INT16
# define _CPT_IDENT_4            _CPT_IDENT_UINT16
# define _CPT_IDENT_5            _CPT_IDENT_INT32
# define _CPT_IDENT_6            _CPT_IDENT_UINT32
# define _CPT_IDENT_7            _CPT_IDENT_INT64
# define _CPT_IDENT_8            _CPT_IDENT_UINT64
# define _CPT_IDENT_9            _CPT_IDENT_FLOAT
# define _CPT_IDENT_10           _CPT_IDENT_DOUBLE
# define _CPT_IDENT_11           _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_12           _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_13           _CPT_IDENT_RGB
# define _CPT_IDENT_14           _CPT_IDENT_RGBA
# define _CPT_IDENT_15           _CPT_IDENT_POINTER

# define _CPT_IDENT_CPT_VOID     _CPT_IDENT_VOID
# define _CPT_IDENT_CPT_INT8     _CPT_IDENT_INT8
# define _CPT_IDENT_CPT_UINT8    _CPT_IDENT_UINT8
# define _CPT_IDENT_CPT_INT16    _CPT_IDENT_INT16
# define _CPT_IDENT_CPT_UINT16   _CPT_IDENT_UINT16
# define _CPT_IDENT_CPT_INT32    _CPT_IDENT_INT32
# define _CPT_IDENT_CPT_UINT32   _CPT_IDENT_UINT32
# define _CPT_IDENT_CPT_INT64    _CPT_IDENT_INT64
# define _CPT_IDENT_CPT_UINT64   _CPT_IDENT_UINT64
# define _CPT_IDENT_CPT_FLOAT    _CPT_IDENT_FLOAT
# define _CPT_IDENT_CPT_DOUBLE   _CPT_IDENT_DOUBLE
# define _CPT_IDENT_CPT_SCOMPLEX _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_CPT_DCOMPLEX _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_CPT_RGB      _CPT_IDENT_RGB
# define _CPT_IDENT_CPT_RGBA     _CPT_IDENT_RGBA
# define _CPT_IDENT_CPT_POINTER  _CPT_IDENT_POINTER

# define _CPT_IDENT_x            _CPT_IDENT_VOID
# define _CPT_IDENT_i8           _CPT_IDENT_INT8
# define _CPT_IDENT_u8           _CPT_IDENT_UINT8
# define _CPT_IDENT_i16          _CPT_IDENT_INT16
# define _CPT_IDENT_u16          _CPT_IDENT_UINT16
# define _CPT_IDENT_i32          _CPT_IDENT_INT32
# define _CPT_IDENT_u32          _CPT_IDENT_UINT32
# define _CPT_IDENT_i64          _CPT_IDENT_INT64
# define _CPT_IDENT_u64          _CPT_IDENT_UINT64
# define _CPT_IDENT_f            _CPT_IDENT_FLOAT
# define _CPT_IDENT_d            _CPT_IDENT_DOUBLE
# define _CPT_IDENT_c            _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_z            _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_rgb          _CPT_IDENT_RGB
# define _CPT_IDENT_rgba         _CPT_IDENT_RGBA
# define _CPT_IDENT_p            _CPT_IDENT_POINTER

# define _CPT_IDENT_void           _CPT_IDENT_VOID
# define _CPT_IDENT_int8_t         _CPT_IDENT_INT8
# define _CPT_IDENT_uint8_t        _CPT_IDENT_UINT8
# define _CPT_IDENT_int16_t        _CPT_IDENT_INT16
# define _CPT_IDENT_uint16_t       _CPT_IDENT_UINT16
# define _CPT_IDENT_int32_t        _CPT_IDENT_INT32
# define _CPT_IDENT_uint32_t       _CPT_IDENT_UINT32
# define _CPT_IDENT_int64_t        _CPT_IDENT_INT64
# define _CPT_IDENT_uint64_t       _CPT_IDENT_UINT64
# define _CPT_IDENT_float          _CPT_IDENT_FLOAT
# define _CPT_IDENT_double         _CPT_IDENT_DOUBLE
# define _CPT_IDENT_scomplex       _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_dcomplex       _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_scomplex_t     _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_dcomplex_t     _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_cpt_scomplex_t _CPT_IDENT_SCOMPLEX
# define _CPT_IDENT_cpt_dcomplex_t _CPT_IDENT_DCOMPLEX
# define _CPT_IDENT_rgb_t          _CPT_IDENT_RGB
# define _CPT_IDENT_rgba_t         _CPT_IDENT_RGBA
# define _CPT_IDENT_cpt_rgb_t      _CPT_IDENT_RGB
# define _CPT_IDENT_cpt_rgba_t     _CPT_IDENT_RGBA
# define _CPT_IDENT_pointer        _CPT_IDENT_POINTER

#endif /* DOXYGEN */

/* For the macros defined in this file to work, the (private) names used as
   unique suffixes must not be defined (that is, they must not expand to
   something else). */
#ifdef _CPT_VOID
# warning macro _CPT_VOID defined
# undef _CPT_VOID
#endif
#ifdef _CPT_INT8
# warning macro _CPT_INT8 defined
# undef _CPT_INT8
#endif
#ifdef _CPT_UINT8
# warning macro _CPT_UINT8 defined
# undef _CPT_UINT8
#endif
#ifdef _CPT_INT16
# warning macro _CPT_INT16 defined
# undef _CPT_INT16
#endif
#ifdef _CPT_UINT16
# warning macro _CPT_UINT16 defined
# undef _CPT_UINT16
#endif
#ifdef _CPT_INT32
# warning macro _CPT_INT32 defined
# undef _CPT_INT32
#endif
#ifdef _CPT_UINT32
# warning macro _CPT_UINT32 defined
# undef _CPT_UINT32
#endif
#ifdef _CPT_INT64
# warning macro _CPT_INT64 defined
# undef _CPT_INT64
#endif
#ifdef _CPT_UINT64
# warning macro _CPT_UINT64 defined
# undef _CPT_UINT64
#endif
#ifdef _CPT_FLOAT
# warning macro _CPT_FLOAT defined
# undef _CPT_FLOAT
#endif
#ifdef _CPT_DOUBLE
# warning macro _CPT_DOUBLE defined
# undef _CPT_DOUBLE
#endif
#ifdef _CPT_SCOMPLEX
# warning macro _CPT_SCOMPLEX defined
# undef _CPT_SCOMPLEX
#endif
#ifdef _CPT_DCOMPLEX
# warning macro _CPT_DCOMPLEX defined
# undef _CPT_DCOMPLEX
#endif
#ifdef _CPT_RGB
# warning macro _CPT_RGB defined
# undef _CPT_RGB
#endif
#ifdef _CPT_RGBA
# warning macro _CPT_RGBA defined
# undef _CPT_RGBA
#endif
#ifdef _CPT_POINTER
# warning macro _CPT_POINTER defined
# undef _CPT_POINTER
#endif

/** @brief Concatenate arguments with expansion.  */
#define CPT_JOIN(a,b)                          _CPT_JOIN2(a,b)
#ifndef DOXYGEN
# define CPT_JOIN2(a1,a2)                      _CPT_JOIN2(a1,a2)
# define CPT_JOIN3(a1,a2,a3)                   _CPT_JOIN3(a1,a2,a3)
# define CPT_JOIN4(a1,a2,a3,a4)                _CPT_JOIN4(a1,a2,a3,a4)
# define CPT_JOIN5(a1,a2,a3,a4,a5)             _CPT_JOIN5(a1,a2,a3,a4,a5)
# define CPT_JOIN6(a1,a2,a3,a4,a5,a6)          _CPT_JOIN6(a1,a2,a3,a4,a5,a6)
# define CPT_JOIN7(a1,a2,a3,a4,a5,a6,a7)       _CPT_JOIN7(a1,a2,a3,a4,a5,a6,a7)
# define CPT_JOIN8(a1,a2,a3,a4,a5,a6,a7,a8)    _CPT_JOIN8(a1,a2,a3,a4,a5,a6,a7,a8)
# define CPT_JOIN9(a1,a2,a3,a4,a5,a6,a7,a8,a9) _CPT_JOIN9(a1,a2,a3,a4,a5,a6,a7,a8,a9)
#endif /* DOXYGEN */

#ifndef DOXYGEN
/** @brief Concatenate arguments without expansion. */
# define _CPT_JOIN2(a1,a2)                      a1##a2
# define _CPT_JOIN3(a1,a2,a3)                   a1##a2##a3
# define _CPT_JOIN4(a1,a2,a3,a4)                a1##a2##a3##a4
# define _CPT_JOIN5(a1,a2,a3,a4,a5)             a1##a2##a3##a4##a5
# define _CPT_JOIN6(a1,a2,a3,a4,a5,a6)          a1##a2##a3##a4##a5##a6
# define _CPT_JOIN7(a1,a2,a3,a4,a5,a6,a7)       a1##a2##a3##a4##a5##a6##a7
# define _CPT_JOIN8(a1,a2,a3,a4,a5,a6,a7,a8)    a1##a2##a3##a4##a5##a6##a7##a8
# define _CPT_JOIN9(a1,a2,a3,a4,a5,a6,a7,a8,a9) a1##a2##a3##a4##a5##a6##a7##a8##a9
# if 0
/* Note: the same behaviour is obtained with the following definitions: */
#  define _CPT_JOIN(a1,a2)                      a1##a2
#  define CPT_JOIN(a,b)                         _CPT_JOIN(a,b)
#  define CPT_JOIN2(a1,a2)                      CPT_JOIN(a1,a2)
#  define CPT_JOIN3(a1,a2,a3)                   CPT_JOIN(a1,CPT_JOIN2(a2,a3))
#  define CPT_JOIN4(a1,a2,a3,a4)                CPT_JOIN(a1,CPT_JOIN3(a2,a3,a4))
#  define CPT_JOIN5(a1,a2,a3,a4,a5)             CPT_JOIN(a1,CPT_JOIN4(a2,a3,a4,a5))
#  define CPT_JOIN6(a1,a2,a3,a4,a5,a6)          CPT_JOIN(a1,CPT_JOIN5(a2,a3,a4,a5,a6))
# endif
#endif /* DOXYGEN */

/** @brief Make argument into a string.
 *
 * This macro expands its argument and makes it into a string.
 */
#define CPT_STRINGIFY(a) _CPT_STRINGIFY(a) /* expand argument */
#ifndef DOXYGEN
# define _CPT_STRINGIFY(a) #a
#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/* Use constants in limits.h to setup macro definitions for basic integer
   types. */

#ifndef DOXYGEN

#define CPT_UINT8_MAX     255 
#define CPT_INT8_MIN    (-128) 
#define CPT_INT8_MAX      127 
#define CPT_INT16_MIN	(-32768)
#define CPT_INT16_MAX	  32767
#define CPT_UINT16_MAX	  65535U
#define CPT_INT32_MIN	(-CPT_INT32_MAX - 1L)
#define CPT_INT32_MAX	 2147483647L
#define CPT_UINT32_MAX	 4294967295UL
#define CPT_INT64_MIN	(-CPT_INT64_MAX - 1L)
#define CPT_INT64_MAX	 9223372036854775807LL
#define CPT_UINT64_MAX	18446744073709551615ULL

/* ANSI-C norm specifies that "char" (not necessarily same as "signed char")
   has at least 8 bits with a minimal magnitude of -127 to +127 for the
   "signed char" and 0 to 255 for the "unsigned char". */
#if (CHAR_MIN == CPT_INT8_MIN && CHAR_MAX == CPT_INT8_MAX)
# define  CPT_CHAR         CPT_INT8
# define _CPT_IDENT_char  _CPT_IDENT_INT8
#elif (CHAR_MIN == 0 && CHAR_MAX == CPT_UINT8_MAX)
# define  CPT_CHAR         CPT_UINT8
# define _CPT_IDENT_char  _CPT_IDENT_UINT8
#endif
#if (UCHAR_MAX == CPT_UINT8_MAX)
# define  CPT_UCHAR         CPT_UINT8
# define _CPT_IDENT_uchar  _CPT_IDENT_UINT8
#endif

/* ANSI-C norm specifies that "short" (same as "signed short") has at least 16
   bits with a minimal magnitude of -32767 to +32767 for the "signed short"
   and 0 to 65535 for the "unsigned short". */
#if (USHRT_MAX == CPT_UINT16_MAX)
#  define  CPT_SHORT            CPT_INT16
#  define  CPT_USHORT           CPT_UINT16
#  define _CPT_IDENT_short     _CPT_IDENT_INT16
#  define _CPT_IDENT_ushort    _CPT_IDENT_UINT16
#elif (USHRT_MAX == CPT_UINT32_MAX)
#  define  CPT_SHORT            CPT_INT32
#  define  CPT_USHORT           CPT_UINT32
#  define _CPT_IDENT_short     _CPT_IDENT_INT32
#  define _CPT_IDENT_ushort    _CPT_IDENT_UINT32
#elif (USHRT_MAX == CPT_UINT64_MAX)
#  define  CPT_SHORT            CPT_INT64
#  define  CPT_USHORT           CPT_UINT64
#  define _CPT_IDENT_short     _CPT_IDENT_INT64
#  define _CPT_IDENT_ushort    _CPT_IDENT_UINT64
#endif

/* ANSI-C norm specifies that "int" (same as "signed int") has at least 16
   bits with a minimal magnitude of -32767 to +32767 for the "signed int" and
   0 to 65535 for the "unsigned int". */
#if (UINT_MAX == CPT_UINT16_MAX)
# define  CPT_INT            CPT_INT16
# define  CPT_UINT           CPT_UINT16
# define _CPT_IDENT_int     _CPT_IDENT_INT16
# define _CPT_IDENT_uint    _CPT_IDENT_UINT16
#elif (UINT_MAX == CPT_UINT32_MAX)
# define  CPT_INT            CPT_INT32
# define  CPT_UINT           CPT_UINT32
# define _CPT_IDENT_int     _CPT_IDENT_INT32
# define _CPT_IDENT_uint    _CPT_IDENT_UINT32
#elif (UINT_MAX == CPT_UINT64_MAX)
# define  CPT_INT            CPT_INT64
# define  CPT_UINT           CPT_UINT64
# define _CPT_IDENT_int     _CPT_IDENT_INT64
# define _CPT_IDENT_uint    _CPT_IDENT_UINT64
#endif

/* ANSI-C norm specifies that "long" (same as "signed long") has at least 32
   bits with a minimal magnitude of -2147483647 to +2147483647 for the "signed
   long" and 0 to 4294967295 for the "unsigned long". */
#if (ULONG_MAX == CPT_UINT32_MAX)
# define  CPT_LONG           CPT_INT32
# define  CPT_ULONG          CPT_UINT32
# define _CPT_IDENT_long    _CPT_IDENT_INT32
# define _CPT_IDENT_ulong   _CPT_IDENT_UINT32
#elif (ULONG_MAX == CPT_UINT64_MAX)
# define  CPT_LONG           CPT_INT64
# define  CPT_ULONG          CPT_UINT64
# define _CPT_IDENT_long    _CPT_IDENT_INT64
# define _CPT_IDENT_ulong   _CPT_IDENT_UINT64
#endif

/* ANSI-C norm specifies that "long long" (same as "signed long long") has at
   least 64 bits with a minimal magnitude of -9223372036854775807 to
   +9223372036854775807 for the "signed long long" and 0 to
   8446744073709551615 for the "unsigned long long". */
#if (ULLONG_MAX == CPT_UINT64_MAX)
# define  CPT_LLONG          CPT_INT64
# define  CPT_ULLONG         CPT_UINT64
# define _CPT_IDENT_llong   _CPT_IDENT_INT64
# define _CPT_IDENT_ullong  _CPT_IDENT_UINT64
#endif

#endif /* DOXYGEN */

/*---------------------------------------------------------------------------*/

/* Check that macros are properly working. */

#if CPT_IS_INTEGER(float) || ! CPT_IS_REAL(float) || ! CPT_IS_FLOAT(float)
# error float is integer, or not real, or not float (BUG)
#endif

#if ! CPT_IS_INTEGER(uint32_t) || ! CPT_IS_UNSIGNED(uint32_t)
# error uint32_t is not an unsigned integer (BUG)
#endif

/** @} */

#endif /* _CPT_H ------------------------------------------------------------*/

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * c-basic-offset: 2
 * fill-column: 78
 * coding: utf-8
 * End:
 */
