#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "body.h"
#include "scene.h"
#include "collision.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const double VX = 400;
const double VY = 400;

const size_t N_POINTS_PLAYER = 360;
const double PLAYER_WIDTH = 30;
const rgb_color_t GREEN = (rgb_color_t) {0, 1, 0};
const double PLAYER_MASS = 1;

const int OFF_X_AXIS = 1;
const int OFF_Y_AXIS = 2;

const size_t INITIAL_N_INVADERS = 16;
const size_t N_POINTS_INVADER = 123;
const double INVADER_RADIUS = 25;
const double INVADER_MASS = 1;
const vector_t INVADER_VELOCITY = (vector_t) {100, 0};
const double PADDING = 5.0;
const int INVADERS_PER_COL = 3;
const rgb_color_t GREY = (rgb_color_t) {0.5, 0.5, 0.5};
const double INVADER_SHOOTS_DT = 3.0;

const int N_POINTS_PELLET = 5;
const double PELLET_WIDTH = 7.0;
const double PELLET_MASS = 1;
const vector_t PLAYER_PELLET_VELOCITY = (vector_t) {0, 400};
const vector_t INVADER_PELLET_VELOCITY = (vector_t) {0, -400};

const int WIDTH = 500;
const int HEIGHT = 250;
const int INITIAL_SIZE_LIST = 8;
const double STAR_DT = 1;

const double ROTATION_RATIO = 2.0 / 3;

// Not const because body info takes a non-const void*
char *TYPE_PLAYER = "player";
char *TYPE_PLAYER_PELLET = "player pellet";
char *TYPE_INVADER = "invader";
char *TYPE_INVADER_PELLET = "invader pellet";

void do_nothing(void * variable) {
  ;
}

/**
 * Takes a scene and inits and adds a player on the screen. Returns
 * the body of the player.
 */
body_t *init_and_show_player(scene_t *scene) {
  list_t *points = list_init(N_POINTS_PLAYER, free);

  double vertical_shift = -HEIGHT + PLAYER_WIDTH + PADDING;

  vector_t *v = malloc(sizeof(vector_t));
  // add the first point in the circle
  *v = (vector_t) {0, PLAYER_WIDTH};
  list_add(points, v);
  vector_t old_v = *v;
  // draw the rest of the circle
  for (size_t j = 1; j < N_POINTS_PLAYER; j++) {
    double rotation_angle = 2 * M_PI / ((int) N_POINTS_PLAYER - 1);
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;
  }

  // create players body and add it to the scene
  char *type_player = TYPE_PLAYER;
  body_t *player = body_init_with_info(points, PLAYER_MASS, GREEN, type_player, do_nothing);
  body_set_centroid(player, (vector_t){0, vertical_shift});
  scene_add_body(scene, player);
  return player;
}

/**
 * Return a random invader from the invaders in the scene
 */
body_t *get_random_invader(scene_t * scene) {

  int n_bodies = (int) scene_bodies(scene);
  body_t *random_invader = scene_get_body(scene, rand() % n_bodies);
  while(body_get_info(random_invader) != TYPE_INVADER) {
    random_invader = scene_get_body(scene, rand() % n_bodies);
  }
  return random_invader;
}

/**
 * Produce translation vectors for invaders
 */
vector_t get_invader_translation(int space_invader_num) {
  int column_num = (space_invader_num == 0) ? 0 :
    space_invader_num / INVADERS_PER_COL;
  int row_num = (space_invader_num == 0) ? 0 :
    space_invader_num % INVADERS_PER_COL;

  double x_comp = -WIDTH + PADDING + INVADER_RADIUS +
    column_num * (2.0 * INVADER_RADIUS + PADDING);
  double y_comp = HEIGHT - PADDING - INVADER_RADIUS -
    row_num * (INVADER_RADIUS + PADDING);

  return (vector_t) {x_comp, y_comp};
}


/**
 * Takes a scene and a number of invaders, and inits and adds that number
 * of invaders to the scene starting from the top left
 */
void init_and_show_invaders(scene_t *scene, int num_invaders) {
  for (size_t i = 0; i < num_invaders; i++) {
    list_t *points = list_init(N_POINTS_INVADER, free);
    vector_t *v = malloc(sizeof(vector_t));

    // add the first point, an outer corner
    *v = (vector_t) {0, INVADER_RADIUS};
    list_add(points, v);
    vector_t old_v = *v;

    // add the inner corner of the invader, which is at the center of the circle
    vector_t *origin = malloc(sizeof(vector_t));
    *origin = (vector_t) {0, 0};
    list_add(points, origin);

    // rotate 240º, add the other outer corner of the invader
    double rotation_angle = 4 * M_PI / 3;
    v = malloc(sizeof(vector_t));
    *v = vec_rotate(old_v, rotation_angle);
    list_add(points, v);
    old_v = *v;

    // draw the rest of the invader's body as a circle
    for (size_t j = 2; j < N_POINTS_INVADER; j++) {
      rotation_angle = ROTATION_RATIO * M_PI / ((int) N_POINTS_INVADER - 3);
      v = malloc(sizeof(vector_t));
      *v = vec_rotate(old_v, rotation_angle);
      list_add(points, v);
      old_v = *v;
    }

    // translate the invader to a spot on the starting grid
    polygon_translate(points, get_invader_translation((int) i));

    body_t *invader = body_init_with_info(points, INVADER_MASS, GREY, TYPE_INVADER, do_nothing);
    body_set_rotation(invader, 2.0 * M_PI / 6.0);
    body_set_velocity(invader, INVADER_VELOCITY);
    scene_add_body(scene, invader);
  }
}

void player_shoots(body_t *player, scene_t *scene) {
  list_t *points = list_init(N_POINTS_PELLET, free);

  vector_t *vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {0, 0};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {0, PELLET_WIDTH};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {PELLET_WIDTH, PELLET_WIDTH};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {PELLET_WIDTH, 0};
  list_add(points, vec);

  // create pellet's body and add it to the scene
  body_t *pellet = body_init_with_info(points, PELLET_MASS, GREEN, TYPE_PLAYER_PELLET, do_nothing);
  body_set_centroid(pellet, body_get_centroid(player));
  body_set_velocity(pellet, PLAYER_PELLET_VELOCITY);
  scene_add_body(scene, pellet);
}

void invader_shoots(body_t *invader, scene_t *scene) {
  list_t *points = list_init(N_POINTS_PELLET, free);

  vector_t *vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {0, 0};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {0, PELLET_WIDTH};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {PELLET_WIDTH, PELLET_WIDTH};
  list_add(points, vec);

  vec = malloc(sizeof(vector_t));
  *vec = (vector_t) {PELLET_WIDTH, 0};
  list_add(points, vec);

  // create pellet's body and add it to the scene
  body_t *pellet = body_init_with_info(points, PELLET_MASS, GREY, TYPE_INVADER_PELLET,do_nothing);
  body_set_centroid(pellet, body_get_centroid(invader));
  body_set_velocity(pellet, INVADER_PELLET_VELOCITY);
  scene_add_body(scene, pellet);
}

/**
 * Returns 0 if body is completely in bounds
  *        1 if body is outside along the x axis
  *        2 if body is outside along the y axis
 */
int is_body_in_bounds(body_t *body, vector_t bottom_left, vector_t top_right) {
    list_t *points = body_get_shape(body);
    size_t length = list_size(points);

    int num_points_off_x_axis = 0;
    int num_points_off_y_axis = 0;

    for (size_t i = 0; i < length; i++) {
      vector_t *point = (vector_t *) list_get(points, i);
      // check if body is outside the x axis
      if (point->x - top_right.x >= 0 || bottom_left.x - point->x >= 0) {
        num_points_off_x_axis++;
        if (num_points_off_x_axis >= length - 1) {
          list_free(points);
          return OFF_X_AXIS;
        }
      }
      // check if body is outside the y axis
      if (point->y - top_right.y >= 0 || bottom_left.y - point->y >= 0) {
        num_points_off_y_axis++;
        if (num_points_off_y_axis >= length - 1) {
          list_free(points);
          return OFF_Y_AXIS;
        }
      }
    }
    // if body is inside both bounds, return 0
    list_free(points);
    return 0;
  }

/**
 * Dictates the direction of player's movement on arrow presses, and shoots
 * when encountering a space.
 */
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
      case ' ':
        player_shoots(player, scene);
    }
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(player, (vector_t) {0,0});
  }
}

void run_sim(scene_t *scene, vector_t bottom_left, vector_t top_right) {
  bool is_game_over = false;
  double time_passed_invader_shoots = 0;

  body_t *player = init_and_show_player(scene);
  init_and_show_invaders(scene, INITIAL_N_INVADERS);

  sdl_on_key(on_key);

  while(!sdl_is_done(player, scene, NULL) && !is_game_over) {
    int n_bodies = (int) scene_bodies(scene);

    int invaders_idx_list[n_bodies];
    int n_invaders = 0;

    int invader_pellets_idx_list[n_bodies];
    int n_invader_pellets = 0;

    int player_pellets_idx_list[n_bodies];
    int n_player_pellets = 0;

    int idx = 0;

    // add all bodies to their respective lists and deal with out of bounds bodies
    while (idx < n_bodies) {
      body_t *body = scene_get_body(scene, idx);
      char *type = body_get_info(body);

      int wrap = is_body_in_bounds(body, bottom_left, top_right);

        if (type == TYPE_PLAYER_PELLET) {
          if (wrap == 0) {
            player_pellets_idx_list[n_player_pellets] = idx;
            idx++;
            n_player_pellets++;
          }
          else {
            scene_remove_body(scene, idx);
            n_bodies--;
          }
        }
        else if (type == TYPE_INVADER_PELLET) {
          if (wrap == 0) {
            invader_pellets_idx_list[n_invader_pellets] = idx;
            idx++;
            n_invader_pellets++;
          }
          else {
            scene_remove_body(scene, idx);
            n_bodies--;
          }
        }
        else if (type == TYPE_INVADER) {
          if (wrap == OFF_X_AXIS) {
            vector_t centroid = body_get_centroid(body);
            body_set_centroid(body, (vector_t) {centroid.x, centroid.y
              - 4.0 * (INVADER_RADIUS + PADDING)});
            vector_t velocity = body_get_velocity(body);
            body_set_velocity(body, (vector_t) {-velocity.x, velocity.y});

            invaders_idx_list[n_invaders] = idx;
            idx++;
            n_invaders++;
          }
          if (wrap == OFF_Y_AXIS) {
            is_game_over = true;
          }
          if (wrap == 0) {
            invaders_idx_list[n_invaders] = idx;
            idx++;
            n_invaders++;
          }
        }
        else if (type == TYPE_PLAYER) {
          if (wrap == OFF_X_AXIS) {
            vector_t centroid = body_get_centroid(body);
            body_set_centroid(body, (vector_t) {-centroid.x, centroid.y});
          }
          idx++;
        }

      for (int i = 0; i < n_invader_pellets; i++) {
        list_t *player_shape = body_get_shape(player);
        list_t *pellet_shape = body_get_shape(scene_get_body(scene, invader_pellets_idx_list[i]));
        bool is_collided = find_collision(player_shape, pellet_shape).collided;
        list_free(player_shape);
        list_free(pellet_shape);

        if (is_collided) {
          is_game_over = true;
          break;
        }
      }

      int num_invaders_remaining = n_invaders;

      for (int j = 0; j < n_player_pellets; j++) {
        for (int k = 0; k < n_invaders; k++) {
          if (invaders_idx_list[k] != -1) {
            list_t *invader_shape = body_get_shape(scene_get_body(scene, invaders_idx_list[k]));
            list_t *pellet_shape = body_get_shape(scene_get_body(scene, player_pellets_idx_list[j]));
            bool is_collided = find_collision(
              invader_shape,
              pellet_shape
            ).collided;
            list_free(invader_shape);
            list_free(pellet_shape);

            if (is_collided && num_invaders_remaining == 1) {
              is_game_over = true;
              break;
            }

            if (is_collided) {
              scene_remove_body(scene, invaders_idx_list[k]);
              scene_remove_body(scene, player_pellets_idx_list[j]);

              invaders_idx_list[k] = -1;
              num_invaders_remaining--;
              break;
            }
          }
        }
      }

    }

    // shoot a new pellet about every INVADER_SHOOTS_DT seconds
    double time = time_since_last_tick();
    time_passed_invader_shoots += time;

    if (time_passed_invader_shoots > INVADER_SHOOTS_DT) {
      body_t *random_invader = get_random_invader(scene);
      invader_shoots(random_invader, scene);
      time_passed_invader_shoots = 0;
    }

    scene_tick(scene, time);
    sdl_render_scene(scene);
  }
}

int main() {
  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);

  scene_t *scene = scene_init();
  run_sim(scene, bottom_left, top_right);

  scene_free(scene);
  return 0;
}
