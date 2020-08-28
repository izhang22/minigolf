#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <stdio.h>

const double DT = 0.00001;
const double ROTATION = 0.003;
const double VX = 0.5;
const double VY = -0.4;
const size_t N_POINTS = 2;
const double STAR_RADIUS = 20;
const int WIDTH = 500;
const int HEIGHT = 250;
const int INITIAL_SIZE_LIST = 8;
const double STAR_DT = 1;
const double GRAVITY = -0.01;
const int ELASTICITY_WIDTH = 10;
const double ELASTICITY_PRECISION = 100.0;
const double MIN_ELASTICITY = 0.9;

/**
 * Returns 0 if star in bounds
 *         1 if star is completely off the screen to the right
 */
int is_off_screen(list_t *points, vector_t top_right) {
  size_t length = list_size(points);

  for (size_t i = 0; i < length; i++) {
    vector_t *point = (vector_t *) list_get(points, i);
    if (point->x - top_right.x <= 0) {
      return 0;
    }
  }
  return 1;
}

/**
 * Returns 0 if star completely in bounds
 *         1 if star outside along the y axis, and needs to bounce
 */
int should_bounce(list_t *points, vector_t bottom_left, double vy) {
  size_t length = list_size(points);

  for (size_t i = 0; i < length; i++) {
    vector_t *point = (vector_t *) list_get(points, i);
    if (bottom_left.y - (point->y + vy) >= 0) {
      return 1;
    }
  }
  return 0;
}

polygon_t *make_new_star(size_t n_points) {
  list_t *vertices = star_init(STAR_RADIUS, n_points);
  polygon_translate(vertices, (vector_t)
    {-WIDTH + STAR_RADIUS, HEIGHT - STAR_RADIUS});
  polygon_t *polygon = malloc(sizeof(polygon_t));
  polygon->vertices = vertices;
  polygon->color = get_random_color();

  // get random elasticity between 0.90 and 0.99
  polygon->elasticity = (rand() % ELASTICITY_WIDTH) / ELASTICITY_PRECISION + MIN_ELASTICITY;
  polygon->velocity = (vector_t) {VX, VY};
  return polygon;
}

void delete_star(list_t *list, size_t i) {
  polygon_t *p = (polygon_t *) list_remove(list, i);
  list_free(p->vertices);
  free(p);
}

void move_star(polygon_t *polygon, vector_t bottom_left) {
  list_t *vertices = polygon->vertices;
  vector_t center = polygon_centroid(vertices);
  polygon_rotate(vertices, ROTATION, center);

  // if necessary, bounce the star from the bottom of the screen
  if (should_bounce(vertices, bottom_left, polygon->velocity.y)) {
    polygon->velocity.y *= -1 * polygon->elasticity;
    polygon->velocity.x *= polygon->elasticity;
  }

  // accelerate the downwards velocity by GRAVITY amount
  polygon->velocity.y += GRAVITY;
  polygon_translate(vertices, polygon->velocity);
}

void draw_polygons(list_t * list) {
  sdl_clear();
  size_t length = list_size(list);
  for (size_t i = 0; i < length; i++) {
    polygon_t *polygon = (polygon_t*) list_get(list, i);
    sdl_draw_polygon(polygon->vertices, polygon->color);
  }
  sdl_show();
}

void run_sim(list_t *list, vector_t bottom_left, vector_t top_right) {
  double time_since_tick = 0;
  double time_passed = 0;
  size_t num_star_points = N_POINTS;

  while(!sdl_is_done(NULL, NULL, NULL)) {
    double time = time_since_last_tick();
    time_passed += time;
    time_since_tick += time;

    // add a new star about every STAR_DT seconds
    if (time_passed > STAR_DT) {
      polygon_t *star = make_new_star(num_star_points);
      list_add(list, star);
      time_passed = 0;
      num_star_points += 1;
    }

    // rotate, translate, and then draw each star
    if (time_since_tick > DT) {
      time_since_tick = 0;

      size_t i = 0;
      while (i < list_size(list)) {
        polygon_t *polygon = (polygon_t *) list_get(list, i);
        list_t *vertices = polygon->vertices;

        // if necessary, delete the star from the screen when it goes off screen
        if (is_off_screen(vertices, top_right)) {
          delete_star(list, i);
        }
        else {
          move_star(polygon, bottom_left);
          i++;
        }
      }
    }
    draw_polygons(list);
  }
}

void free_vertices(void *item) {
  polygon_t *polygon = (polygon_t *) item;
  list_t *vertices_list = polygon->vertices;
  list_free(vertices_list);
  free(polygon);
}

int main() {
  sdl_clear();

  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);

  list_t *list = list_init(INITIAL_SIZE_LIST, free_vertices);
  run_sim(list, bottom_left, top_right);

  list_free(list);
  return 0;
}
