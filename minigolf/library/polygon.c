#include "polygon.h"
#include <math.h>
#include <stdlib.h>
#include "list.h"
#include "vector.h"

// https://mathworld.wolfram.com/Pentagram.html defines the inner radius and
// and outer radius size for a star.
const double RADIUS_RATIO = 0.200811 / 0.525731;
const double AREA_FACTOR = 2;
const double CENTROID_FACTOR = 6;

vector_t *get_vector_from_polygon(list_t *polygon, size_t i) {
    return (vector_t *)list_get(polygon, i);
}

double polygon_area(list_t *polygon) {
    double area = 0;
    size_t num_vertices = list_size(polygon);
    if (num_vertices > 2) {
        size_t i = 0;
        for (i = 0; i < num_vertices - 1; i++) {
            area += (get_vector_from_polygon(polygon, i + 1)->x +
                     get_vector_from_polygon(polygon, i)->x) *
                    (get_vector_from_polygon(polygon, i + 1)->y -
                     get_vector_from_polygon(polygon, i)->y);
        }
        area += (get_vector_from_polygon(polygon, 0)->x +
                 get_vector_from_polygon(polygon, i)->x) *
                (get_vector_from_polygon(polygon, 0)->y -
                 get_vector_from_polygon(polygon, i)->y);
    }
    return 1 / AREA_FACTOR * fabs(area);
}

vector_t polygon_centroid(list_t *polygon) {
    size_t length = list_size(polygon);

    double area = polygon_area(polygon);

    double c_x = 0;
    double c_y = 0;

    size_t i = 0;
    for (i = 0; i < length - 1; i++) {
        double x_i = get_vector_from_polygon(polygon, i)->x;
        double x_next_i = get_vector_from_polygon(polygon, i + 1)->x;
        double y_i = get_vector_from_polygon(polygon, i)->y;
        double y_next_i = get_vector_from_polygon(polygon, i + 1)->y;

        c_x += 1 / (CENTROID_FACTOR * area) * (x_i + x_next_i) * (x_i * y_next_i - x_next_i * y_i);
        c_y += 1 / (CENTROID_FACTOR * area) * (y_i + y_next_i) * (x_i * y_next_i - x_next_i * y_i);
    }
    // account for the edge between the last and first vertex in the polygon list
    c_x +=
        1 / (CENTROID_FACTOR * area) *
        (get_vector_from_polygon(polygon, i)->x +
         get_vector_from_polygon(polygon, 0)->x) *
        (get_vector_from_polygon(polygon, i)->x * get_vector_from_polygon(polygon, 0)->y -
         get_vector_from_polygon(polygon, 0)->x *
             get_vector_from_polygon(polygon, length - 1)->y);
    c_y +=
        1 / (CENTROID_FACTOR * area) *
        (get_vector_from_polygon(polygon, i)->y +
         get_vector_from_polygon(polygon, 0)->y) *
        (get_vector_from_polygon(polygon, i)->x * get_vector_from_polygon(polygon, 0)->y -
         get_vector_from_polygon(polygon, 0)->x *
             get_vector_from_polygon(polygon, length - 1)->y);

    vector_t centroid = {c_x, c_y};
    return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
    size_t length = list_size(polygon);

    for (size_t i = 0; i < length; i++) {
        vector_t *vertex = get_vector_from_polygon(polygon, i);
        vector_t new_vec = vec_add(*vertex, translation);
        vertex->x = new_vec.x;
        vertex->y = new_vec.y;
    }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
    size_t length = list_size(polygon);

    // translate each vertex so that the point of rotation is at the origin,
    // rotate, and then undo the translation
    vector_t translation_vec = vec_negate(point);
    for (size_t i = 0; i < length; i++) {
        vector_t *vertex = get_vector_from_polygon(polygon, i);
        vector_t new_vec = vec_add(*vertex, translation_vec);
        new_vec = vec_rotate(new_vec, angle);
        new_vec = vec_add(new_vec, point);

        vertex->x = new_vec.x;
        vertex->y = new_vec.y;
    }
}

list_t *star_init(double radius, size_t num_points) {
    list_t *points = list_init(2 * num_points, free);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t){0, radius};
    vector_t old_v = *v;
    list_add(points, v);

    for (size_t i = 0; i < 2 * num_points - 1; i++) {
        double angle = M_PI / num_points;
        vector_t *v_new = malloc(sizeof(vector_t));

        *v_new = vec_rotate(old_v, angle);
        // switch off between building outer and inner points of the star
        if (i % 2 == 1) {
            *v_new = vec_multiply(RADIUS_RATIO, *v_new);
        } else {
            *v_new = vec_multiply(1 / RADIUS_RATIO, *v_new);
        }
        old_v = *v_new;
        list_add(points, v_new);
    }
    return points;
}
