#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;

/**
 * The click handler, or NULL if none has been configured.
 */
click_handler_t click_handler = NULL;

/**
 * The motion handler, or NULL if none has been configured.
 */
motion_handler_t motion_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

TTF_Font *font;

typedef struct sdl_music {
    Uint32 wav_length;
    Uint8 *wav_buffer;
    SDL_AudioDeviceID device;
} sdl_music_t;

typedef struct sdl_text {
    SDL_Texture *texture;
    SDL_Rect rect;
} sdl_text_t;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
    int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
    assert(width != NULL);
    assert(height != NULL);
    SDL_GetWindowSize(window, width, height);
    vector_t dimensions = {.x = *width, .y = *height};
    free(width);
    free(height);
    return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
    // Scale scene so it fits entirely in the window
    double x_scale = window_center.x / max_diff.x, y_scale = window_center.y / max_diff.y;
    return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
    // Scale scene coordinates by the scaling factor
    // and map the center of the scene to the center of the window
    vector_t scene_center_offset = vec_subtract(scene_pos, center);
    double scale = get_scene_scale(window_center);
    vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
    vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                      // Flip y axis since positive y is down on the screen
                      .y = round(window_center.y - pixel_center_offset.y)};
    return pixel;
}

/** Maps a window coordinate to a scene coordinate */
vector_t get_scene_position(vector_t window_pos, vector_t window_center) {
  vector_t window_center_offset = vec_subtract(window_pos, window_center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply( 1.0 / scale, window_center_offset);
  vector_t pixel = {pixel_center_offset.x, -pixel_center_offset.y};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:
            return LEFT_ARROW;
        case SDLK_UP:
            return UP_ARROW;
        case SDLK_RIGHT:
            return RIGHT_ARROW;
        case SDLK_DOWN:
            return DOWN_ARROW;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode)(char)key ? key : '\0';
    }
}

void sdl_init(vector_t min, vector_t max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    window =
        SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, 0);

}

bool sdl_is_done(body_t *body, scene_t *scene, void *aux) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event != NULL);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (key_handler == NULL) break;
                char key = get_keycode(event->key.keysym.sym);
                if (key == '\0') break;

                uint32_t timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                key_event_type_t type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(scene, key, type, held_time, body);
                break;
            case SDL_MOUSEBUTTONDOWN: {
                switch (event->button.button)
                {
                    case SDL_BUTTON_LEFT: {
                        if (click_handler == NULL) break;
                        click_handler(aux);
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                if (motion_handler == NULL) break;
                vector_t mouse_loc = (vector_t) {event->motion.x, event->motion.y};
                mouse_loc = get_scene_position(mouse_loc, get_window_center());
                motion_handler(mouse_loc, aux);
                break;
            }
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    vector_t window_center = get_window_center();

    // Convert each vertex to a point on screen
    int16_t *x_points = malloc(sizeof(*x_points) * n),
            *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points != NULL);
    assert(y_points != NULL);
    for (size_t i = 0; i < n; i++) {
        vector_t *vertex = list_get(points, i);
        vector_t pixel = get_window_position(*vertex, window_center);
        x_points[i] = pixel.x;
        y_points[i] = pixel.y;
    }

    // Draw polygon with the given color
    filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255, color.g * 255,
                      color.b * 255, 255);
    free(x_points);
    free(y_points);
}

void sdl_show(void) {
    // Draw boundary lines
    vector_t window_center = get_window_center();
    vector_t max = vec_add(center, max_diff), min = vec_subtract(center, max_diff);
    vector_t max_pixel = get_window_position(max, window_center),
             min_pixel = get_window_position(min, window_center);
    SDL_Rect *boundary = malloc(sizeof(*boundary));
    boundary->x = min_pixel.x;
    boundary->y = max_pixel.y;
    boundary->w = max_pixel.x - min_pixel.x;
    boundary->h = min_pixel.y - max_pixel.y;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, boundary);
    free(boundary);

    SDL_RenderPresent(renderer);
}

void sdl_draw_scene(scene_t *scene) {
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        body_t *body = scene_get_body(scene, i);
        list_t *shape = body_get_shape(body);
        sdl_draw_polygon(shape, body_get_color(body));
        list_free(shape);
    }
}

void sdl_render_scene(scene_t *scene) {
    sdl_clear();
    sdl_draw_scene(scene);
    sdl_show();
}

void sdl_render(scene_t *scene, sdl_text_t *text) {
    sdl_clear();
    sdl_draw_scene(scene);
    sdl_draw_text(text);
    sdl_show();
}

void sdl_on_key(key_handler_t handler) {
    key_handler = handler;
}

void sdl_on_click(click_handler_t handler) {
    click_handler = handler;
}

void sdl_on_motion(motion_handler_t handler) {
    motion_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock ? (double)(now - last_clock) / CLOCKS_PER_SEC
                                   : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}


sdl_music_t *init_music(char* file) {
    static Uint32 wav_length;
    static Uint8 *wav_buffer;
    static SDL_AudioSpec wav_spec;

    if (SDL_LoadWAV(file, &wav_spec, &wav_buffer, &wav_length) == NULL) {
        fprintf(stderr, "Could not open audio file: %s\n", SDL_GetError());
    }

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);

    sdl_music_t *music = malloc(sizeof(sdl_music_t));
    *music = (sdl_music_t) {wav_length, wav_buffer, device};

    return music;
}

void queue_music(sdl_music_t *music) {
    int success = SDL_QueueAudio(music->device, music->wav_buffer, music->wav_length);
    if (success < 0) {
        fprintf(stderr, "Could not queue audio file: %s\n", SDL_GetError());
    }
    SDL_PauseAudioDevice(music->device, 0);
}

void free_music(sdl_music_t *music) {
    SDL_CloseAudioDevice(music->device);
    SDL_FreeWAV(music->wav_buffer);
    free(music);
}


void set_font(char *fontpath, int fontsize) {
    if(TTF_Init() < 0) {
	    fprintf(stderr, "Couldn't initialize TTF lib: %s\n", TTF_GetError());
	    exit(EXIT_FAILURE);
	}

    font = TTF_OpenFont(fontpath, fontsize);
    if (font == NULL) {
        fprintf(stderr, "error: font not found %s\n", SDL_GetError());
    }
}

sdl_text_t *init_text(vector_t pos, char *words) {
    SDL_Color text_color = {0, 0, 0};
    SDL_Surface *surface = TTF_RenderText_Solid(font, words, text_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int text_width = surface->w;
    int text_height = surface->h;
    SDL_FreeSurface(surface);

    vector_t window_center = get_window_center();
    vector_t vect = get_window_position(pos, window_center);
    SDL_Rect rect;
    rect.x = vect.x - text_width/2.0;
    rect.y = vect.y - text_height/2.0;
    rect.w = text_width;
    rect.h = text_height;

    sdl_text_t *text = malloc(sizeof(sdl_text_t));
    text->texture = texture;
    text->rect = rect;

    return text;
}


void sdl_draw_text(sdl_text_t *text) {
    SDL_RenderCopy(renderer, text->texture, NULL, &text->rect);
}

void free_text(sdl_text_t *text) {
    SDL_DestroyTexture(text->texture);
    free(text);
}

void free_font() {
    TTF_CloseFont(font);
    TTF_Quit();
}
