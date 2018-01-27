#ifndef __JOYPAD__
#define __JOYPAD__
#include <stdbool.h>

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>

#include "config.h"
#include "special_registers.h"

struct gb;
struct joypad {
	struct config *config;
	struct spec_reg *spec_reg;
	bool running;
	SDL_Event event;
	uint8_t button_key; /* = 0x0f */
	uint8_t button_dir; /* = 0x0f */
	SDL_Keycode sym_right;
	SDL_Keycode sym_left;
	SDL_Keycode sym_up;
	SDL_Keycode sym_down;
	SDL_Keycode sym_a;
	SDL_Keycode sym_b;
	SDL_Keycode sym_select;
	SDL_Keycode sym_start;
};

void joypad_init(struct joypad *joypad, struct config *config,
		struct spec_reg *spec_reg);
unsigned char joypad_get_state(const struct joypad *joypad,
		uint8_t register_p1);
#define joypad_is_running(joypad) ((joypad)->running)
void joypad_handle_event(struct gb *gb);

#endif
