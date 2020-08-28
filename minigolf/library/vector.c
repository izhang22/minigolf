#include "vector.h"
#include <math.h>

const vector_t VEC_ZERO = {0, 0};

vector_t vec_add(vector_t v1, vector_t v2) {
    vector_t add_vec = {v1.x + v2.x, v1.y + v2.y};
    return add_vec;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
    vector_t subtract_vec = {v1.x - v2.x, v1.y - v2.y};
    return subtract_vec;
}

vector_t vec_negate(vector_t v) {
    vector_t negated_vec = {-v.x, -v.y};
    return negated_vec;
}

vector_t vec_multiply(double scalar, vector_t v) {
    vector_t multiplied_vec = {scalar * v.x, scalar * v.y};
    return multiplied_vec;
}

double vec_dot(vector_t v1, vector_t v2) {
    double dot_prod = v1.x * v2.x + v1.y * v2.y;
    return dot_prod;
}

double vec_cross(vector_t v1, vector_t v2) {
    double z_component = v1.x * v2.y - v1.y * v2.x;
    return z_component;
}

vector_t vec_rotate(vector_t v, double angle) {
    vector_t rotated_vec = {v.x * cos(angle) - v.y * sin(angle),
                            v.x * sin(angle) + v.y * cos(angle)};
    return rotated_vec;
}

double vec_distance(vector_t v1, vector_t v2) {
    vector_t rad = vec_subtract(v2, v1);
    return sqrt(vec_dot(rad, rad));
}
