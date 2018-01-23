#ifndef GB_GPU
#define GB_GPU

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "gb.h"
#include "io.h"
#include "ae_config.h"

#define R 0x00ff0000
#define G 0x0000ff00
#define B 0x000000ff

enum gpuMode {
	HBLANK = 0,
	VBLANK = 1,
	OAM = 2,
	VRAM = 3,
};

struct gpu {
	unsigned char scanline;
	unsigned last_tick;
	unsigned int tick;
	SDL_Window *window;
	SDL_Surface *screenSurface;
	SDL_Texture *texture;
	SDL_Renderer *renderer;
	unsigned int *pixels;
	bool mouse_visible;

	SDL_Window *window_d;
	SDL_Surface *screenSurface_d;
	SDL_Texture *texture_d;
	SDL_Renderer *renderer_d;
	unsigned int *pixels_d;

	SDL_Event event;
	enum gpuMode gpuMode;
	int color_0;
	int color_1;
	int color_2;
	int color_3;
};

void gpu_init(struct gpu *gpu, struct ae_config *conf);
void gpu_update(struct gb *gb);
void displayAll(struct gb *gb);
void renderingWindow(struct gb *gb); // TODO remove ?
void renderingSprite(struct gb *gb);
int color_index_to_value(const struct gpu *gpu, int color);

#endif
