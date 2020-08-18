#include "PicProcess.h"

#include "Picture.h"

#define MAX_INTENSITY 255

static void invert_pixel(struct picture *pic, int x, int y);
static void greyscale_pixel(struct picture *pic, int x, int y);
static void rotate_picture_90(struct picture *pic);
static void rotate_picture_180(struct picture *pic);
static void rotate_pixel_180(struct picture *pic, int x, int y);
static void rotate_picture_270(struct picture *pic);
static bool is_vertical(char plane);
static bool is_horizontal(char plane);
static void flip_picture_h(struct picture *pic);
static void flip_picture_v(struct picture *pic);
static struct pixel get_blurred_pixel_of_pic(struct picture *pic, int x, int y);

static void add_pixel_to_pixel(struct pixel *accumulator, struct pixel *pix);
static void invert_colour(struct pixel *pix);
static void greyify_pixel_colour(struct pixel *pix);
static bool is_within_borders(struct picture *pic, int x, int y);
static struct picture *create_new_picture(int width, int height);
static void set_as_new_pic(struct picture *old, struct picture *new);
static struct pixel get_blank_pixel();

void invert_picture(struct picture *pic) {
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < pic->height; j++) {
      invert_pixel(pic, i, j);
    }
  }
}

static void invert_pixel(struct picture *pic, int x, int y) {
  struct pixel pix = get_pixel(pic, x, y);
  invert_colour(&pix);
  set_pixel(pic, x, y, &pix);
}

static void invert_colour(struct pixel *pix) {
  pix->red = MAX_INTENSITY - pix->red;
  pix->green = MAX_INTENSITY - pix->green;
  pix->blue = MAX_INTENSITY - pix->blue;
}

void grayscale_picture(struct picture *pic) {
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < pic->height; j++) {
      greyscale_pixel(pic, i, j);
    }
  }
}

static void greyscale_pixel(struct picture *pic, int x, int y) {
  struct pixel pix = get_pixel(pic, x, y);
  greyify_pixel_colour(&pix);
  set_pixel(pic, x, y, &pix);
}

static void greyify_pixel_colour(struct pixel *pix) {
  int avg = (pix->red + pix->green + pix->blue) / 3;

  pix->red = avg;
  pix->green = avg;
  pix->blue = avg;
}

void rotate_picture(struct picture *pic, int angle) {
  switch (angle) {
    case 90:rotate_picture_90(pic);
      break;
    case 180:rotate_picture_180(pic);
      break;
    case 270:rotate_picture_270(pic);
      break;
    default:printf("ERROR: Rotation angle not implemented yet!\n");
      exit(-1);
  }
}

static void rotate_picture_90(struct picture *pic) {
  struct picture *out = create_new_picture(pic->height, pic->width);
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < pic->height; j++) {
      struct pixel pix = get_pixel(pic, i, j);
      set_pixel(out, out->width - j - 1, i, &pix);
    }
  }
  set_as_new_pic(pic, out);
}

static void rotate_picture_180(struct picture *pic) {
  for (int i = 0; i < (pic->width) / 2; i++) {
    for (int j = 0; j < pic->height; j++) {
      rotate_pixel_180(pic, i, j);
    }
  }
}

static void rotate_pixel_180(struct picture *pic, int x, int y) {
  struct pixel pix1 = get_pixel(pic, x, y);
  struct pixel pix2 = get_pixel(pic,
                                pic->width - x - 1,
                                pic->height - y - 1);
  set_pixel(pic, pic->width - x - 1, pic->height - y - 1, &pix1);
  set_pixel(pic, x, y, &pix2);
}

static void rotate_picture_270(struct picture *pic) {
  struct picture *out = create_new_picture(pic->height, pic->width);
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < pic->height; j++) {
      struct pixel pix = get_pixel(pic, i, j);
      set_pixel(out, j, out->height - i - 1, &pix);
    }
  }
  set_as_new_pic(pic, out);
}

void flip_picture(struct picture *pic, char plane) {
  if (is_horizontal(plane)) {
    flip_picture_h(pic);
  } else if (is_vertical(plane)) {
    flip_picture_v(pic);
  } else {
    printf("ERROR: this plane does not exist. Aborting...\n");
    exit(-1);
  }
}

static bool is_horizontal(char plane) {
  return plane == 'h' || plane == 'H';
}

static bool is_vertical(char plane) {
  return plane == 'v' || plane == 'V';
}

static void flip_picture_h(struct picture *pic) {
  for (int i = 0; i < (pic->width) / 2; i++) {
    for (int j = 0; j < pic->height; j++) {
      struct pixel pix_l = get_pixel(pic, i, j);
      struct pixel pix_r = get_pixel(pic, (pic->width - i - 1), j);

      set_pixel(pic, i, j, &pix_r);
      set_pixel(pic, (pic->width - i - 1), j, &pix_l);
    }
  }
}

static void flip_picture_v(struct picture *pic) {
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < (pic->height) / 2; j++) {
      struct pixel pix_u = get_pixel(pic, i, j);
      struct pixel pix_d = get_pixel(pic, i, (pic->height - j - 1));

      set_pixel(pic, i, j, &pix_d);
      set_pixel(pic, i, (pic->height - j - 1), &pix_u);
    }
  }
}

void blur_picture(struct picture *pic) {
  struct picture *out = create_new_picture(pic->width, pic->height);
  for (int i = 0; i < pic->width; i++) {
    for (int j = 0; j < pic->height; j++) {
      struct pixel blurred = get_blurred_pixel_of_pic(pic, i, j);
      set_pixel(out, i, j, &blurred);
    }
  }
  set_as_new_pic(pic, out);
}

static struct pixel get_blurred_pixel_of_pic(struct picture *pic, int x, int y) {
  if (is_within_borders(pic, x, y)) {
    struct pixel avg = get_blank_pixel();
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        struct pixel pix = get_pixel(pic, x + i, y + j);

        add_pixel_to_pixel(&avg, &pix);
      }
    }
    avg.red /= 9;
    avg.green /= 9;
    avg.blue /= 9;
    return avg;
  } else {
    return get_pixel(pic, x, y);
  }
}

static bool is_within_borders(struct picture *pic, int x, int y) {
  return x > 0 && y > 0 && x < pic->width - 1 && y < pic->height - 1;
}

static struct pixel get_blank_pixel() {
  struct pixel avg;
  avg.red = 0;
  avg.green = 0;
  avg.blue = 0;
  return avg;
}

static void add_pixel_to_pixel(struct pixel *accumulator, struct pixel *pix) {
  accumulator->red += pix->red;
  accumulator->green += pix->green;
  accumulator->blue += pix->blue;
}

static struct picture *create_new_picture(int width, int height) {
  struct picture *out = (struct picture *) malloc(sizeof(struct picture));
  init_picture_from_size(out, width, height);
  return out;
}

static void set_as_new_pic(struct picture *old, struct picture *new) {
  free_image(old->img);

  old->img = new->img;
  old->width = new->height;
  old->height = new->width;
  free(new);
}
