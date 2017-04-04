/*
 * image.i --
 *
 * Yorick plugin for image processing.
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

if (is_func(plug_in)) plug_in, "yimage";

/*---------------------------------------------------------------------------*/
/* BASIC IMAGE FUNCTIONS */

extern _img_init;
/* DOCUMENT _img_init;
     This private function initializes the internals of the "Image" plug-in.
     It is automatically called at statrtup and should not be needed
     otherwise.

   SEE ALSO is_image, img_get_version. */
_img_init; /* initializes internals */

extern img_get_version;
/* DOCUMENT img_get_version();
         or img_get_version(n);
     This function returns the version of the "Image" plug-in.  If the
     argument is nil, a string of the form "MAJOR.MINOR.PATCH" is returned.
     Otherwise, the major version number, the minor version number, or the
     patch level is returned as an integer depending whether N is 0, 1 or 2.

   SEE ALSO is_image. */
_img_init; /* initializes internals */

extern is_image;
/* DOCUMENT is_image(img);
     This function checks whether IMG is a valid image.  The returned value
     is: 0 if IMG is not a valid image;
         1 if IMG is a grayscale image;
         2 if IMG is a complex image;
         3 if IMG is a color image (RGB or RGBA).
     Images of different types correspond to Yorick numerical arrays as
     follows (with WIDTH and HEIGHT the dimensions of the image):
      - Grayscale images are any WIDTH-by-HEIGHT integer or real arrays.
      - Complex images are WIDTH-by-HEIGHT complex arrays.
      - RGB images are 3-by-WIDTH-by-HEIGHT byte arrays with the first
        dimension corresponding to the red, green, and blue channels
        respectively.
      - RGBA images are 4-by-WIDTH-by-HEIGHT byte arrays with the first
        dimension corresponding to the red, green, blue, and alpha
        (transparency) channels respectively.

   SEE ALSO pli, img_is_rgb, img_get_type, img_get_width, img_get_height. */


extern img_is_complex;
/* DOCUMENT img_is_complex(img);

     This function check whether IMG is a complex image.  An error is raised
     if IMG is not a valid image (see is_image for the definition of a valid
     image).

   SEE ALSO is_image, img_get_channel. */

extern img_is_color;
extern img_is_rgb;
extern img_is_rgba;
/* DOCUMENT img_is_color(img);
         or img_is_rgb(img);
         or img_is_rgba(img);
     These functions check whether IMG is a color image, an RGB image or an
     RGBA image.  An error is raised if IMG is not a valid image (see is_image
     for the definition of a valid image).

   SEE ALSO is_image, img_get_channel. */

extern img_get_width;
extern img_get_height;
/* DOCUMENT img_get_width(img);
         or img_get_height(img);
     These functions return the width and the height of image IMG. An error is
     raised if IMG is not a valid image (see is_image for the definition
     of a valid image).

   SEE ALSO is_image */

func img_get_symbol(__1)
/* DOCUMENT img_get_symbol(name)
     Returns the current definition of symbol NAME (which can be []).  This
     function is a workaround symbol_def function for which the symbol must
     exist.  The side effect is that NAME is created in Yorick table of global
     symbol if it does not yet exists.

   SEE ALSO: symbol_def, catch.
 */
{
  // FIXME: an alternative is to use Yeti's symbol_exists function.
  //if (is_func(symbol_exists)) {
  //  if (symbol_exists(__1)) {
  //    return symbol_def(__1);
  //  }
  //  return;
  //}
  if (catch(0x08)) return;
  return symbol_def(__1);
}

func img_define_constant(__1, __2)
/* DOCUMENT img_define_constant, name, value;
     This subroutine defines a global constant in Yorick.  An error is
     raised if the constant is already defined with a different value.

   SEE ALSO: img_get_symbol.
 */
{
  __3 = img_get_symbol(__1);
  if (is_void(__3)) {
    symbol_set, __1, __2;
  } else if (anyof(__2 != __3)) {
    error, "inconsistency in definition of constant \"" + __1 + "\"";
  }
}

// local Y_CHAR, Y_SHORT, Y_INT, Y_LONG, Y_FLOAT, Y_DOUBLE, Y_COMPLEX;
// local Y_STRING, Y_POINTER, Y_STRUCT, Y_RANGE, Y_LVALUE, Y_VOID;
// local Y_FUNCTION, Y_BUILTIN, Y_STRUCTDEF, Y_STREAM, Y_OPAQUE;
local IMG_TYPE_BYTE, IMG_TYPE_SHORT, IMG_TYPE_INT, IMG_TYPE_LONG;
local IMG_TYPE_FLOAT, IMG_TYPE_DOUBLE, IMG_TYPE_COMPLEX;
local IMG_TYPE_RGB, IMG_TYPE_RGBA;
extern img_get_type;
/* DOCUMENT img_get_type(img);

     This function returns the pixel type of image IMG as an integer.  An
     error is raised if IMG is not a valid image (see is_image for the
     definition of a valid image).  The result is one of the
     following global integer constants:

       IMG_TYPE_BYTE    = Y_CHAR       =  0
       IMG_TYPE_SHORT   = Y_SHORT      =  1
       IMG_TYPE_INT     = Y_INT        =  2
       IMG_TYPE_LONG    = Y_LONG       =  3
       IMG_TYPE_FLOAT   = Y_FLOAT      =  4
       IMG_TYPE_DOUBLE  = Y_DOUBLE     =  5
       IMG_TYPE_COMPLEX = Y_COMPLEX    =  6
       IMG_TYPE_RGB     = Y_OPAQUE + 1 = 18
       IMG_TYPE_RGBA    = Y_OPAQUE + 1 = 19

     As a convenience, the Yorick types (Y_CHAR, Y_SHORT, etc.) are also
     defined as global integer constants.

   SEE ALSO is_image, img_define_constant. */

img_define_constant, "Y_CHAR",        0;
img_define_constant, "Y_SHORT",       1;
img_define_constant, "Y_INT",         2;
img_define_constant, "Y_LONG",        3;
img_define_constant, "Y_FLOAT",       4;
img_define_constant, "Y_DOUBLE",      5;
img_define_constant, "Y_COMPLEX",     6;
img_define_constant, "Y_STRING",      7;
img_define_constant, "Y_POINTER",     8;
img_define_constant, "Y_STRUCT",      9;
img_define_constant, "Y_RANGE",      10;
img_define_constant, "Y_LVALUE",     11; /* FIXME: never returned */
img_define_constant, "Y_VOID",       12;
img_define_constant, "Y_FUNCTION",   13;
img_define_constant, "Y_BUILTIN",    14;
img_define_constant, "Y_STRUCTDEF",  15;
img_define_constant, "Y_STREAM",     16;
img_define_constant, "Y_OPAQUE",     17;
img_define_constant, "IMG_TYPE_BYTE",    Y_CHAR;
img_define_constant, "IMG_TYPE_SHORT",   Y_SHORT;
img_define_constant, "IMG_TYPE_INT",     Y_INT;
img_define_constant, "IMG_TYPE_LONG",    Y_LONG;
img_define_constant, "IMG_TYPE_FLOAT",   Y_FLOAT;
img_define_constant, "IMG_TYPE_DOUBLE",  Y_DOUBLE;
img_define_constant, "IMG_TYPE_COMPLEX", Y_COMPLEX;
img_define_constant, "IMG_TYPE_RGB",     Y_OPAQUE + 1;
img_define_constant, "IMG_TYPE_RGBA",    Y_OPAQUE + 2;

extern img_get_channel;
extern img_get_red;
extern img_get_green;
extern img_get_blue;
extern img_get_alpha;
/* DOCUMENT img_get_channel(img, chn);
         or img_get_red(img);
         or img_get_green(img);
         or img_get_blue(img);
         or img_get_alpha(img);
     These functions return a given color channel of image IMG.  CHN is 1 for
     red, 2 for blue, 3 for green, and 4 for alpha (transparency).  Image IMG
     must be of RGB or RGBA type.

   SEE ALSO: img_is_rgb. */

/*---------------------------------------------------------------------------*/
/* LINEAR TRANSFORM */

extern img_extract_rectangle;
/* DOCUMENT roi = img_extract_rectangle(img, xspan, yspan);
 *       or roi = img_extract_rectangle(img, xspan, yspan, a);
 *       or roi = img_extract_rectangle(img, xspan, yspan, a1,a2,a3,a4,a5,a6);
 *
 *   This function extracts a rectangular region of interest (ROI) from image
 *   IMG and returns the result as a 2-D array.  XSPAN (respectively YSPAN) is
 *   the witdh (height) of the destination or a range to specify the start and
 *   end destination abscissae (ordinates) to use.  For instance, XSPAN=WIDTH
 *   is the same as XSPAN=1:WIDTH.
 *
 *   If A is omitted, the image and region of interest have the same
 *   coordinate system.  If argument A is provided, it is a 6-element array of
 *   reals to convert image coordinates (X,Y) into coordinates (XR,YR) in the
 *   frame of the rectangle as follows:
 *
 *     XR = A(1) + A(2)*X + A(3)*Y;
 *     YR = A(4) + A(5)*X + A(6)*Y;
 *
 *   Note that Yorick conventions are followed, hence the origin of the source
 *   coordinates are (1,1).  Unless a range is explicitly specified, this is
 *   also true for the destination.
 *
 *   Alternatively, the array A may be specified by the list of its
 *   coefficients: A1, A2, A3, A4, A5, and A6.
 *
 *   The values in the region of interest are then computed by bi-linear
 *   interpolation of the image.  Note that this is suitable to most linear
 *   transforms except for significant sub-sampling (that is, when the pixel
 *   size in ROI is much larger than that in IMG).  If IMG has integer pixel
 *   type, the result is rounded to the same integer type.
 *
 *   If keyword INVERSE is true, then the coefficients are those of the
 *   inverse coordinates transform; that is, from the destination to the
 *   source image.
 *
 *
 * EXAMPLE OF A ROTATION
 *
 *   We consider a rotation by an angle Q around center at (SRC_X0, SRC_Y0) in
 *   source image and (DST_X0, DST_Y0) in destination image.  The direct
 *   transform writes:
 *
 *     dst_x = dst_x0 + cos(q)*(src_x - src_x0) - sin(q)*(src_y - src_y0)
 *     dst_y = dst_y0 + sin(q)*(src_x - src_x0) + cos(q)*(src_y - src_y0)
 *
 *   hence:
 *
 *     a2 =  (a6 = cos(q));
 *     a3 = -(a5 = sin(q));
 *     a1 =  dst_x0 - a2*src_x0 - a3*src_y0;
 *     a4 =  dst_y0 - a5*src_x0 - a6*src_y0;
 *
 *   while the inverse transform writes:
 *
 *     src_x = src_x0 + cos(q)*(dst_x - dst_x0) + sin(q)*(dst_y - dst_y0)
 *     src_y = src_y0 - sin(q)*(dst_x - dst_x0) + cos(q)*(dst_y - dst_y0)
 *
 * hence:
 *
 *   ainv6 =  (ainv2 = cos(q));
 *   ainv5 = -(ainv3 = sin(q));
 *   ainv1 =  src_x0 - ainv2*dst_x0 - ainv3*dst_y0;
 *   ainv4 =  src_y0 - ainv5*dst_x0 - ainv6*dst_y0;
 *
 *
 * CHANGING THE ORIGIN OF THE COORDINATES
 *
 *   If you want to use a different convention for the origin of the
 *   coordinates, says (SRC_X0, SRC_Y0) and (DST_X0, DST_Y0) for the source
 *   and destination instead of (1, 1) for both, then the direct transform
 *   obeys:
 *
 *   FIXME:
 *
 *     DST_X - DST_X0 + 1 = A1 + A2*(SRC_X - SRC_X0 + 1) + A3*(Y - SRC_X0 + 1)
 *     DST_Y - DST_Y0 + 1 = A4 + A5*(SRC_X - SRC_X0 + 1) + A6*(Y - SRC_X0 + 1)
 *
 *     A1 <== A1 - (1 - DST_X0) + A2*(1 - SRC_X0) + A3*(1 - SRC_X0)
 *     A4 <== A4 - (1 - DST_Y0) + A5*(1 - SRC_X0) + A6*(1 - SRC_X0)
 *
 *
 * SEE ALSO interp, img_rotate.
 */

func img_rotate(img, theta, xcen, ycen, pad=)
/* DOCUMENT img_rotate(img, theta);
         or img_rotate(img, theta, xcen, ycen);

     Rotate image IMG by angle THETA (in degrees counterclockwise) around its
     geometrical center or around point of coordinates (XCEN,YCEN).  The
     result has the same size and type as the input image.

     Keyword PAD can be used to specify the value of pixels ouside the image
     boundaries.  By default, missing values are extrapolated from the edges.

   SEE ALSO:
 */
{
  if (! is_array(img) || ((dims = dimsof(img))(1)) != 2) {
    error, "expecting 2-D array";
  }
  width = dims(2);
  height = dims(3);
  dst_x0 = (1 + width)/2.0;
  dst_y0 = (1 + height)/2.0;
  src_x0 = (is_void(xcen) ? dst_x0 : xcen);
  src_y0 = (is_void(ycen) ? dst_y0 : ycen);
  if (is_scalar(pad)) {
    temp = array(structof(img)(pad), width + 2, height + 2);
    temp( 2 : -1, 2 : -1) = img;
    eq_nocopy, img, temp;
    ++src_x0;
    ++src_y0;
  } else if (! is_void(pad)) {
    error, "value of keyword PAD must be nil or a scalar";
  }
  pi = 3.141592653589793238462643383279503;
  q = (pi/180.0)*theta;
  a2 =  cos(q);
  a3 = -sin(q);
  a5 = -a3;
  a6 =  a2;
  a1 =  dst_x0 - a2*src_x0 - a3*src_y0;
  a4 =  dst_y0 - a5*src_x0 - a6*src_y0;

  return img_extract_rectangle(img, width, height, a1, a2, a3, a4, a5, a6);
}

/*---------------------------------------------------------------------------*/
/* NOISE LEVEL */

extern img_estimate_noise;
/* DOCUMENT img_estimate_noise(img)
         or img_estimate_noise(img, method)
         or img_estimate_noise(img, x0, x1, y0, y1)
         or img_estimate_noise(img, x0, x1, y0, y1, method)

      This function estimates the noise level in image IMG or in sub-image
      IMG(X0:X1,Y0:Y1).

      Note that the sub-image bounds (X0, X1, Y0, and Y1) follow Yorick
      conventions: they are 1-based, and a value less or equal zero
      indicates a bound relative to the end.

   SEE ALSO:
 */

/*---------------------------------------------------------------------------*/
/* SUB-IMAGE COMPARISON */

extern img_cost_l2;
/* DOCUMENT img_cost_l2(a, ax0, ax1, ay0, ay1,
                        b, bx0, bx1, by0, by1,
                        dx, dy, bg, scl);

      This function computes the squared difference between sub-image
      A(AX0:AX1,AY0:AY1) and sub-image B(BX0:BX1, BY0:BY1) with (DX,DY) the
      coordinates of the first pixel in the sub-image B with respect to the
      first pixel in the sub-image A).  For the non-overlapping parts, missing
      pixels (in either sub-image A or sub-image B) are assumed to have the
      background value BG. SCL is a scaling factor, the special value SCL=0.0
      means that the average squared difference is to be returned.

      Note that the sub-image bounds (AX0, AX1, etc.) follow Yorick
      conventions: 1-based, less or equal zero to indicate a bound
      relative to the end.

   SEE ALSO:
 */

/*---------------------------------------------------------------------------*/
/* MORPHO-MATH FUNCTIONS */

extern img_morph_erosion;
extern img_morph_dilation;
extern img_morph_lmin_lmax;
/* DOCUMENT lmin = img_morph_erosion(img, r);
         or lmax = img_morph_dilation(img, r);
         or img_morph_lmin_lmax, img, r, lmin, lmax;

      These functions perform basic morpho-math operations on image IMG with
      circular structuring element of radius R.

      The function img_morph_erosion() returns the result of a morpho-math
      erosion; that is, the local minimima of the pixels of IMG.

      The function img_morph_dilation() returns the result of a morpho-math
      dilation; that is, the local maxima of the pixels of IMG.

      The subroutine img_morph_lmin_lmax stores the result of an erosion and
      a dilation in symbols LMIN and LMAX respectively.  It is almost twice
      as fast as the equivalent:
          lmin = img_morph_erosion(img, r);
          lmax = img_morph_dilation(img, r);

      These functions are faster (by about 20%) but less general than the
      morph_erosion() and morph_dilation() functions provided by Yeti.


   SEE ALSO: morph_erosion, morph_dilation,
             img_morph_closing, img_morph_opening,
             img_morph_white_top_hat, img_morph_black_top_hat;
 */

extern img_morph_closing;
extern img_morph_opening;
/* DOCUMENT img_morph_closing(img, r);
         or img_morph_opening(img, r);

     Perform an image closing/opening of image IMG by a structuring element of
     radius R.  A closing is a dilation followed by an erosion, whereas an
     opening is an erosion followed by a dilation.  See img_morph_dilation for
     the meaning of the arguments.


   SEE ALSO: img_morph_erosion, img_morph_dilation,
             img_morph_white_top_hat, img_morph_black_top_hat. */

local img_morph_white_top_hat;
local img_morph_black_top_hat;
/* DOCUMENT img_morph_white_top_hat(img, r);
         or img_morph_white_top_hat(img, r, s);
         or img_morph_black_top_hat(img, r);
         or img_morph_black_top_hat(img, r, s);

     Perform a summit/valley detection by applying a top-hat filter to image
     IMG.  Argument R defines the radius of the structuring element for the
     feature detection.  Optional argument S gives the radius of the
     structuring element used to apply a smoothing to IMG prior to the top-hat
     filter (S should be smaller than R).  For instance:

       img_morph_white_top_hat(bitmap, 3, 1)

     may be used to detect text or lines in a bimap image.


   SEE ALSO: img_morph_dilation, img_morph_closing, img_morph_enhance. */

func img_morph_white_top_hat(a, r, s)
{
  if (! is_void(s)) a = img_morph_closing(a, s);
  return a - img_morph_opening(a, r);
}

func img_morph_black_top_hat(a, r, s)
{
  if (! is_void(s)) a = img_morph_opening(a, s);
  return img_morph_closing(a, r) - a;
}

func img_morph_enhance(a, r, s)
/* DOCUMENT img_morph_enhance(img, r);
         or img_morph_enhance(img, r, s);

     Perform noise reduction with edge preserving on image IMG.  The result is
     obtained by rescaling the values in IMG in a non-linear way between the
     local minimum and the local maximum.  Argument R defines the structuring
     element for the local neighborhood.  Argument S is a shape factor for the
     rescaling function which is a sigmoid function.  If S is given, it must
     be a non-negative value, the larger is S, the steeper is the rescaling
     function.  The shape factor should be larger than 3 or 5 to have a
     noticeable effect.

     If S is omitted, a step-like rescaling function is chosen: the output
     elements are set to either the local minimum or the local maximum which
     one is the closest.  This corresponds to the limit of very large shape
     factors and implements the "toggle filter" proposed by Kramer & Bruckner
     [1].

     The morph_enhance() may be iterated to achieve deblurring of the input
     image IMG (hundreds of iterations may be required).


  REFERENCES
     [1] H.P. Kramer & J.B. Bruckner, "iterations of a nonlinear
         transformation for enhancement of digital images", Pattern
         Recognition, vol. 7, pp. 53-58, 1975.


   SEE ALSO: img_morph_lmin_lmax.
 */
{
  if (is_void(s)) {
    s = -1.0; /* special value */
  } else if (s < 0.0) {
    error, "S must be non-negative";
  } else {
    /* Pre-compute the range of the sigmoid function to detect early return
       with no change and skip the time consuming morpho-math operations. */
    s = double(s);
    hi = 1.0/(1.0 + exp(-s));
    lo = ((hi == 1.0) ? 0.0 : 1.0/(1.0 + exp(s)));
    if (hi == lo) {
      return a;
    }
  }

  /* Compute the local minima and maxima. */
  local amin, amax;
  img_morph_lmin_lmax, a, r, amin, amax;

  /* Staircase remapping of values. */
  if (s < 0.0) {
    test = ((a - amin) >= (amax - a));
    return merge(amax(where(test)), amin(where(! test)), test);
  }

  /* Remapping of values with a sigmoid. */
  test = ((amin < a)&(a < amax)); // values that need to change
  w = where(test);
  if (! is_array(w)) {
    return a;
  }
  type = structof(a);
  integer = is_integer(a);
  if (numberof(w) != numberof(a)) {
    /* Not all values change, select only those for which there is a
       difference between the local minimum and the local maximum. */
    unchanged = a(where(! test));
    a = a(w);
    amin = amin(w);
    amax = amax(w);
  }

  /* We use the sigmoid function f(t) = 1/(1 + exp(-t)) for the rescaling
     function g(t) = alpha*f(s*t) + beta with S the shape parameter, and
     (ALPHA,BETA) chosen to map the range [-1,1] into [0,1]. */
  alpha = 1.0/(hi - lo);
  beta = alpha*lo;

  /* Linearly map the values in the range [-1,1] -- we already know that the
     local minimum and maximum are different. */
  a = (double(a - amin) - double(amax - a))/double(amax - amin);
  a = alpha/(1.0 + exp(-s*a)) - beta;
  a = a*amax + (1.0 - a)*amin;
  if (! is_void(unchanged)) {
    a = merge(a, unchanged, test);
  }
  if (type != structof(a)) {
    if (integer) return type(round(a));
    return type(a);
  }
  return a;
}

func img_morph_trilevel(a, r, cmin=, white=, black=)
/* DOCUMENT img_morph_trilevel(img, r, atol, rtol);

     The result is an image of same dimensions as IMG and with pixels set to 0
     where IMG is "black", 1 where IMG is "grey", and 2 where is "white".

     black = fraction of "black" levels in a neighborhood
     white = fraction of "white" levels in a neighborhood
     cmin = minimum number of levels
       - a single value to set the absolute minimum number of levels
       - two value to set the absolute minimum and relative number of levels
         CMIN = max(CMIN(1), CMIN(2)*avg(LMAX - LMIN))
       - a function to compute the value of CMIN
       func CMIN(lmin, lmax) {
         return 5 + 0.2*(lmax - lmin);
       }

     black if (lmax - lmin) > cmin and pixel <= lmin + black*(lmax - lmin)
     white if (lmax - lmin) > cmin and pixel >= lmax - white*(lmax - lmin)
     grey otherwise

     where LMIN and LMAX are the local minimum and maximum of the image (in
     the neighborhood of the pixel).

   SEE ALSO: img_morph_lmin_lmax.
 */
{
  /* Compute the local minima and maxima of the image. */
  local amin, amax;
  img_morph_lmin_lmax, a, r, amin, amax;

  result = 2*((a - amin) >= (amax - a));
  if (is_void(atol)) atol = 0.0;
  if (is_void(rtol)) rtol = 0.0;
  if (atol != 0.0 || rtol != 0.0) {
    type = structof(a);
    contrast = amax - amin;
    write, format="max. local constrast = %g\n", double(max(contrast));
    write, format="avg. local constrast = %g\n", double(avg(contrast));
    threshold = atol;
    if (rtol != 0.0) threshold += rtol*avg(contrast);
    if (is_integer(a)) {
      threshold = type(floor(threshold + 0.5));
    } else {
      threshold = type(threshold);
    }
    k = where(contrast < threshold);
    if (is_array(k)) result(k) = 1;
  }
  return result;
}

/*---------------------------------------------------------------------------*/
/* SEGMENTATION */

extern img_watershed;
/* DOCUMENT img_watershed, lab, arr;
         or img_watershed(lab, arr);

     Performs watershed segmentation of 2D array ARR updating the labels in
     LAB.

     The array of labels LAB must be of type long and have the same
     dimensions as ARR.  On entry, LAB must be initialized as follows:

         LAB(i) < 0   to ignore point i;
         LAB(i) = 0   to update the label of point i by the watershing
                      algorithm;
         LAB(i) > 0   to seed the labelling of basins.

     On return, the zero values in LAB are updated as follows:

         LAB(i) = -1  if point i is a crest point;
         LAB(i) > 0   if point i has reached the basin initially labelled
                      with the value LAB(i).

     When called as a function, the contents of LAB is left unchanged and
     an array with the new labels is returned.  When called as a
     subroutine, the operation is performed in-place (i.e. the contents of
     LAB are updated).

   SEE ALSO: img_segmentation_new.
 */

extern img_segmentation_new;
extern img_segmentation_get_number;
extern img_segmentation_get_nrefs;
extern img_segmentation_get_image_width;
extern img_segmentation_get_image_height;
extern img_segmentation_select;
/* DOCUMENT sgm = img_segmentation_new(img, threshold);
         or   n = img_segmentation_get_number(sgm);
         or   n = img_segmentation_get_nrefs(sgm);
         or len = img_segmentation_get_image_width(sgm);
         or len = img_segmentation_get_image_height(sgm);
         or dst = img_segmentation_select(src, idx);

     The function img_segmentation_new() segments image IMG and returns an
     opaque image segmentation object SGM.  THRESHOLD is the maximum absolute
     difference between neighbor pixels to be considered as being part of the
     same region.

     The expression img_segmentation_get_number(sgm) yields the number of
     segments in SGM.

     The expressions img_segmentation_get_image_width(sgm) and
     img_segmentation_get_image_height(sgm) yields the width and height of the
     image from which SGM was built.

     The expression img_segmentation_get_nrefs(sgm) yields the number of
     internal references set on the opaque image segmentation object SGM.

     The expression img_segmentation_select(src, idx) extracts the segments of
     SRC given by the list of indices IDX and returns a new image segmentation
     object.


   SEE ALSO: img_segmentation_get_count, img_segmentation_get_x. */

extern img_segmentation_get_count;
extern img_segmentation_get_xmin;
extern img_segmentation_get_xmax;
extern img_segmentation_get_ymin;
extern img_segmentation_get_ymax;
extern img_segmentation_get_xcen;
extern img_segmentation_get_ycen;
extern img_segmentation_get_width;
extern img_segmentation_get_height;
/* DOCUMENT img_segmentation_get_count(sgm[, i]);
         or img_segmentation_get_xmin(sgm[, i]);
         or img_segmentation_get_xmax(sgm[, i]);
         or img_segmentation_get_ymin(sgm[, i]);
         or img_segmentation_get_ymax(sgm[, i]);
         or img_segmentation_get_xcen(sgm[, i]);
         or img_segmentation_get_ycen(sgm[, i]);
         or img_segmentation_get_width(sgm[, i]);
         or img_segmentation_get_height(sgm[, i]);

     The expression img_segmentation_get_count(sgm) yields the numbers of
     pixels in every segments of SGM; while img_segmentation_get_count(sgm, i)
     yields the number of pixels in i-th segment of SGM.  Index i must be a
     scalar but can be less or equal 0 for indexing from the end.

     The functions img_segmentation_get_xmin(), img_segmentation_get_xmax(),
     img_segmentation_get_ymin(), and img_segmentation_get_ymax() yield the
     (inclusive) coordinates of the bounding box(es) of the segment(s).  They
     have the same semantics as img_segmentation_get_count() for the optional
     index parameter.  Note that coordinates are integers starting at 0.

     The functions img_segmentation_get_xcen(), img_segmentation_get_ycen(),
     img_segmentation_get_width(), and img_segmentation_get_height() yield the
     center and size of the bounding box(es) of the segment(s).  They have the
     same semantics as img_segmentation_get_count() for the optional index
     parameter.  These bounding-box parameters are defined as:

       xcen   = (xmin + xmax)/2.0;
       ycen   = (ymin + ymax)/2.0;
       width  = xmax - xmin + 1;
       height = ymax - ymin + 1;


   SEE ALSO: img_watershed, img_segmentation_new, img_segmentation_get_x. */


local IMG_LINK_EAST, IMG_LINK_WEST, IMG_LINK_NORTH, IMG_LINK_SOUTH;
extern img_segmentation_get_x;
extern img_segmentation_get_y;
extern img_segmentation_get_link;
/* DOCUMENT x = img_segmentation_get_x(sgm, i);
         or y = img_segmentation_get_y(sgm, i);
         or lnk = img_segmentation_get_link(sgm, i);

     These functions query the coordinates or the link flags of the pixels of
     i-th image segment in opaque image segmentation object SGM.  The result
     is a vector of long integers.  Note that the coordinates (X,Y) are
     0-based (unlike Yorick indices).  The link flags are bitwise combination
     of:

       IMG_LINK_EAST  - linked with next pixel on the same row
       IMG_LINK_WEST  - linked with previous pixel on the same row
       IMG_LINK_NORTH - linked with next pixel on the same column
       IMG_LINK_SOUTH - linked with previous pixel on the same column

   SEE ALSO: img_segmentation_new, img_segmentation_get_count. */
IMG_LINK_EAST =   1; /* linked with next pixel on the same row */
IMG_LINK_WEST =   2; /* linked with previous pixel on the same row */
IMG_LINK_NORTH =  4; /* linked with previous pixel on the same column */
IMG_LINK_SOUTH =  8; /* linked with next pixel on the same column */

extern img_chainpool_new;
/* DOCUMENT chn = img_chainpool_new(sgm);

     This function search for chains of segments in image segmentation SGM
     and returns an opaque chain-pool object.  The available keywords are:

       satol=NPIXELS   absolute tolerance for size of characters in pixels
                       (default: satol = 2.0).
       srtol=FACTOR    relative tolerance for size of characters
                       (default: srtol = 0.05).
       drmin=FACTOR    minimun relative distance between characters
                       (default: drmin = 0.4).
       drmax=FACTOR    maximun relative distance between characters
                       (default: drmax = 2.5).
       slope=VALUE     maximum slope for text baseline
                       (default: slope = 0.3).
       aatol=VALUE     absolute alignement tolerance in pixels
                       (default: aatol = 2.0).
       artol=VALUE     relative alignement tolerance
                       (default: artol = 0.05).
       prec=VALUE      precision for distortion estimation in pixels
                       (default: prec = 0.05).
       lmin=NUMBER     minimum number of characters per chain
                       (default: lmin = 3).
       lmax=NUMBER     maximum number of characters per chain
                       (default: lmax = 10).

   SEE ALSO img_segmentation_new.
 */

extern img_chainpool_get_number;
/* DOCUMENT img_chainpool_get_number(chn)
     Get the number of chains in chain-pool object CHN.

   SEE ALSO: img_chainpool_new.
 */

extern img_chainpool_get_image_width;
extern img_chainpool_get_image_height;
/* DOCUMENT img_chainpool_get_image_height(chn)
         or img_chainpool_get_image_height(chn)
     Get the dimensions of the image which wasused to build the chain-pool
     object CHN.

   SEE ALSO: img_chainpool_new.
 */

extern img_chainpool_get_segmentation;
/* DOCUMENT img_chainpool_get_segmentation(chn)
     Retrieve the image segmentation object from which the chains in the
     chain-pool object CHN were extracted.

   SEE ALSO: img_chainpool_new, img_chainpool_get_segments.
 */

extern img_chainpool_get_segments;
/* DOCUMENT img_chainpool_get_segments(chn, i);
     Get the indices of the image segments that belongs to I-th chain in
     chain-pool object CHN.

   SEE ALSO: img_chainpool_new, img_chainpool_get_segmentation,
             img_segmentation_select.
 */

extern img_chainpool_get_vertical_shear;
extern img_chainpool_get_horizontal_shear;
extern img_chainpool_get_xmin;
extern img_chainpool_get_xmax;
extern img_chainpool_get_ymin;
extern img_chainpool_get_ymax;
extern img_chainpool_get_length;
/* DOCUMENT img_chainpool_get_number(chn)
         or img_chainpool_get_vertical_shear(chn [, i]);
         or img_chainpool_get_horizontal_shear(chn [, i]);
         or img_chainpool_get_xmin(chn [, i]);
         or img_chainpool_get_xmax(chn [, i]);
         or img_chainpool_get_ymin(chn [, i]);
         or img_chainpool_get_ymax(chn [, i]);
         or img_chainpool_get_length(chn [, i]);

     Get the number of chains in chain-pool object CHN.

     FIXME:

   SEE ALSO: img_chainpool_new.
 */

/*---------------------------------------------------------------------------*/
/* DETECTION */

extern img_detect_spot;
/* DOCUMENT msk = img_detect_spot(img, c0,c1,c2, t0,t1,t2);
*/
