#include "vector.h"
#include "scene.h"
#include "body.h"
#include "list.h"
#include "sdl_wrapper.h"
#include "polygon.h"
#include "color.h"
#include "forces.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

const int RADIUS = 10;
const double K = 2;
const size_t N_POINTS = 360;
const double CIRCLE_MASS = 1;
const vector_t FIRST_CIRCLE_VELOCITY = (vector_t) {0, 20};
const rgb_color_t WHITE = (rgb_color_t) {1, 1, 1};
const double MAX_GAMMA = 1.0;
const int MIN_N_POINTS = 3;

const int WIDTH = 500;
const int HEIGHT = 250;

const int NUM_CIRCLES = WIDTH / RADIUS;

/**
 * Gets random integer between min and min + width
 */
int get_random_num(int min, int width) {
  return (rand() % width + min);
}

list_t *init_circle() {
  list_t *points = list_init(N_POINTS, free);
  vector_t *v = malloc(sizeof(vector_t));
  // add the first point in the circle
  *v = (vector_t) {0, RADIUS};
  list_add(points, v);
  vector_t old_v = *v;
  // draw the rest of the circle
  for (size_t j = 1; j < N_POINTS; j++) {
    double rotation_angle = 2 * M_PI / ((int) N_POINTS - 1);
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;
  }
  return points;
}

/**
 * Takes a scene, and inits and adds circles to the scene in a line across
 * the screen
 */
void init_and_show_circles(scene_t *scene) {
  // create the white points along x axis of the screen with infinite mass
  vector_t centroid = (vector_t) {-WIDTH + RADIUS, 0};
  for (size_t i = 0; i < NUM_CIRCLES; i++) {
    list_t *points = init_circle();
    body_t *body = body_init(points, INFINITY, WHITE);
    body_set_centroid(body, centroid);
    scene_add_body(scene, body);
    centroid = (vector_t) {centroid.x + 2 * RADIUS, 0};
  }

  // create the color spheres
  centroid = (vector_t) {-WIDTH + RADIUS, HEIGHT};
  for (size_t i = 0; i < NUM_CIRCLES; i++) {
    list_t *points = init_circle();
    body_t *circle = body_init(points, CIRCLE_MASS, get_random_color());
    body_set_centroid(circle, centroid);
    scene_add_body(scene, circle);
    centroid = (vector_t) {centroid.x + 2 * RADIUS,
      (double) HEIGHT * cos(1.0 * ((int) i + 1) / NUM_CIRCLES * M_PI)};
  }
}

/**
 * Adds spring between circle and invisible column in middle.
 */
void create_springs_between_circles(scene_t *scene) {
  for (size_t i = NUM_CIRCLES; i < 2 * NUM_CIRCLES; i++) {
    body_t *body1 = scene_get_body(scene, i);
    body_t *body2 = scene_get_body(scene, i - NUM_CIRCLES);
    create_spring(scene, K, body1, body2);
  }
}

/**
 * adds drag to each circle, starting from gamma = 0 on the left and gamma =
 * MAX_GAMMA on the right
 */
void create_drag_on_circles(scene_t *scene) {
  double gamma = 0.0;
  for (size_t i = NUM_CIRCLES; i < 2 * NUM_CIRCLES; i++) {
    body_t *body = scene_get_body(scene, i);
    create_drag(scene, gamma, body);

    gamma += MAX_GAMMA / NUM_CIRCLES;
  }
}

void run_sim(scene_t *scene) {
  init_and_show_circles(scene);
  create_springs_between_circles(scene);
  create_drag_on_circles(scene);

  time_since_last_tick();
  while(!sdl_is_done(NULL, scene, NULL)) {
    scene_tick(scene, time_since_last_tick());
    sdl_render_scene(scene);
  }
}


int main() {
  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);

  scene_t *scene = scene_init();
  run_sim(scene);

  scene_free(scene);
  return 0;
}
