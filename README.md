# YImage

YImage is a Yorick plugin for image-like operations.


## Image Formats

As far as possible Yorick arrays are interpreted as images: numerical 2D array
are grayscale images, 3-by-*width*-by-*height* `char` arrays are RGB images (of
dimensions *width* and *height*), and 4-by-*width*-by-*height* `char` arrays are
RGBA images (red, green, blue, alpha).


## Features

This version provides:

- basic image management: `is_image`, `img_is_complex`, `img_is_color`,
  `img_is_rgb`, `img_is_rgba`, `img_get_height`, `img_get_width`,
  `img_get_type`, `img_get_red`, `img_get_green`, `img_get_blue`,
  `img_get_alpha` and `img_get_channel`;

- linear geometrical transform of an image: `img_extract_rectangle`,
  `img_rotate`;

- morpho-math operations: `img_morph_black_top_hat`, `img_morph_closing`,
  `img_morph_dilation`, `img_morph_enhance`, `img_morph_erosion`,
  `img_morph_lmin_lmax`, `img_morph_opening`, `img_morph_trilevel` and
  `img_morph_white_top_hat`;

- image segmentation: see `img_watershed` and `img_segmentation_new`;

- functions to manage pools of chains of image segments: see
  `img_chainpool_new`;

- pixelwise comparison of images: see `img_cost_l2`;

- estimate the noise level in an image: see `img_estimate_noise`;

- miscellaneous functions: `img_define_constant`, `img_get_symbol` and
  `img_get_version`;


## Installation

Installation is done in a few steps: (1) unpack the archive, (2) make a build
directory, (3) configure for compilation, (4) build the plug-in and (5) install
it:

    tar jxvf YImage-$VERSION.tar.bz2
    mkdir -p $BUILD
    cd $BUILD
    $SRCDIR/configure
    make clean
    make
    make install

where `$VERSION` yields the version number, `$BUILD` yields the build directory
and `$SRCDIR` yields the top directory where are the plugin sources.  The build
directory can be anywhere, including inside the directory tree of the plugin
source.  The `configure` script has some options which are listed with:

    configure --help


## Installation (the old way)

Unpack the archive, then update the Makefile, build the plug-in and install it
(`$VERSION` yields the version number):

    tar jxvf YImage-$VERSION.tar.bz2
    cd YImage-$VERSION
    yorick -batch make.i
    make clean
    make
    make install


## Quick start

To use the plugin, just load `image.i` (all functions provided by the plugin
are prefixed with `img_` and global constant names are prefixed with `IMG_`):

    yorick
    > include, "image.i";
    > help, img_get_width

Thanks to the *autoload* feature of Yorick, if YImage has been properly
installed, `include, "image.i"` is not necessary.


## Author

* Éric Thiébaut (Centre de Recherche Astrophysique de Lyon, Observatoire de
  Lyon, France) <eric.thiebaut@univ-lyon1.fr>
