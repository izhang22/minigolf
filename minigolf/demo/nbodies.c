#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "body.h"
#include "color.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "vector.h"

const int N_STARS = 40;
const int MIN_STAR_RADIUS = 5;
const int MAX_STAR_RADIUS = 10;
const int MIN_STAR_MASS = 10000;
const int MAX_STAR_MASS = 20000;
const int MIN_STAR_VELOCITY = -100;
const int MAX_STAR_VELOCITY = 100;
const int N_STAR_POINTS = 4;
const double G = 200;

const int WIDTH = 500;
const int HEIGHT = 250;

/**
 * Randomize where a star will appear on the screen relative to the center
 */
vector_t get_random_translation() {
    return (vector_t){(rand() % (2 * WIDTH)) - WIDTH, (rand() % (2 * HEIGHT)) - HEIGHT};
}

/**
 * Gets random integer between min and min + width
 */
int get_random_num(int min, int width) {
    return (rand() % width + min);
}

/**
 * Takes a scene and a number of stars, and inits and adds that number
 * of stars to the scene in random locations
 */
void init_and_show_stars(scene_t *scene, int num_stars) {
    for (size_t i = 0; i < num_stars; i++) {
        int radius = get_random_num(MIN_STAR_RADIUS, MAX_STAR_RADIUS - MIN_STAR_RADIUS);
        list_t *points = star_init(radius, N_STAR_POINTS);

        int mass = get_random_num(MIN_STAR_MASS, MAX_STAR_MASS - MIN_STAR_MASS);
        body_t *star = body_init(points, mass, get_random_color());
        vector_t centroid = get_random_translation();
        body_set_centroid(star, centroid);
        int vx = get_random_num(MIN_STAR_VELOCITY, MAX_STAR_VELOCITY - MIN_STAR_VELOCITY);
        int vy = get_random_num(MIN_STAR_VELOCITY, MAX_STAR_VELOCITY - MIN_STAR_VELOCITY);

        body_set_velocity(star, (vector_t) {vx, vy});
        scene_add_body(scene, star);
    }
}

void create_gravities_between_stars(scene_t *scene) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        for (size_t j = i + 1; j < scene_bodies(scene); j++) {
            body_t *body1 = scene_get_body(scene, i);
            body_t *body2 = scene_get_body(scene, j);
            create_newtonian_gravity(scene, G, body1, body2);
        }
    }
}

void run_sim(scene_t *scene) {
    init_and_show_stars(scene, N_STARS);
    create_gravities_between_stars(scene);

    time_since_last_tick();
    while (!sdl_is_done(NULL, scene, NULL)) {
        scene_tick(scene, time_since_last_tick());
        sdl_render_scene(scene);
    }
}

int main() {
    vector_t bottom_left = {-WIDTH, -HEIGHT};
    vector_t top_right = {WIDTH, HEIGHT};
    sdl_init(bottom_left, top_right);

    scene_t *scene = scene_init();
    run_sim(scene);

    scene_free(scene);
    return 0;
}
