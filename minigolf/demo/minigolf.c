#include "sdl_wrapper.h"
#include "scene.h"
#include "minigolf_utils.h"
#include "minigolf_levels.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

const int WIDTH = 500;
const int HEIGHT = 250;

const int NUM_LEVELS = 5;

const double BGM_LENGTH = 102;

const rgb_color_t LINE_COLOR = (rgb_color_t) {0.9, 0.9, 0.9};
const rgb_color_t HIDDEN_LINE_COLOR = (rgb_color_t) {0, 0.5, 0};
const int LINE_WIDTH = 2;
const int LINE_OFFSET = 10;
const double VELOCITY_FACTOR = 5;

#define BGM_PATH "demo/Golf_BGM.wav"
#define PING_PATH "demo/Golf_ping.wav"
#define VIC_PATH "demo/victory.wav"

void shoot_ball(void *aux) {
  minigolf_course_t *minigolf_course = (minigolf_course_t *) aux;
  vector_t velocity = minigolf_course->velocity_vec;
  body_t *ball = minigolf_course->ball;

  vector_t ball_velocity = body_get_velocity(ball);
  if (ball_velocity.x == 0 && ball_velocity.y == 0) {
    body_set_velocity(ball, velocity);
    body_set_shape(minigolf_course->velocity_line,
      make_rectangle_with_width(VEC_ZERO, (vector_t) {0, 0.1}, 1, 0));
    body_set_color(minigolf_course->velocity_line, HIDDEN_LINE_COLOR);
    increment_stroke_count(minigolf_course);
  }
}

void aim_ball(vector_t mouse_loc, void *aux) {
  minigolf_course_t *minigolf_course = (minigolf_course_t *) aux;
  body_t *ball = minigolf_course->ball;

  vector_t ball_velocity = body_get_velocity(ball);
  if (ball_velocity.x == 0 && ball_velocity.y == 0) {
    list_t *ball_shape = body_get_shape(ball);
    vector_t centroid = polygon_centroid(ball_shape);
    list_free(ball_shape);


    list_t *shape = make_rectangle_with_width(mouse_loc, centroid, LINE_WIDTH, LINE_OFFSET);
    body_t *velocity_line = minigolf_course->velocity_line;
    body_set_shape(velocity_line, shape);
    body_set_color(minigolf_course->velocity_line, LINE_COLOR);
    vector_t dist = vec_subtract(centroid, mouse_loc);
    minigolf_course->velocity_vec = vec_multiply(VELOCITY_FACTOR, dist);
  }
}

void run_sim() {
    sdl_music_t *bgm = init_music(BGM_PATH);
    sdl_music_t *victory = init_music(VIC_PATH);

    queue_music(bgm);

    set_font("demo/OpenSans-Regular.ttf", 24);

    double bgm_timer = 0;
    for (int i = 1; i <= NUM_LEVELS; i++) {
        // make minigolf course
        scene_t *scene = scene_init();
        minigolf_course_t *course = malloc(sizeof(minigolf_course_t));
        *course = get_level(scene, i);

        body_t *hole = course->hole;
        list_t *hole_shape = body_get_shape(hole);
        body_t *ball = course->ball;
        list_t *ball_shape = body_get_shape(ball);

        char par[12];
        bool done = sdl_is_done(NULL, scene, course);
        while(!done && find_collision(hole_shape, ball_shape).collided == 0) {
            double time = time_since_last_tick();
            bgm_timer += time;

            if (bgm_timer >= BGM_LENGTH) {
                queue_music(bgm);
                bgm_timer = 0;
            }

            snprintf(par, 12, "par: %d", get_stroke_count(*course));
            sdl_text_t *text = init_text((vector_t) {450, 200}, par);

            scene_tick(scene, time);
            sdl_render(scene, text);
            list_free(ball_shape);
            ball_shape = body_get_shape(ball);

            if (find_collision(hole_shape, ball_shape).collided != 0) {
                // body_set_centroid(ball, body_get_centroid(hole)); // lmao this doesn't work
                queue_music(victory);
                // SDL_Delay(500);
            }
            done = sdl_is_done(NULL, scene, course);
            free_text(text);
        }

        list_free(hole_shape);
        list_free(ball_shape);
        free(course);
        scene_free(scene);
        if (done) {
          break;
        }
    }
    //free_text(text);
	free_font();
    free_music(bgm);
    free_music(victory);
}


int main() {
  vector_t bottom_left = {-WIDTH, -HEIGHT};
  vector_t top_right = {WIDTH, HEIGHT};
  sdl_init(bottom_left, top_right);
  sdl_on_click(shoot_ball);
  sdl_on_motion(aim_ball);

  run_sim();

  return 0;
}
