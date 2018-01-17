#ifndef GB_GPU
#define GB_GPU

#include <stdbool.h>

#include "io.h"
#include "GB.h"

#define R 0x00ff0000
#define G 0x0000ff00
#define B 0x000000ff

int color_index_to_value(int color);
void rendering(struct s_gb *gb);
void initGpu(struct s_gb *gb);
char lcdIsEnable(unsigned char lcdc);
void setLcdStatus(struct s_gb *gb);
void updateGpu(struct s_gb *gb);
void initDisplay(struct s_gb *gb);
void renderingBg(struct s_gb *s_gb);
void displayAll(struct s_gb *gb);
void renderingWindow(struct s_gb *gb);
void renderingSprite(struct s_gb *gb);

enum gpuMode {
	HBLANK = 0,
	VBLANK = 1,
	OAM = 2,
	VRAM = 3,
};

struct s_gpu {
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
};

#endif
