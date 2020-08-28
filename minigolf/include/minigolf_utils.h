#ifndef __MINIGOLF_UTILS_H__
#define __MINIGOLF_UTILS_H__

#include "list.h"
#include "vector.h"
#include "collision.h"
#include "scene.h"
#include <assert.h>

typedef struct minigolf_course {
  int par;
  int stroke_count;
  body_t *ball;
  body_t *hole;
  body_t *velocity_line;
  vector_t velocity_vec;
} minigolf_course_t;

void make_obstacle(scene_t *scene, list_t *obstacle_shape, minigolf_course_t course);

list_t *make_rectangle_with_width(vector_t point1, vector_t point2, int width, int offset);

/**
 * Makes a minigolf course in a scene out of wall coordinates, the center of the
 * ball, the center of the hole, and the par for the course
 * Wall coordinates are the corners of the course in counterclockwise order.
 */
minigolf_course_t make_minigolf_course(scene_t *scene, list_t *wall_coordinates,
  vector_t ball_center, vector_t hole_center, int par);

int get_course_par(minigolf_course_t course);

int get_stroke_count(minigolf_course_t course);

void increment_stroke_count(minigolf_course_t *course);

body_t *get_golf_ball(minigolf_course_t course);

#endif // #ifndef __MINIGOLF_UTILS_H__
