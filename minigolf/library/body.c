#include "body.h"

typedef struct body {
    list_t *shape;
    double mass;
    rgb_color_t color;
    vector_t centroid;
    vector_t velocity;
    double angle;
    vector_t force;
    vector_t impulse;
    void *info;
    free_func_t info_freer;
    bool remove;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
    body_t *body = malloc(sizeof(body_t));

    assert(mass > 0);
    assert(body != NULL);

    body->shape = shape;
    body->mass = mass;
    body->color = color;
    body->centroid = polygon_centroid(shape);
    body->velocity = VEC_ZERO;
    body->angle = 0.0;
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    body->info = NULL;
    body->info_freer = null_free;
    body->remove = false;
    return body;
}

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the mass is positive and that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving)
 * @param color the color of the body, used to draw it on the screen
 * @param info additional information to associate with the body,
 *   e.g. its type if the scene has multiple types of bodies
 * @param info_freer if non-NULL, a function call on the info to free it
 * @return a pointer to the newly allocated body
 */
body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
    void *info, free_func_t info_freer) {

    body_t *body = body_init(shape, mass, color);
    body->info = info;
    if (info_freer != NULL) {
        body->info_freer = info_freer;
    }

    return body;
}

void body_free(body_t *body) {
    list_free(body->shape);
    body->info_freer(body->info);
    free(body);
}

list_t *body_get_shape(body_t *body) {
    size_t size = list_size(body->shape);
    list_t *new_list = list_init(size, free);
    for (size_t i = 0; i < size; i++) {
        vector_t v_og = *(vector_t *)list_get(body->shape, i);
        /* make a deep copy of each vector */
        vector_t *v_copy = malloc(sizeof(vector_t));
        assert(v_copy != NULL);
        v_copy->x = v_og.x;
        v_copy->y = v_og.y;
        list_add(new_list, v_copy);
    }
    return new_list;
}

vector_t body_get_centroid(body_t *body) {
    return body->centroid;
}

vector_t body_get_velocity(body_t *body) {
    return body->velocity;
}

double body_get_mass(body_t *body) {
    return body->mass;
}

rgb_color_t body_get_color(body_t *body) {
    return body->color;
}

void *body_get_info(body_t *body) {
    return body->info;
}

void body_set_centroid(body_t *body, vector_t x) {
    vector_t move = vec_subtract(x, body->centroid);
    polygon_translate(body->shape, move);
    body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) {
    body->velocity = v;
}

void body_set_rotation(body_t *body, double angle) {
    polygon_rotate(body->shape, angle - body->angle, body->centroid);
    body->angle = angle;
}

void body_add_force(body_t *body, vector_t force) {
    body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
    body->impulse = vec_add(body->impulse, impulse);
}


void body_tick(body_t *body, double dt) {
    vector_t total = body->velocity;
    if (isfinite(body->mass)) {
        vector_t acc = vec_multiply(1.0 / body->mass, body->force);
        vector_t vel_force = vec_multiply(dt, acc);
        vector_t vel_impulse = vec_multiply(1.0 / body->mass, body->impulse);

        total = vec_add(vec_add(vel_impulse, vel_force), body->velocity);
        // set velocity to the avg between vel and current velocity
        vector_t vel_avg = vec_multiply(0.5, vec_add(total, body->velocity));
        body_set_velocity(body, vel_avg);
    }
    vector_t dist = vec_multiply(dt, body->velocity);
    vector_t center = vec_add(body->centroid, dist);
    body_set_centroid(body, center);
    body_set_velocity(body, total);
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
}


void body_remove(body_t *body) {
    body->remove = true;
}


bool body_is_removed(body_t *body) {
    if (body->remove) {
        return true;
    }
    return false;
}

void body_set_shape(body_t *body, list_t *shape) {
  list_free(body->shape);
  body->shape = shape;
}

void body_set_color(body_t *body, rgb_color_t color) {
    body->color = color;
}
