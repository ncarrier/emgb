#ifndef GB_GPU
#define GB_GPU
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#define R 0x00ff0000
#define G 0x0000ff00
#define B 0x000000ff

enum gpu_mode {
	HBLANK = 0,
	VBLANK = 1,
	OAM = 2,
	VRAM = 3,
};

struct cpu;
struct memory;
struct gpu {
	struct cpu *cpu;
	struct memory *memory;
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

	/* serialized fields */
	uint32_t last_tick;
	uint32_t tick;
	uint32_t *pixels;
	enum gpu_mode mode;
};

struct ae_config;
void gpu_init(struct gpu *gpu, struct cpu *cpu, struct memory *memory,
		struct ae_config *conf);
void gpu_update(struct gpu *gpu);
int gpu_save(const struct gpu *gpu, FILE *f);
void gpu_cleanup(struct gpu *gpu);

#endif
