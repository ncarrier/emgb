#ifndef INCLUDE_KEY_ACTION_H_
#define INCLUDE_KEY_ACTION_H_

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>

struct key_action {
	const char *command;
	const char *help;
	SDL_Keycode sym;
	void (*action)(const struct key_action *key_action);
};

int key_action_register(const struct key_action **key_actions,
		const struct key_action *key_action);
/*
 * Executes the action matching either sym or command, if found in the
 * key_actions array.
 * sym takes precedence over command.
 * returns true if key was handled, false otherwise.
 */
bool key_action_handle(const struct key_action **key_actions, SDL_Keycode sym,
		const char *command);

#endif /* INCLUDE_KEY_ACTION_H_ */
