#include "collision.h"
#include "list.h"
#include "vector.h"
#include "test_util.h"
#include "polygon.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>

const int N_POINTS = 360;
const int RADIUS = 50;

list_t *init_circle() {
  list_t *points = list_init(N_POINTS, free);
  vector_t *v = malloc(sizeof(vector_t));
  // add the first point in the circle
  *v = (vector_t) {0, RADIUS};
  list_add(points, v);
  vector_t old_v = *v;
  // draw the rest of the circle
  for (size_t j = 1; j < N_POINTS; j++) {
    double rotation_angle = 2 * M_PI / ((int) N_POINTS);
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;
  }
  return points;
}

void test_same_shape() {
  list_t *shape = init_circle();
  assert(find_collision(shape, shape).collided == 1);
  list_free(shape);
}

void test_overlapping_shapes() {
  list_t *shape1 = init_circle();
  list_t *shape2 = init_circle();
  polygon_translate(shape2, (vector_t) {RADIUS / 4, RADIUS / 4});
  assert(find_collision(shape1, shape2).collided == 1);
  list_free(shape1);
  list_free(shape2);
}

void test_tangent_shapes() {
  list_t *shape1 = init_circle();
  list_t *shape2 = init_circle();
  polygon_translate(shape2,
    (vector_t) {2 * RADIUS * cos(M_PI / 4), 2 * RADIUS * sin(M_PI / 4)});
  assert(find_collision(shape1, shape2).collided == 1);
  list_free(shape1);
  list_free(shape2);
}

void test_overlapping_boundaries() {
  list_t *shape1 = init_circle();
  list_t *shape2 = init_circle();
  polygon_translate(shape2,
    (vector_t) {2 * RADIUS * cos(M_PI / 4) + 1, 2 * RADIUS * sin(M_PI / 4) + 1});
  assert(find_collision(shape1, shape2).collided == 0);
  list_free(shape1);
  list_free(shape2);
}

void test_non_overlapping_boundaries() {
  list_t *shape1 = init_circle();
  list_t *shape2 = init_circle();
  polygon_translate(shape2,
    (vector_t) {3 * RADIUS * cos(M_PI / 4), 3 * RADIUS * sin(M_PI / 4)});
  assert(find_collision(shape1, shape2).collided == 0);
  list_free(shape1);
  list_free(shape2);
}

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_same_shape)
    DO_TEST(test_overlapping_shapes)
    DO_TEST(test_tangent_shapes)
    DO_TEST(test_overlapping_boundaries)
    DO_TEST(test_non_overlapping_boundaries)

    puts("collision_test PASS");
}
