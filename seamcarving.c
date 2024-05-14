//Main file

#include <stdio.h>
#include <stdlib.h>
#include "c_img.h"
#include "c_img.c"
#include <math.h>
#include "seamcarving.h"

double min(double a, double b, double c) {
  double min = a;
  min = (b < min) ? b : min;
  min = (c < min) ? c : min;
  return min;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad) {
  create_img(grad, (*im).height, (*im).width);
  int r_left, g_left, b_left, r_right, g_right, b_right, r_up, g_up, b_up, r_down, g_down, b_down, dx, dy;
  for (int h = 0; h < (*im).height; h++) {
    for (int w = 0; w < (*im).width; w++) {
      r_up = h == 0 ? get_pixel(im, (int) (*im).height - 1, w, 0) : get_pixel(im, h - 1, w, 0);
      g_up = h == 0 ? get_pixel(im, (int) (*im).height - 1, w, 1) : get_pixel(im, h - 1, w, 1);
      b_up = h == 0 ? get_pixel(im, (int) (*im).height - 1, w, 2) : get_pixel(im, h - 1, w, 2);
      r_down = h == (int) (*im).height - 1 ? get_pixel(im, 0, w, 0) : get_pixel(im, h + 1, w, 0);
      g_down = h == (int) (*im).height - 1 ? get_pixel(im, 0, w, 1) : get_pixel(im, h + 1, w, 1);
      b_down = h == (int) (*im).height - 1 ? get_pixel(im, 0, w, 2) : get_pixel(im, h + 1, w, 2);
      r_left = w == 0 ? get_pixel(im, h, (int) (*im).width - 1, 0) : get_pixel(im, h, w - 1, 0);
      g_left = w == 0 ? get_pixel(im, h, (int) (*im).width - 1, 1) : get_pixel(im, h, w - 1, 1);
      b_left = w == 0 ? get_pixel(im, h, (int) (*im).width - 1, 2) : get_pixel(im, h, w - 1, 2);
      r_right = w == (int) (*im).width - 1 ? get_pixel(im, h, 0, 0) : get_pixel(im, h, w + 1, 0);
      g_right = w == (int) (*im).width - 1 ? get_pixel(im, h, 0, 1) : get_pixel(im, h, w + 1, 1);
      b_right = w == (int) (*im).width - 1 ? get_pixel(im, h, 0, 2) : get_pixel(im, h, w + 1, 2);
      dx = (r_right - r_left) * (r_right - r_left)
           + (g_right - g_left) * (g_right - g_left) + (b_right - b_left) * (b_right - b_left);
      dy = (r_down - r_up) * (r_down - r_up) +
           (g_down - g_up) * (g_down - g_up) + (b_down - b_up) * (b_down - b_up);
      set_pixel(*grad, h, w, (int) (sqrt(dx + dy) / 10), (int) (sqrt(dx + dy) / 10),
                (int) (sqrt(dx + dy) / 10));
    }
  }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr) {
  *best_arr = (double *) malloc(sizeof(double) * (*grad).height * (*grad).width);
  for (int h = 0; h < (*grad).height; h++) {
    for (int w = 0; w < (*grad).width; w++) {
      if (h == 0) (*best_arr)[h * (*grad).width + w] = get_pixel(grad, h, w, 0);
      else if (w == 0)
        (*best_arr)[h * (*grad).width + w] =
                min((*best_arr)[(h - 1) * (*grad).width + w], (*best_arr)[(h - 1) * (*grad).width + w + 1],
                    269) + get_pixel(grad, h, w, 0);
      else if (w == (int) (*grad).width - 1)
        (*best_arr)[h * (*grad).width + w] =
                min((*best_arr)[(h - 1) * (*grad).width + w - 1], (*best_arr)[(h - 1) * (*grad).width + w],
                    269) + get_pixel(grad, h, w, 0);
      else
        (*best_arr)[h * (*grad).width + w] =
                min((*best_arr)[(h - 1) * (*grad).width + w - 1], (*best_arr)[(h - 1) * (*grad).width + w],
                    (*best_arr)[(h - 1) * (*grad).width + w + 1]) + get_pixel(grad, h, w, 0);
    }
  }
}

void recover_path(double *best, int height, int width, int **path) {
  *path = (int *) malloc(height * sizeof(int));
  int min_col;
  for (int h = height - 1; h >= 0; h--) {
    min_col = 0;
    if (h == height - 1) {
      for (int w = 0; w < width; w++) {
        min_col = best[h * width + min_col] > best[h * width + w] ? w : min_col;
      }
    } else if ((*path)[h + 1] == 0)
      min_col = best[h * width + (*path)[h + 1]] >
                best[(h) * width + (*path)[h + 1] + 1] ? (*path)[h + 1] + 1 : (*path)[
                        h + 1];
    else if ((*path)[h + 1] == width - 1)
      min_col = best[h * width + (*path)[h + 1] - 1] >
                best[(h) * width + (*path)[h + 1]] ? (*path)[h + 1] :
                (*path)[h + 1] - 1;
    else {
      min_col = best[h * width + (*path)[h + 1] - 1] > best[(h) * width + (*path)[h + 1]] ? (*path)[h + 1] :
                (*path)[h + 1] - 1;
      min_col = best[h * width + min_col] > best[(h) * width + (*path)[h + 1] + 1] ? (*path)[h + 1] + 1 : min_col;
    }
    (*path)[h] = min_col;
  }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
  *dest = (struct rgb_img *) malloc(sizeof(struct rgb_img));
  (*dest)->height = (*src).height;
  (*dest)->width = (*src).width - 1;
  (*dest)->raster = (uint8_t *) malloc((*dest)->height * (*dest)->width * 3 * sizeof(uint8_t));
  for (int h = 0; h < src->height; h++) {
    for (int w = 0; w < src->width; w++) {
      if (w < path[h])
        set_pixel(*dest, h, w, get_pixel(src, h, w, 0),
                  get_pixel(src, h, w, 1), get_pixel(src, h, w, 2));
      else if (w > path[h])
        set_pixel(*dest, h, w - 1, get_pixel(src, h, w, 0),
                  get_pixel(src, h, w, 1), get_pixel(src, h, w, 2));
    }
  }
}

int main {
  struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 5; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}#if !defined(CIMG)
#define CIMG

#include <stdlib.h>
#include <stdint.h>


struct rgb_img{
    uint8_t *raster;
    size_t height;
    size_t width;
};

void create_img(struct rgb_img **im, size_t height, size_t width);
void read_in_img(struct rgb_img **im, char *filename);
void write_img(struct rgb_img *im, char *filename);
uint8_t get_pixel(struct rgb_img *im, int y, int x, int col);
void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b);
void destroy_image(struct rgb_img *im);
void print_grad(struct rgb_img *grad);


#endif
