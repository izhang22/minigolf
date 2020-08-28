#include "polygon.h"
#include "vector.h"
#include "sdl_wrapper.h"

#include <stdlib.h>

const double DT = 0.00001;
const double ROTATION = 0.003;
const double VX = 0.3;
const double VY = 0.3;
const size_t N_POINTS = 5;
const double STAR_RADIUS = 20;
const int WIDTH = 500;
const int HEIGHT = 250;
const int OFF_X_AXIS = 1;
const int OFF_Y_AXIS = 2;
const int OFF_BOTH_AXES = 3;

/**
 * Returns 0 if star completely in bounds
 *         1 if star outside along the x axis
 *         2 if star outside along the y axis
 *         3 if star outside along both axes.
 */
int is_off_axis(list_t *points, vector_t bottom_left, vector_t top_right,
  double vx, double vy) {
  size_t length = list_size(points);

  int is_off_x_axis = 0;
  int is_off_y_axis = 0;

  for (size_t i = 0; i < length; i++) {
    vector_t *point = (vector_t *) list_get(points, i);
    if (point->x + vx - top_right.x >= 0 || bottom_left.x - (point->x + vx)
      >= 0) {
      is_off_x_axis = OFF_X_AXIS;
    }
    if (point->y + vy - top_right.y >= 0 || bottom_left.y - (point->y + vy)
      >= 0) {
      is_off_y_axis = OFF_Y_AXIS;
    }
  }
  return is_off_x_axis + is_off_y_axis;
}

void run_sim(list_t *list, vector_t bottom_left, vector_t top_right) {
  double vx = VX;
  double vy = VY;

  double time_passed = 0;
  while(!sdl_is_done(NULL, NULL, NULL)) {
    time_passed += time_since_last_tick();
    if (time_passed > DT) {
      time_passed = 0;
      vector_t center = polygon_centroid(list);
      polygon_rotate(list, ROTATION, center);

      int is_off = is_off_axis(list, bottom_left, top_right, vx, vy);

      // go in the opposite direction on the axis specified by the output of
      // the is_off_axis function
      if (is_off == OFF_X_AXIS || is_off == OFF_BOTH_AXES) {
        vx *= -1;
      }
      if (is_off == OFF_Y_AXIS || is_off == OFF_BOTH_AXES) {
        vy *= -1;
      }
      polygon_translate(list, (vector_t) {vx, vy});
    }
    sdl_clear();
    sdl_draw_polygon(list, (rgb_color_t) {1, 0, 1});
    sdl_show();
  }
}

int main() {
  sdl_clear();

  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);

  list_t *list = star_init(STAR_RADIUS, N_POINTS);
  run_sim(list, bottom_left, top_right);

  list_free(list);

  return 0;
}
