#ifndef GB_GPU
#define GB_GPU
#include <stdbool.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "cpu.h"
#include "memory.h"
#include "ae_config.h"

#define R 0x00ff0000
#define G 0x0000ff00
#define B 0x000000ff

enum gpu_mode {
	HBLANK = 0,
	VBLANK = 1,
	OAM = 2,
	VRAM = 3,
};

struct gpu {
	struct cpu *cpu;
	struct memory *memory;
	unsigned last_tick;
	unsigned int tick;
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	unsigned int *pixels;
	bool mouse_visible;

	SDL_Window *window_d;
	SDL_Surface *surface_d;
	SDL_Texture *texture_d;
	SDL_Renderer *renderer_d;
	unsigned int *pixels_d;

	SDL_Event event;
	enum gpu_mode mode;
	int color_0;
	int color_1;
	int color_2;
	int color_3;
};

void gpu_init(struct gpu *gpu, struct cpu *cpu, struct memory *memory,
		struct ae_config *conf);
void gpu_update(struct gpu *gpu);
void gpu_cleanup(struct gpu *gpu);

#endif
