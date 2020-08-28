#include "list.h"
#include "vector.h"
#include "collision.h"
#include "minigolf_utils.h"
#include "forces.h"
#include "polygon.h"
#include <assert.h>
#include <stdlib.h>

const rgb_color_t GRASS_COLOR = (rgb_color_t) {0, 0.5, 0};
const rgb_color_t HOLE_COLOR = (rgb_color_t) {0, 0, 0};
const rgb_color_t WALL_COLOR = (rgb_color_t) {0.1, 0.8, 0.1};
const rgb_color_t BALL_COLOR = (rgb_color_t) {1, 1, 1};
const rgb_color_t OBS_COLOR =  (rgb_color_t) {0.9, 0.9, 0.9};

const int WALL_WIDTH = 5;
const int HOLE_RADIUS = 7;
const int BALL_RADIUS = 5;

const double BALL_MASS = 5;

const double BALL_ELASTICITY = 0.5;
const double GRASS_FRICTION = 10;

const int NUM_POINTS = 360;

list_t *make_circle(int radius, vector_t center) {
  list_t *points = list_init(NUM_POINTS, free);
  vector_t *v = malloc(sizeof(vector_t));
  // add the first point in the circle
  *v = (vector_t) {0, radius};
  list_add(points, v);
  vector_t old_v = *v;
  // draw the rest of the circle
  for (size_t j = 1; j < NUM_POINTS; j++) {
    double rotation_angle = 2 * M_PI / ((int) NUM_POINTS);
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;
  }
  polygon_translate(points, center);
  return points;
}

vector_t make_wall_unit_vector(vector_t v1, vector_t v2) {
  vector_t direction = vec_subtract(v2, v1);
  double magnitude = vec_distance(v1, v2);
  vector_t unit_vector = vec_multiply(1.0 / magnitude, direction);
  return unit_vector;
}

vector_t get_vector_perpendicular(vector_t v) {
  vector_t perpendicular = (vector_t) {-v.y / sqrt(v.x * v.x + v.y * v.y),
    v.x / sqrt(v.x * v.x + v.y * v.y)};
  return perpendicular;
}
list_t *make_rectangle_with_width(vector_t point1, vector_t point2, int width, int offset) {
  vector_t unit_vector = make_wall_unit_vector(point1, point2);
  vector_t perpendicular_vec = get_vector_perpendicular(unit_vector);
  vector_t scaled_vec = vec_multiply(width, perpendicular_vec);
  point1 = vec_add(point1, vec_multiply(offset, unit_vector));
  point2 = vec_add(point2, vec_multiply(-offset, unit_vector));

  list_t *rectangle = list_init(4, free);

  vector_t *corner1 = malloc(sizeof(vector_t));
  vector_t *corner2 = malloc(sizeof(vector_t));
  vector_t *corner3 = malloc(sizeof(vector_t));
  vector_t *corner4 = malloc(sizeof(vector_t));

  *corner1 = vec_add(point1, scaled_vec);
  *corner2 = vec_add(point2, scaled_vec);
  *corner3 = vec_add(point2, vec_multiply(-1, scaled_vec));
  *corner4 = vec_add(point1, vec_multiply(-1, scaled_vec));

  list_add(rectangle, corner1);
  list_add(rectangle, corner2);
  list_add(rectangle, corner3);
  list_add(rectangle, corner4);

  return rectangle;
}

list_t *make_walls(list_t *wall_coordinates) {
  size_t length = list_size(wall_coordinates);
  list_t *walls = list_init(length, (free_func_t) null_free);
  for (size_t i = 0; i < length; i++) {
    vector_t *point1 = (vector_t *) list_get(wall_coordinates, i % length);
    vector_t *point2 = (vector_t *) list_get(wall_coordinates, (i + 1) % length);

    list_t *rectangle = make_rectangle_with_width(*point1, *point2, WALL_WIDTH, -WALL_WIDTH);
    body_t *wall = body_init(rectangle, INFINITY, WALL_COLOR);
    list_add(walls, wall);
  }
  return walls;
}


void make_obstacle(scene_t *scene, list_t *obstacle_shape, minigolf_course_t course) {
    body_t *obstacle = body_init(obstacle_shape, INFINITY, OBS_COLOR);
    scene_add_body(scene, obstacle);
    create_physics_collision(scene, BALL_ELASTICITY, course.ball, obstacle);
}


minigolf_course_t make_minigolf_course(scene_t *scene, list_t *wall_coordinates,
  vector_t ball_center, vector_t hole_center, int par) {
  // make grass and add to scene
  body_t *grass = body_init(wall_coordinates, INFINITY, GRASS_COLOR);
  scene_add_body(scene, grass);

  // make walls and add to scene
  list_t *walls = make_walls(wall_coordinates);
  for (int i = 0; i < list_size(walls); i++) {
    scene_add_body(scene, (body_t *) list_get(walls, i));
  }

  // make hole and add to scene
  body_t *hole = body_init(make_circle(HOLE_RADIUS, hole_center), INFINITY, HOLE_COLOR);
  scene_add_body(scene, hole);

  // make ball and add to scene
  body_t *ball = body_init(make_circle(BALL_RADIUS, ball_center), BALL_MASS, BALL_COLOR);
  scene_add_body(scene, ball);

  // check that the entire ball and the entire hole are in the course
  list_t *ball_shape = body_get_shape(ball);
  list_t *hole_shape = body_get_shape(hole);
  assert(find_collision(ball_shape, wall_coordinates).collided == 1);
  assert(find_collision(hole_shape, wall_coordinates).collided == 1);

  // set up collisions for ball with walls and ball with hole, and ball with grass
  for (int i = 0; i < list_size(walls); i++) {
    body_t *wall = (body_t *) list_get(walls, i);
    list_t *wall_shape = body_get_shape(wall);
    create_physics_collision(scene, BALL_ELASTICITY, ball, wall);

    assert(find_collision(hole_shape, wall_shape).collided == 0);
    assert(find_collision(ball_shape, wall_shape).collided == 0);

    list_free(wall_shape);
  }

  // create_friction_collision() between ball and grass
  create_friction_collision(scene, ball, grass, GRASS_FRICTION);

  list_free(ball_shape);
  list_free(hole_shape);
  list_free(walls);

  body_t *velocity_line =
    body_init(make_rectangle_with_width(VEC_ZERO, (vector_t) {0, 0.1}, 1, 0), INFINITY, GRASS_COLOR);
  scene_add_body(scene, velocity_line);

  return (minigolf_course_t) {par, 0, ball, hole, velocity_line, VEC_ZERO};
}

int get_course_par(minigolf_course_t course) {
  return course.par;
}

int get_stroke_count(minigolf_course_t course) {
  return course.stroke_count;
}

void increment_stroke_count(minigolf_course_t *course) {
  course->stroke_count += 1;
}

body_t *get_golf_ball(minigolf_course_t course) {
  return course.ball;
}
