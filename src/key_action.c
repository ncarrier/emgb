#include <stdlib.h>
#include <stdbool.h>

#include "key_action.h"
#include "log.h"
#include "utils.h"

int key_action_register(const struct key_action **key_actions,
		const struct key_action *key_action)
{
	unsigned i;

	if (key_action->sym == SDLK_UNKNOWN && key_action->command == NULL)
		ERR("Either \e[1msym\e[0m or \e[1mcommand\e[0m must be valid.");
	for (i = 0; i < KEY_ACTIONS_MAX; i++)
		if (key_actions[i] == NULL)
			break;

	if (i == KEY_ACTIONS_MAX)
		ERR("No memory left for registering key action %s",
				key_action->command);
	key_actions[i] = key_action;

	return 0;
}

bool key_action_handle(const struct key_action **key_actions, SDL_Keycode sym,
		const char *command)
{
	unsigned i;
	const struct key_action *ka;

	for (i = 0; i < KEY_ACTIONS_MAX; i++) {
		ka = key_actions[i];
		if (ka == NULL)
			break;
		if ((sym != SDLK_UNKNOWN && ka->sym == sym)
				|| str_matches(ka->command, command)) {
			ka->action(ka);
			return true;
		}
	}

	return false;
}
