#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "color.h"
#include "list.h"
#include "vector.h"

/**
 * Contains the polygon's vertices, color, velocity, and elasticity.
 */
typedef struct polygon {
    list_t *vertices;
    rgb_color_t color;
    vector_t velocity;
    double elasticity;
} polygon_t;

/**
 * Computes the area of a polygon.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the polygon
 */
double polygon_area(list_t *polygon);

/**
 * Computes the center of mass of a polygon.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the polygon
 */
vector_t polygon_centroid(list_t *polygon);

/**
 * Translates all vertices in a polygon by a given vector.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param translation the vector to add to each vertex's position
 */
void polygon_translate(list_t *polygon, vector_t translation);

/**
 * Rotates vertices in a polygon by a given angle about a given point.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param angle the angle to rotate the polygon, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */
void polygon_rotate(list_t *polygon, double angle, vector_t point);

/**
 * Initializes a star with a given radius and number of points.
 *
 * @param radius - the outer radius of the star
 * @param num_points - the number of outer points in the star
 * @return list of points that make up a star
 **/
list_t *star_init(double radius, size_t num_points);

/**
 * Gets the ith vector as a vector_t of a list_t polygon.
 *
 * @param polygon - a list_t of vector_ts
 * @param i - the index of the vector
 * @return a pointer to the vector
 */
vector_t *get_vector_from_polygon(list_t *polygon, size_t i);

#endif // #ifndef __POLYGON_H__
