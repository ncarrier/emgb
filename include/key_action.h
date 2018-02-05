#ifndef INCLUDE_KEY_ACTION_H_
#define INCLUDE_KEY_ACTION_H_

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>

struct key_action {
	const char *command;
	SDL_Keycode sym;
	void (*action)(const struct key_action *key_action);
};

#endif /* INCLUDE_KEY_ACTION_H_ */
