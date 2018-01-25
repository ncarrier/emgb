#ifndef __JOYPAD__
#define __JOYPAD__

#include <SDL2/SDL_keycode.h>

#include "ae_config.h"

struct gb;
void handleEvent(struct gb *gb_s);
void keyUp(struct gb *gb_s);
void keyDown(struct gb *gb_s);

struct joypad {
	unsigned char button_key; /* = 0x0f */
	unsigned char button_dir; /* = 0x0f */
	SDL_Keycode sym_right;
	SDL_Keycode sym_left;
	SDL_Keycode sym_up;
	SDL_Keycode sym_down;
	SDL_Keycode sym_a;
	SDL_Keycode sym_b;
	SDL_Keycode sym_select;
	SDL_Keycode sym_start;
};

void joypad_init(struct joypad *pad, struct ae_config *config);
unsigned char joypad_get_state(const struct joypad *pad, uint8_t register_p1);

#endif
