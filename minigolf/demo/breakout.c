#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include "forces.h"
#include "force_aux.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

const double VX = 500;
const double VY = 500;

const double PLAYER_WIDTH = 30;
const double PLAYER_HEIGHT = 7.5;
const double PADDING = 5.0;

const int BLOCK_COLS = 10;
const int BLOCK_ROWS = 3;

const int N_BALL_POINTS = 400;
const int BALL_WIDTH = 7;

const double ELASTICITY = 1.0;
const double MASS = 10;
const int WIDTH = 500;
const int HEIGHT = 250;

// change to this instead
typedef enum {
    BALL,
    PLAYER,
    WALL,
    POWERUP,
    BLOCK
} body_type_t;


body_type_t *make_type_info(body_type_t type) {
    body_type_t *info = malloc(sizeof(*info));
    *info = type;
    return info;
}


body_type_t get_type(body_t *body) {
    return *(body_type_t *) body_get_info(body);
}


list_t *make_rectangle(double width, double height) {
    list_t *shape = list_init(4, free);

    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {width, height};
    list_add(shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-width, height};
    list_add(shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-width, -height};
    list_add(shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {width, -height};
    list_add(shape, v);

    return shape;
}

/**
 * Takes a scene and inits and adds a player on the screen. Returns
 * the body of the player.
 */
body_t *make_player(scene_t *scene) {
    list_t *shape = make_rectangle(PLAYER_WIDTH, PLAYER_HEIGHT);

    vector_t centroid = (vector_t) {0, -HEIGHT + PADDING + PLAYER_HEIGHT};

    // create players body and add it to the scene
    body_t *player = body_init_with_info(shape, INFINITY,
      (rgb_color_t) {1, 0, 0}, make_type_info(PLAYER), free);
    body_set_centroid(player, centroid);
    scene_add_body(scene, player);

    return player;
}

// problematic !!!!
void block_handler(body_t *ball, body_t *block, vector_t axis, void *aux) {
    physics_collision_handler(ball, block, axis, aux);

    if (get_type(block) == POWERUP) {
        body_set_velocity(ball, vec_multiply(3.0, body_get_velocity(ball)));
    }

    body_remove(block);
}


body_t *make_ball(scene_t *scene) {
    list_t *shape = list_init(N_BALL_POINTS, free);
    vector_t centroid = (vector_t) {0, -HEIGHT + 2 * PADDING + BALL_WIDTH + 2 * PLAYER_HEIGHT};

    vector_t *v = malloc(sizeof(vector_t));
    // add the first point in the circle
    *v = (vector_t) {0, BALL_WIDTH};
    list_add(shape, v);
    vector_t old_v = *v;
    // draw the rest of the circle
    for (size_t j = 1; j < N_BALL_POINTS; j++) {
        double rotation_angle = 2 * M_PI / ((int) N_BALL_POINTS - 1);
        v = malloc(sizeof(vector_t));
        *v = vec_rotate(old_v, rotation_angle);
        list_add(shape, v);
        old_v = *v;
    }

    // create ball body and add it to the scene
    body_t *ball = body_init_with_info(shape, MASS,
        (rgb_color_t) {1, 0, 0}, make_type_info(BALL), free);
    body_set_centroid(ball, centroid);
    body_set_velocity(ball, (vector_t) {VX / 3.0, VY / 3.0});
    scene_add_body(scene, ball);

    size_t body_count = scene_bodies(scene);
    // Add force creators with other bodies
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        switch (get_type(body)) {
            case BALL:
            case PLAYER:
            case WALL:
                // Bounce off walls/player and other balls lmao.
                create_physics_collision(scene, ELASTICITY, ball, body);
                break;
            case BLOCK:
            case POWERUP:
                // make the ball faster lmao
                create_collision(scene, ball, body, block_handler,
                    force_init(NULL, ELASTICITY), (free_func_t) force_free);
                break;
        }
    }
    return ball;
}


// make a bouncy wall. yes. jk make it bouncy elsewhere
void make_wall(scene_t *scene, vector_t centroid, double wall_width,
    double wall_height) {

    list_t *wall_shape = make_rectangle(wall_width, wall_height);

    body_t *wall = body_init_with_info(wall_shape, INFINITY,
      (rgb_color_t) {1, 1, 1}, make_type_info(WALL), free);
    body_set_centroid(wall, centroid);
    scene_add_body(scene, wall);
}


rgb_color_t get_rainbow(int num) {
    double rainbow[] = {0, 0, 0, 0, 0, 0.5, 1, 1, 1, 1, 1, 0.5};
    rgb_color_t color = (rgb_color_t) {rainbow[(num + 8) % 12],
        rainbow[(num + 4) % 12] , rainbow[num % 12]};
    return color;
}


void make_block(scene_t *scene, vector_t centroid, double block_width,
    double block_height, rgb_color_t color, bool powerup) {

    list_t *shape = make_rectangle(block_width, block_height);

    body_type_t *type_block = make_type_info(BLOCK);
    if (powerup) {
        type_block = make_type_info(POWERUP);
    }

    body_t *block = body_init_with_info(shape, INFINITY, color, type_block, free);
    body_set_centroid(block, centroid);
    scene_add_body(scene, block);
}

// makes all normal blocks + 1 special
void make_all_blocks(scene_t *scene) {
    int num_blocks = BLOCK_COLS * BLOCK_ROWS;
    // block half width lmao
    double block_width = (2 * WIDTH - PADDING * (BLOCK_COLS + 1.0)) / (2.0 * BLOCK_COLS);
    double block_height = block_width / 2.0;

    int special = rand() % num_blocks;

    // start on the upper left corner
    for (int i = 0; i < num_blocks; i++) {
        int row = i / 10;
        int col = i % 10;

        rgb_color_t color = get_rainbow(col);
        if (i == special) {
            color = (rgb_color_t) {0, 0, 0};
        }
        double width = -WIDTH + PADDING / 2.0 + (block_width + PADDING / 2.0) * (2 * col + 1);
        double height = HEIGHT - PADDING / 2.0 - (block_height + PADDING / 2.0) * (2 * row + 1);
        vector_t centroid = (vector_t) {width, height};

        make_block(scene, centroid, block_width, block_height, color, false);
    }
}


bool check_game_reset(body_t *ball, vector_t bottom_left, vector_t top_right) {
    list_t *points = body_get_shape(ball);
    size_t length = list_size(points);

    for (size_t i = 0; i < length; i++) {
        vector_t *point = (vector_t *) list_get(points, i);
        // check if ball is touching the bottom of the screen
        if (bottom_left.y - point->y >= 0) {
            list_free(points);
            return true;
        }
    }
    // if ball isn't touching the bottom, then return false
    list_free(points);
    return false;
}


void delete_game(scene_t *scene) {
    size_t body_count = scene_bodies(scene);
    // delete all blocks
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        switch (get_type(body)) {
            case BLOCK:
            case POWERUP:
            case BALL:
            case PLAYER:
                // reset ball, block and player
                body_remove(body);
                break;
            case WALL:
                break;
        }
    }
}


void on_key(scene_t *scene, char key, key_event_type_t type, double held_time,
    body_t *player) {

    if (type == KEY_PRESSED) {
        switch (key) {
            case UP_ARROW:
                break;
            case LEFT_ARROW:
                body_set_velocity(player, (vector_t) {-VX, 0});
                break;
            case DOWN_ARROW:
                break;
            case RIGHT_ARROW:
                body_set_velocity(player, (vector_t) {VX, 0});
                break;
        }
    }
    if (type == KEY_RELEASED) {
        body_set_velocity(player, (vector_t) {0,0});
    }
}


int main() {
    vector_t bottom_left = {-WIDTH, -HEIGHT};
    vector_t top_right = {WIDTH, HEIGHT};

    sdl_init(bottom_left, top_right);

    scene_t *scene = scene_init();
    bool is_game_over = false;

    make_wall(scene, (vector_t) {-2 * WIDTH, 0}, WIDTH, HEIGHT); // left
    make_wall(scene, (vector_t) {2 * WIDTH, 0}, WIDTH, HEIGHT); // right
    make_wall(scene, (vector_t) {0, 2 * HEIGHT}, WIDTH, HEIGHT); // top
    make_all_blocks(scene);

    body_t *player = make_player(scene);
    body_t *ball = make_ball(scene);

    sdl_on_key(on_key);

    while(!sdl_is_done(player, scene, NULL) && !is_game_over) {
        double time = time_since_last_tick();
        scene_tick(scene, time);
        sdl_render_scene(scene);

        if (check_game_reset(ball, bottom_left, top_right)) {
            delete_game(scene);
            make_all_blocks(scene);
            player = make_player(scene);
            ball = make_ball(scene);
        }
    }

    scene_free(scene);
    return 0;
}
