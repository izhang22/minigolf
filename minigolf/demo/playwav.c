#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "sdl_wrapper.h"
#include "vector.h"
#include <stdio.h>

/*
** PLAYING A SOUND IS MUCH MORE COMPLICATED THAN IT SHOULD BE
** RENDERING TEXT IS MUCH MORE COMPLICATED THAN IT SHOULD BE
*/
int main(int argc, char* argv[]) {

	sdl_init((vector_t) {-10, -10}, (vector_t) {10, 10});

	set_font("demo/OpenSans-Regular.ttf", 24);

	char words[20];

	sprintf(words, "par: %d fuck", 10);

	sdl_text_t *text = init_text((vector_t) {0, 0}, words);

    // static Uint32 wav_length;
    // static Uint8 *wav_buffer;
    // static SDL_AudioSpec wav_spec;
	//
    // if (SDL_LoadWAV("demo/victory.wav", &wav_spec, &wav_buffer, &wav_length) == NULL) {
    //     fprintf(stderr, "Could not open audio file: %s\n", SDL_GetError());
    //     return 1;
    // }
	//
    // SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
    // int success = SDL_QueueAudio(device, wav_buffer, wav_length);
    // success = SDL_QueueAudio(device, wav_buffer, wav_length);
    // SDL_PauseAudioDevice(device, 0);
	//
    // double time = 0;
    while(!sdl_is_done(NULL, NULL, NULL)) {
        // double dt = time_since_last_tick();
        // time += dt;
        // printf("%f\n", time);
        // printf("%d\n", wav_length);
        // if (time > 0.5) {
        //     success = SDL_QueueAudio(device, wav_buffer, wav_length);
        //     time = 0;
        //     printf("queued\n");
        // }
		sdl_clear();
		sdl_draw_text(text);
		sdl_show();
    }
	//
    // SDL_CloseAudioDevice(device);
    // SDL_FreeWAV(wav_buffer);

	free_text(text);
	free_font();
}
