#ifndef __JOYPAD__
#define __JOYPAD__
#include <stdbool.h>

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>

#include "save.h"

#define KEY_ACTIONS_MAX 2

struct key_action;
struct config;
struct spec_reg;
struct joystick_config;
struct joypad {
	struct config *config;
	struct spec_reg *spec_reg;
	struct joystick_config *joystick_config;
	SDL_Event event;
	SDL_Keycode sym_right;
	SDL_Keycode sym_left;
	SDL_Keycode sym_up;
	SDL_Keycode sym_down;
	SDL_Keycode sym_a;
	SDL_Keycode sym_b;
	SDL_Keycode sym_select;
	SDL_Keycode sym_start;
	const struct key_action *key_action[KEY_ACTIONS_MAX];

	struct save_start save_start;
	bool running;
	bool mouse_visible;
	uint8_t button_key; /* = 0x0f */
	uint8_t button_dir; /* = 0x0f */
	struct save_end save_end;
};

void joypad_reset(struct joypad *joypad);
void joypad_init(struct joypad *joypad, struct config *config,
		struct spec_reg *spec_reg,
		struct joystick_config *joystick_config);
int joypad_register_key_action(struct joypad *joypad,
		const struct key_action *key_action);
#define joypad_is_running(joypad) ((joypad)->running)
void joypad_handle_event(struct joypad *joypad);

#endif
