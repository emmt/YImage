- [x] Modify `c_pseudo_template` to have RGB and RGBA types.

- [x] In `image_t` remove `yor_type` member and rename `img_type` as type.

- [ ] Use a configuration script/program to guess the data types.

- [ ] Write the doxygen documentation of all functions.

- [ ] Provide a Makefile to build the library separately.

- [ ] Move/implement all functions from "img.i" in "image.i" to
      have only one file to include.

- [ ] Implement other types of interpolations (bi-cubic, etc.).

- [ ] Write faster code for interpolation with a linear coordinate
      transform (e.g. using simple shears).

- [ ] Implement non-linear coordinates transform (at least polynomial of order 2
      and 3).  Possibly, arbitrary transforms:

        interpolate(z, x, y)

      where Z is WxH source image, while X and Y are WPxHP arrays of
      coordinates in the source image of the pixels of the WPxHP destination
      image.  Consider the special case of separable X and Y transforms (i.e. X
      is WPx1 and Y is 1xHP).

- [ ] Speed-up morpho-math operations.

- [ ] Make the functions of itempool.c, itemstack.c and memstack.c private
      (hidden) functions (see "GCC visibility").

- [ ] Make a special version of the library for the Yorick plug-in (avoiding
      all data types not supported by Yorick).
