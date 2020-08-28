#include "collision.h"
#include "list.h"
#include "vector.h"
#include "polygon.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

double max(double d1, double d2) {
  return d1 > d2 ? d1 : d2;
}

double min(double d1, double d2) {
  return d1 < d2 ? d1 : d2;
}

/**
 * Takes in a list of vector_ts and returns either the top right or the bottom
 * left bounding corner based on whether comparison_func is either min or max
 */
vector_t bounding_corner(list_t *shape, double (*comparison_func) (double, double)) {
  size_t size = list_size(shape);
  assert(size != 0);
  vector_t *point = (vector_t *) list_get(shape, 0);
  double x = point->x;
  double y = point->y;
  for (size_t i = 1; i < size; i++) {
    point = (vector_t *) list_get(shape, i);
    x = comparison_func(point->x, x);
    y = comparison_func(point->y, y);
  }
  return (vector_t) {x, y};
}

/**
 * Takes in two shapes and returns whether the bounding box intersects
 */
bool bounding_box_intersect(list_t *shape1, list_t *shape2) {
  vector_t bottom_left1 = bounding_corner(shape1, min);
  vector_t bottom_left2 = bounding_corner(shape2, min);
  vector_t top_right1 = bounding_corner(shape1, max);
  vector_t top_right2 = bounding_corner(shape2, max);
  bool xs_overlap =
    top_right1.x >= bottom_left2.x && top_right2.x >= bottom_left1.x;
  bool ys_overlap =
    top_right1.y >= bottom_left2.y && top_right2.y >= bottom_left1.y;

  return xs_overlap && ys_overlap;
}

/**
 * Takes two vector_ts and returns a unit vector_t pointing in the direction
 * of the line formed by connecting the two vertices
 */
vector_t make_unit_vector(vector_t v1, vector_t v2) {
  vector_t direction = vec_subtract(v2, v1);
  double magnitude = vec_distance(v1, v2);
  vector_t unit_vector = vec_multiply(1.0 / magnitude, direction);
  return unit_vector;
}

vector_t get_perpendicular(vector_t v, vector_t centroid1, vector_t centroid2) {
  vector_t unit = make_unit_vector(centroid1, centroid2);
  vector_t perpendicular = (vector_t) {-v.y / sqrt(v.x * v.x + v.y * v.y),
    v.x / sqrt(v.x * v.x + v.y * v.y)};
  if (vec_dot(unit, perpendicular) > 0) {
    return perpendicular;
  }
  return vec_multiply(-1, perpendicular);
}

/**
 * Takes a list_t of vector_ts representing a shape, and two vector_ts,
 * and projects each point in the shape onto the line created by the two
 * vectors, then returns the minimum and maximum values produced as a vector_t
 * of form {min, max}.
 */
vector_t project_shape(list_t *shape, vector_t min_point, vector_t max_point, vector_t centroid1, vector_t centroid2) {
  vector_t unit_vector = get_perpendicular(make_unit_vector(min_point, max_point), centroid1, centroid2);
  size_t size = list_size(shape);
  assert(size > 0);
  vector_t *point = (vector_t *) list_get(shape, 0);
  double min_projection = vec_dot(unit_vector, *point);
  double max_projection = vec_dot(unit_vector, *point);

  for (size_t i = 1; i < size; i++) {
    point = (vector_t *) list_get(shape, i);
    double projection = vec_dot(unit_vector, *point);
    min_projection = min(min_projection, projection);
    max_projection = max(max_projection, projection);
  }
  return (vector_t) {min_projection, max_projection};
}

double overlap(vector_t projection1, vector_t projection2) {
  double overlap;
  // e.g. (0, 3) (2, 5)
  // projection1 is left of projection2
  if (projection1.x < projection2.x && projection1.y < projection2.y) {
    overlap = projection1.y - projection2.x;
  }
  // e.g. (2, 5) (0, 3)
  // projection2 is left of projection1
  else if (projection1.x >= projection2.x && projection1.y >= projection2.y) {
    overlap = projection2.y - projection1.x;
  }
  // e.g. (3, 4) (2, 5)
  // projection1 is inside of projection2
  else if (projection1.x >= projection2.x && projection1.y < projection2.y) {
    overlap = projection1.y - projection1.x;
  }
  // e.g. (2, 5) (3, 4)
  // projection2 is inside of projection1
  else {
    overlap = projection2.y - projection2.x;
  }

  return overlap;
}

/**
 * Takes two list_ts of vector_ts shape1 and shape2, and returns a
 * collision_info_t that indicates whether the shapes are colliding
 * and along what axis
 */
collision_info_t all_projections_overlapping(list_t *shape1, list_t *shape2) {
  size_t size1 = list_size(shape1);
  double min_overlap = 0;
  vector_t axis = VEC_ZERO;

  vector_t centroid1 = polygon_centroid(shape1);
  vector_t centroid2 = polygon_centroid(shape2);
  for (size_t i = 0; i < size1; i++) {
    vector_t *point1 = (vector_t *) list_get(shape1, i % size1);
    vector_t *point2 = (vector_t *) list_get(shape1, (i + 1) % size1);
    vector_t projection1 = project_shape(shape1, *point1, *point2, centroid1, centroid2);
    vector_t projection2 = project_shape(shape2, *point1, *point2, centroid1, centroid2);

    double overlap_amt = overlap(projection1, projection2);
    if (projection1.y < projection2.x || projection2.y < projection1.x) {
      return (collision_info_t) {false, axis};
    } else if (min_overlap == 0 || overlap_amt < min_overlap) {
      min_overlap = overlap_amt;
      axis = get_perpendicular(make_unit_vector(*point1, *point2), centroid1, centroid2);
    }
  }

  size_t size2 = list_size(shape2);
  for (size_t i = 0; i < size2; i++) {
    vector_t *point1 = (vector_t *) list_get(shape2, i % size2);
    vector_t *point2 = (vector_t *) list_get(shape2, (i + 1) % size2);
    vector_t projection1 = project_shape(shape1, *point1, *point2, centroid1, centroid2);
    vector_t projection2 = project_shape(shape2, *point1, *point2, centroid1, centroid2);

    double overlap_amt = overlap(projection1, projection2);
    if (projection1.y < projection2.x || projection2.y < projection1.x) {
      return (collision_info_t) {false, axis};
    } else if (min_overlap == 0 || overlap_amt < min_overlap) {
      min_overlap = overlap_amt;
      axis = get_perpendicular(make_unit_vector(*point1, *point2), centroid1, centroid2);
    }
  }
  return (collision_info_t) {true, axis};
}


collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  if (!bounding_box_intersect(shape1, shape2)) {
    return (collision_info_t) {false, VEC_ZERO};
  }
  return all_projections_overlapping(shape1, shape2);
}
