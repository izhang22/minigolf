#include "forces.h"
#include "force_aux.h"
#include "collision.h"
#include "list.h"
#include <math.h>
#include <stdlib.h>

const double MIN_DISTANCE = 0.01;
const double FRICTION_GRAVITY = 9.8;
const double BALL_EPSILON = 5.0;

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2) {
    list_t *bodies = list_init(2, null_free);
    list_add(bodies, body1);
    list_add(bodies, body2);

    force_aux_t *aux = force_init(bodies, G);

    scene_add_bodies_force_creator(scene, newtonian_gravity, (void *)aux,
        bodies, (free_func_t)force_free);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
    list_t *bodies = list_init(2, null_free);
    list_add(bodies, body1);
    list_add(bodies, body2);

    force_aux_t *aux = force_init(bodies, k);

    scene_add_bodies_force_creator(scene, spring, (void *)aux, bodies,
        (free_func_t)force_free);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
    list_t *bodies = list_init(1, null_free);
    list_add(bodies, body);

    force_aux_t *aux = force_init(bodies, gamma);

    scene_add_bodies_force_creator(scene, drag, (void *)aux, bodies,
        (free_func_t)force_free);
}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2) {
    list_t *bodies = list_init(2, null_free);
    list_add(bodies, body1);
    list_add(bodies, body2);

    force_aux_t *aux = force_init(bodies, 1);
    scene_add_bodies_force_creator(scene, destructive_collision, (void *) aux,
        bodies, (free_func_t)force_free);
}

void create_collision(
    scene_t *scene,
    body_t *body1,
    body_t *body2,
    collision_handler_t handler,
    void *aux,
    free_func_t freer
) {
  list_t *bodies = list_init(2, null_free);

  list_add(bodies, body1);
  list_add(bodies, body2);

  force_aux_t *new_aux = force_init(bodies, 1);
  force_set_collision_handler(new_aux, handler);
  force_set_extra_aux(new_aux, aux);
  force_set_freer(new_aux, freer);

  scene_add_bodies_force_creator(scene, collision, new_aux, bodies,
    (free_func_t) force_free);
}

void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis, void *aux) {
    double m_a = body_get_mass(body1);
    double m_b = body_get_mass(body2);

    double c_r = (double) force_get_constant(aux);

    vector_t v_a = body_get_velocity(body1);
    vector_t v_b = body_get_velocity(body2);

    double u_a = vec_dot(v_a, axis);
    double u_b = vec_dot(v_b, axis);

    double reduced_mass = m_a;
    if (m_a == INFINITY) {
        reduced_mass = m_b;
    }
    else if (m_a != INFINITY && m_b != INFINITY) {
        reduced_mass = (m_a * m_b)  / (m_a + m_b);
    }

    double j = reduced_mass * (1 + c_r) * (u_b - u_a);

    vector_t impulse1 = vec_multiply(j, axis);
    vector_t impulse2 = vec_multiply(-j, axis);

    body_add_impulse(body1, impulse1);
    body_add_impulse(body2, impulse2);
}

void create_physics_collision(
    scene_t *scene,
    double elasticity,
    body_t *body1,
    body_t *body2
) {
  force_aux_t *aux = force_init(NULL, elasticity);
  create_collision(scene, body1, body2, physics_collision_handler, aux,
    (free_func_t) force_free);
}

void create_friction_collision(
  scene_t *scene,
  body_t *body,
  body_t *ground,
  double coefficient
) {
  list_t *bodies = list_init(2, null_free);

  list_add(bodies, body);
  list_add(bodies, ground);

  force_aux_t *aux = force_init(bodies, coefficient);

  scene_add_bodies_force_creator(scene, friction_collision, aux, bodies,
    (free_func_t) force_free);
}


void friction_collision(void *aux) {
  body_t *body = force_get_body(aux, 0);
  list_t *body_shape = body_get_shape(body);
  body_t *ground = force_get_body(aux, 1);
  list_t *ground_shape = body_get_shape(ground);

  vector_t velocity = body_get_velocity(body);
  if (fabs(velocity.x) < BALL_EPSILON && fabs(velocity.y) < BALL_EPSILON) {
    body_set_velocity(body, VEC_ZERO);
  }
  else if (find_collision(body_shape, ground_shape).collided) {
    double mass = body_get_mass(body);
    double coefficient = force_get_constant(aux);
    vector_t velocity = body_get_velocity(body);
    double magnitude = sqrt(vec_dot(velocity, velocity));
    vector_t friction_unit_vec = vec_multiply(- 1.0 / magnitude, velocity);
    vector_t friction_force = vec_multiply(coefficient * mass * FRICTION_GRAVITY,
      friction_unit_vec);
    body_add_force(body, friction_force);
  }

  list_free(body_shape);
  list_free(ground_shape);
}

// newtonian gravity force creator
void newtonian_gravity(void *aux) {
    body_t *body1 = force_get_body(aux, 0);
    body_t *body2 = force_get_body(aux, 1);
    double G = force_get_constant(aux);
    vector_t radius = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    double dist = vec_distance(body_get_centroid(body2), body_get_centroid(body1));
    if (dist > MIN_DISTANCE) {
        double m1 = body_get_mass(body1);
        double m2 = body_get_mass(body2);
        vector_t force = vec_multiply(-G * m1 * m2 / pow(dist, 3), radius);
        body_add_force(body2, force);
        body_add_force(body1, vec_negate(force));
    }
}

// spring force creator
void spring(void *aux) {
    body_t *body1 = force_get_body(aux, 0);
    body_t *body2 = force_get_body(aux, 1);
    double k = force_get_constant(aux);
    vector_t radius = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    vector_t force = vec_multiply(k, radius);

    body_add_force(body1, force);
    body_add_force(body2, vec_negate(force));
}

// drag force creator
void drag(void *aux) {
    body_t *body = force_get_body(aux, 0);
    double gamma = force_get_constant(aux);
    body_add_force(body, vec_multiply(-gamma, body_get_velocity(body)));
}

void destructive_collision(void *aux) {
    body_t *body1 = force_get_body(aux, 0);
    body_t *body2 = force_get_body(aux, 1);

    list_t *shape1 = body_get_shape(body1);
    list_t *shape2 = body_get_shape(body2);

    if (find_collision(shape1, shape2).collided) {
        body_remove(body1);
        body_remove(body2);
    }

    list_free(shape1);
    list_free(shape2);
}

void collision(void *aux) {
    body_t *body1 = force_get_body(aux, 0);
    body_t *body2 = force_get_body(aux, 1);

    list_t *shape1 = body_get_shape(body1);
    list_t *shape2 = body_get_shape(body2);
    collision_handler_t handler = force_get_collision_handler(aux);
    bool is_collision_handled = force_get_is_collision_handled(aux);
    void *extra_aux = force_get_extra_aux(aux);
    collision_info_t info = find_collision(shape1, shape2);

    if (info.collided && !is_collision_handled) {
        handler(body1, body2, info.axis, extra_aux);
        force_set_is_collision_handled(aux, true);
    } else if (!find_collision(shape1, shape2).collided) {
        force_set_is_collision_handled(aux, false);
    }
    list_free(shape1);
    list_free(shape2);
}
