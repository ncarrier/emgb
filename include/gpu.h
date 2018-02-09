#ifndef GB_GPU
#define GB_GPU
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "save.h"
#include "video_common.h"

#define R 0x00ff0000
#define G 0x0000ff00
#define B 0x000000ff

struct memory;
struct spec_reg;
struct gpu {
	struct memory *memory;
	struct spec_reg *spec_reg;
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	uint32_t color_0;
	uint32_t color_1;
	uint32_t color_2;
	uint32_t color_3;

	/*
	SDL_Window *window_d;
	SDL_Surface *surface_d;
	SDL_Texture *texture_d;
	SDL_Renderer *renderer_d;
	unsigned int *pixels_d;
	*/

	struct save_start save_start;
	uint32_t last_tick;
	uint32_t tick;
	uint32_t pixels[GB_SURF];
	struct save_end save_end;
};

struct ae_config;
void gpu_init(struct gpu *gpu, struct memory *memory,
		struct ae_config *conf);
void gpu_update(struct gpu *gpu, unsigned cycles);
void gpu_cleanup(struct gpu *gpu);

#endif
