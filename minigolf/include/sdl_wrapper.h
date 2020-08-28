#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include <stdbool.h>
#include "body.h"
#include "color.h"
#include "list.h"
#include "scene.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

// Values passed to a key handler when the given arrow key is pressed
#define LEFT_ARROW 1
#define UP_ARROW 2
#define RIGHT_ARROW 3
#define DOWN_ARROW 4

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

typedef struct sdl_music sdl_music_t;

typedef struct sdl_text sdl_text_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 * @param body a body the key handler needs access to
 */
typedef void (*key_handler_t)(scene_t *scene, char key, key_event_type_t type, double held_time,
                              body_t *body);

/**
 * A mouse click handler.
 * When the left mouse button is clicked, the handler is passed the aux.
 *
 * @param aux a pointer to something the handler will use
*/
typedef void (*click_handler_t)(void *aux);

/**
 * A mouse motion handler.
 * When the mouse is moved, the handler is passed the location of the mouse
 * and the aux.
 *
 * @param mouse_loc the location of the mouse
 * @param aux a pointer to something the handler will use
*/
typedef void (*motion_handler_t)(vector_t mouse_loc, void *aux);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @param body a body that the key handler needs access to
 * @param scene the scene that the key handler needs access to
 * @param aux something the mouse handlers need access to
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(body_t *body, scene_t *scene, void *aux);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

void sdl_draw_scene(scene_t *scene);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene);

void sdl_render(scene_t *scene, sdl_text_t *text);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time the mouse is left clicked.
 * Overwrites any existing handler.
 *
 * @param handler the function to call with each key press
 */
void sdl_on_click(click_handler_t handler);

/**
 * Registers a function to be called every time the mouse is moved.
 * Overwrites any existing handler.
 *
 * @param handler the function to call with each key press
 */
void sdl_on_motion(motion_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

sdl_music_t *init_music(char* file);

void queue_music(sdl_music_t *music);

void free_music(sdl_music_t *music);

void set_font(char *fontpath, int fontsize);

// pos = center of text
sdl_text_t *init_text(vector_t pos, char *words);

void sdl_draw_text(sdl_text_t *text);

void free_text(sdl_text_t *text);

void free_font();

#endif // #ifndef __SDL_WRAPPER_H__
