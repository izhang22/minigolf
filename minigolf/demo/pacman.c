#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const double VX = 400;
const double VY = 400;
const double ACCEL = 1000;

const size_t N_POINTS_PACMAN = 302;
const double PACMAN_RADIUS = 50;
const rgb_color_t YELLOW = (rgb_color_t) {1, 1, 0};
const double PACMAN_MASS = 1;

const int OFF_X_AXIS = 1;
const int OFF_Y_AXIS = 2;

const size_t N_POINTS_PELLET = 100;
const double PELLET_RADIUS = 5;
const double PELLET_MASS = 0.1;
const size_t INITIAL_N_PELLETS = 10;
const double PELLET_GEN_DT = 1.0;

const int WIDTH = 500;
const int HEIGHT = 250;
const int INITIAL_SIZE_LIST = 8;
const double STAR_DT = 1;

const double ROTATION_RATIO = 5.0 / 3;

/**
 * Takes a scene and inits and adds a pacman shape on the screen. Returns
 * the body of pacman.
 */
body_t *init_and_show_pacman(scene_t *scene) {
  list_t *points = list_init(N_POINTS_PACMAN, free);
  vector_t *v = malloc(sizeof(vector_t));

  // add the first point, an outer corner of pacmans mouth
  *v = (vector_t) {0, PACMAN_RADIUS};
  list_add(points, v);
  vector_t old_v = *v;

  // add the inner corner of pacmans mouth, which is at the center of the circle
  vector_t *origin = malloc(sizeof(vector_t));
  *origin = (vector_t) {0, 0};
  list_add(points, origin);

  // rotate 60º, add the other outer corner of pacmans mouth
  double rotation_angle = M_PI / 3;
  v = malloc(sizeof(vector_t));
  *v = vec_rotate(old_v, rotation_angle);
  list_add(points, v);
  old_v = *v;

  // draw the rest of pacmans body as a circle
  for (size_t i = 2; i < N_POINTS_PACMAN; i++) {
    rotation_angle = ROTATION_RATIO * M_PI / ((int) N_POINTS_PACMAN - 3);
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;
  }

  // make pacman face the right
  polygon_rotate(points, 4 * M_PI / 3, *origin);

  body_t *pacman = body_init(points, PACMAN_MASS, YELLOW);
  scene_add_body(scene, pacman);
  return pacman;
}

/**
 * Randomize where a pellet will be by returning a random translation vector
 */
vector_t get_random_translation() {
  return (vector_t) {(rand() % (2 * WIDTH)) - WIDTH,
    (rand() % (2 * HEIGHT)) - HEIGHT};
}

/**
 * Takes a scene and a number of pellets, and inits and adds that number
 * of pellets to the scene in random locations
 */
void init_and_show_pellets(scene_t *scene, int num_pellets) {
  for (size_t i = 0; i < num_pellets; i++) {
    list_t *points = list_init(N_POINTS_PELLET, free);
    vector_t *v = malloc(sizeof(vector_t));

    // add the first point in the pellet
    *v = (vector_t) {0, PELLET_RADIUS};
    list_add(points, v);
    vector_t old_v = *v;

    // draw the rest of the circle
    for (size_t j = 1; j < N_POINTS_PELLET; j++) {
      double rotation_angle = 2 * M_PI / ((int) N_POINTS_PELLET - 1);
      v = malloc(sizeof(vector_t));
      *v = vec_rotate(old_v, rotation_angle);
      list_add(points, v);
      old_v = *v;
    }

    // translate the pellet to a random point in the window
    polygon_translate(points, get_random_translation());

    body_t *pellet = body_init(points, PELLET_MASS, YELLOW);
    scene_add_body(scene, pellet);
  }
}

/**
 * Returns 0 if pacman is completely in bounds
 *         1 if pacman is outside along the x axis
 *         2 if pacman is outside along the y axis
 */
int should_wrap(body_t *pacman, vector_t bottom_left, vector_t top_right) {
  list_t *points = body_get_shape(pacman);
  size_t length = list_size(points);

  int num_points_off_x_axis = 0;
  int num_points_off_y_axis = 0;

  for (size_t i = 0; i < length; i++) {
    vector_t *point = (vector_t *) list_get(points, i);
    // check if pacman is outside the x axis
    if (point->x - top_right.x >= 0 || bottom_left.x - point->x >= 0) {
      num_points_off_x_axis++;
      if (num_points_off_x_axis >= length - 1) {
        list_free(points);
        return OFF_X_AXIS;
      }
    }
    // check if pacman is outside the x axis
    if (point->y - top_right.y >= 0 || bottom_left.y - point->y >= 0) {
      num_points_off_y_axis++;
      if (num_points_off_y_axis >= length - 1) {
        list_free(points);
        return OFF_Y_AXIS;
      }
    }
  }
  // if pacman is inside both bounds, return 0
  list_free(points);
  return 0;
}

/**
 * Returns 0 if given pellet is not within the radius of pacman's centroid
 *         1 if pellet is within the radius of pacman's centroid
 */
int is_pellet_in_pacman(body_t *pacman, body_t *pellet) {
  vector_t pacman_centroid = body_get_centroid(pacman);
  vector_t pellet_centroid = body_get_centroid(pellet);
  vector_t difference = vec_subtract(pacman_centroid, pellet_centroid);
  double distance = sqrt(vec_dot(difference, difference));
  return (distance < PACMAN_RADIUS);
}

/**
 * Deletes any pellets within the radius of pacman's centroid
 */
void eat_pellets(scene_t *scene, body_t *pacman) {
  size_t i = 1;
  while (i < scene_bodies(scene)) {
    body_t *pellet = scene_get_body(scene, i);
    if (is_pellet_in_pacman(pacman, pellet)) {
      scene_remove_body(scene, i);
      body_free(pellet);
    }
    else {
      i++;
    }
  }
}

/**
 * Dictates the direction and speed of pacman's movement on arrow presses
 */
void on_key(scene_t *scene, char key, key_event_type_t type, double held_time, body_t *pacman) {
  double accel_const = 0.0;
  if (type == KEY_PRESSED) {
    accel_const = held_time * ACCEL;
  }
  if (type == KEY_PRESSED || type == KEY_RELEASED) {
    switch (key) {
      case UP_ARROW:
        body_set_rotation(pacman, M_PI / 2);
        body_set_velocity(pacman, (vector_t) {0, VY + accel_const});
        break;
      case LEFT_ARROW:
        body_set_rotation(pacman, M_PI);
        body_set_velocity(pacman, (vector_t) {-VX - accel_const, 0});
        break;
      case DOWN_ARROW:
        body_set_rotation(pacman, 3 * M_PI / 2);
        body_set_velocity(pacman, (vector_t) {0, -VY - accel_const});
        break;
      case RIGHT_ARROW:
        body_set_rotation(pacman, 0);
        body_set_velocity(pacman, (vector_t) {VX + accel_const, 0});
        break;
    }
  }
}

void run_sim(scene_t *scene, vector_t bottom_left, vector_t top_right) {
  double time_passed = 0;
  body_t *pacman = init_and_show_pacman(scene);
  init_and_show_pellets(scene, INITIAL_N_PELLETS);
  sdl_on_key(on_key);

  while(!sdl_is_done(pacman, scene, NULL)) {
    // if pacman is off the screen, wrap him around
    int wrap = should_wrap(pacman, bottom_left, top_right);
    vector_t centroid = body_get_centroid(pacman);
    if (wrap == OFF_X_AXIS) {
      body_set_centroid(pacman, (vector_t) {-centroid.x, centroid.y});
    }
    if (wrap == OFF_Y_AXIS) {
      body_set_centroid(pacman, (vector_t) {centroid.x, -centroid.y});
    }

    // add a new pellet about every PELLET_GEN_DT seconds
    double time = time_since_last_tick();
    time_passed += time;
    if (time_passed > PELLET_GEN_DT) {
      init_and_show_pellets(scene, 1);
      time_passed = 0;
    }

    // check all the pellets for eating
    eat_pellets(scene, pacman);

    scene_tick(scene, time);
    sdl_render_scene(scene);
  }
}

int main() {
  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);

  scene_t *scene = scene_init();
  run_sim(scene, bottom_left, top_right);

  scene_free(scene);
  return 0;
}
