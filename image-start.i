/* Autoload for YImage plugin. */
autoload, "image.i", img_get_version, img_get_symbol, img_define_constant,
  is_image, img_is_complex, img_is_color, img_is_rgb, img_is_rgba,
  img_get_width, img_get_height, img_get_type, img_get_channel, img_get_red,
  img_get_green, img_get_blue, img_get_alpha,
  img_extract_rectangle, img_rotate, img_detect_spot,
  img_estimate_noise, img_cost_l2;
autoload, "image.i", img_morph_erosion, img_morph_dilation,
  img_morph_lmin_lmax, img_morph_closing, img_morph_opening,
  img_morph_white_top_hat, img_morph_black_top_hat,
  img_morph_enhance, img_morph_trilevel;
autoload, "image.i", img_segmentation_new, img_segmentation_get_number,
  img_segmentation_get_nrefs, img_segmentation_get_image_width,
  img_segmentation_get_image_height, img_segmentation_select,
  img_segmentation_get_count, img_segmentation_get_xmin,
  img_segmentation_get_xmax, img_segmentation_get_ymin,
  img_segmentation_get_ymax, img_segmentation_get_xcen,
  img_segmentation_get_ycen, img_segmentation_get_width,
  img_segmentation_get_height, img_segmentation_get_x,
  img_segmentation_get_y, img_segmentation_get_link;
autoload, "image.i", img_chainpool_new, img_chainpool_get_number,
  img_chainpool_get_image_width, img_chainpool_get_image_height,
  img_chainpool_get_segmentation, img_chainpool_get_segments,
  img_chainpool_get_vertical_shear, img_chainpool_get_horizontal_shear,
  img_chainpool_get_xmin, img_chainpool_get_xmax, img_chainpool_get_ymin,
  img_chainpool_get_ymax, img_chainpool_get_length;
/*
 * Autoload not yet supported for constants.
 *
 * autoload, "image.i", IMG_TYPE_BYTE, IMG_TYPE_SHORT, IMG_TYPE_INT,
 *   IMG_TYPE_LONG, IMG_TYPE_FLOAT, IMG_TYPE_DOUBLE, IMG_TYPE_COMPLEX,
 *   IMG_TYPE_RGB, IMG_TYPE_RGBA,
 *   IMG_LINK_EAST, IMG_LINK_WEST, IMG_LINK_NORTH, IMG_LINK_SOUTH;
 */
