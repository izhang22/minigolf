// TODO: IMPLEMENT YOUR TESTS IN THIS FILE
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "vector.h"
#include "forces.h"
#include "body.h"
#include "list.h"
#include "test_util.h"

const double MIN_DIST = 0.01;

list_t *make_shape() {
    list_t *shape = list_init(4, free);
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){-1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){+1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){+1, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (vector_t){-1, +1};
    list_add(shape, v);
    return shape;
}

// Helper functions

double gravity_force(double G, body_t *body1, body_t *body2) {
    double r = vec_distance(body_get_centroid(body2), body_get_centroid(body1));
    double g_force = G * body_get_mass(body1) * body_get_mass(body2) / (r * r);
    return g_force;
}
vector_t gravity_force_vec(double G, body_t *body1, body_t *body2) {
  double r = vec_distance(body_get_centroid(body2), body_get_centroid(body1));
  double g_force = -G * body_get_mass(body1) * body_get_mass(body2) / pow(r, 3);
  return vec_multiply(g_force, vec_subtract(body_get_centroid(body2),
  body_get_centroid(body1)));
}
double kinetic_energy(body_t *body) {
    vector_t v = body_get_velocity(body);
    return body_get_mass(body) * vec_dot(v, v) / 2;
}
double elastic_potential(body_t *mass1, body_t *mass2, double k) {
  vector_t x = vec_subtract(body_get_centroid(mass1), body_get_centroid(mass2));
  return k * vec_dot(x, x) / 2;
}

// Tests that U + K is conserved for a spring

void test_spring_energy_conservation() {
    const double M = 10, M2 = 5;
    const double K = 2;
    const double A = 3;
    const double DT = 1e-6;
    const int STEPS = 1000000;

    scene_t *scene = scene_init();
    body_t *mass1 = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
    body_set_centroid(mass1, (vector_t){A, 0});
    scene_add_body(scene, mass1);
    body_t *mass2 = body_init(make_shape(), M2, (rgb_color_t){0, 0, 0});
    body_set_centroid(mass2, (vector_t){10, 20});
    scene_add_body(scene, mass2);
    create_spring(scene, K, mass1, mass2);
    double initial_energy = elastic_potential(mass1, mass2, K);

    for (int i = 0; i < STEPS; i++) {
      double energy = elastic_potential(mass1, mass2, K) + kinetic_energy(mass1) +
                      kinetic_energy(mass2);
      assert(within(1e-4, energy / initial_energy, 1));
      scene_tick(scene, DT);
    }
    scene_free(scene);
}

// Tests F = ma for newtonian gravity...using vectors
void test_second_law_vector() {
    const double M1 = 4.5, M2 = 7.3;
    const double G = 1e3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    scene_t *scene = scene_init();
    body_t *mass1 = body_init(make_shape(), M1, (rgb_color_t){0, 0, 0});
    scene_add_body(scene, mass1);
    body_t *mass2 = body_init(make_shape(), M2, (rgb_color_t){0, 0, 0});
    body_set_centroid(mass2, (vector_t){200, 20});
    scene_add_body(scene, mass2);
    create_newtonian_gravity(scene, G, mass1, mass2);

    vector_t prev_v_m1 = VEC_ZERO;
    vector_t prev_v_m2 = VEC_ZERO;
    vector_t g_force = VEC_ZERO;

    for (int i = 0; i < STEPS; i++) {
        assert(body_get_centroid(mass1).x < body_get_centroid(mass2).x);

        vector_t curr_v_m1 = body_get_velocity(mass1);
        vector_t curr_v_m2 = body_get_velocity(mass2);

        vector_t acc_vec_m1 = vec_multiply(1/DT, (vec_subtract(curr_v_m1, prev_v_m1)));
        vector_t acc_vec_m2 = vec_multiply(1/DT, (vec_subtract(curr_v_m2, prev_v_m2)));
        //printf("g force: %f %f\n", g_force.x, g_force.y);

        if (i != 0) {
          assert(vec_within(1e-5, g_force, vec_multiply(-M1, acc_vec_m1)));
          assert(vec_within(1e-5, g_force, vec_multiply(M2, acc_vec_m2)));
        }

        prev_v_m1 = curr_v_m1;
        prev_v_m2 = curr_v_m2;
        if (vec_distance(body_get_centroid(mass1), body_get_centroid(mass2))
            > MIN_DIST) {
            g_force = gravity_force_vec(G, mass1, mass2);
        }
        else {
            g_force = VEC_ZERO;
        }
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

// Tests drag force velocity is decreasing
void test_drag_force() {
    const double M = 10;
    const double GAMMA = 0.002;
    const double A = 3;
    const vector_t INITIAL_VELOCITY = {0, 2};
    const double DT = 1e-3;
    const int STEPS = 1000000;

    scene_t *scene = scene_init();
    body_t *body = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
    body_set_centroid(body, (vector_t){A, 0});
    body_set_velocity(body, INITIAL_VELOCITY);
    scene_add_body(scene, body);
    create_drag(scene, GAMMA, body);
    vector_t prev_v = body_get_velocity(body);

    for (int i = 0; i < STEPS; i++) {
      vector_t velocity = body_get_velocity(body);

      if (i % 10000 == 0) {
        assert((prev_v.y - velocity.y) >= 0);
        prev_v = velocity;
      }
      scene_tick(scene, DT);
    }
    scene_free(scene);
}


int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
      read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_spring_energy_conservation)
  DO_TEST(test_drag_force)
  DO_TEST(test_second_law_vector)

  puts("student_tests PASS");
}
