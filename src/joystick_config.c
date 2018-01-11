#define _GNU_SOURCE
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "joystick_config.h"
#include "utils.h"
#include "ae_config.h"

static void reset_mappings(struct mapping mappings[MAPPINGS_SIZE])
{
	int i;

	memset(mappings, 0, sizeof(*mappings) * MAPPINGS_SIZE);

	for (i = 0; i <= GB_BUTTON_LAST; i++)
		mappings[i].button = GB_BUTTON_INVALID;
}

static enum control_type control_type_from_string_prefix(const char *str)
{
	enum control_type type;
	const char *type_str;

	for (type = CONTROL_TYPE_FIRST; type <= CONTROL_TYPE_LAST; type++) {
		type_str = control_type_to_str(type);
		if (strncmp(str, type_str, strlen(type_str)) == 0)
			break;
	}

	return type;
}

void close_joystick_config(struct joystick_config *jc)
{
	if (jc == NULL || jc->joystick.joystick == NULL)
		return;

	SDL_JoystickClose(jc->joystick.joystick);
	if (jc->joystick.name == NULL)
		return;

	free(jc->joystick.name);
}

void cleanup_joystick_config(struct joystick_config *joystick_config)
{
	close_joystick_config(joystick_config);
	reset_joystick_config(joystick_config);
}

void reset_joystick_config(struct joystick_config *joystick_config)
{
	memset(joystick_config, 0, sizeof(*joystick_config));
	reset_mappings(joystick_config->mappings);
}

int init_joystick_config(struct joystick_config *joystick_config,
		int index, const char *mappings_dir)
{
	int ret;
	struct joystick *joystick;
	char cleanup(cleanup_string) * canon_name = NULL;

	reset_joystick_config(joystick_config);

	joystick_config->initialized = true;
	joystick = &joystick_config->joystick;
	joystick->index = index;
	joystick->joystick = SDL_JoystickOpen(index);
	if (joystick->joystick == NULL) {
		fprintf(stderr, "SDL_JoystickOpen(%d)\n", index);
		return -EINVAL;
	}
	joystick->name = strdup(SDL_JoystickNameForIndex(index));
	if (joystick->name == NULL)
		return -errno;
	joystick->num.axes = SDL_JoystickNumAxes(joystick->joystick);
	joystick->num.balls = SDL_JoystickNumBalls(joystick->joystick);
	joystick->num.hats = SDL_JoystickNumHats(joystick->joystick);
	joystick->num.buttons = SDL_JoystickNumButtons(joystick->joystick);
	joystick->id = SDL_JoystickInstanceID(joystick->joystick);
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick->joystick),
			joystick->guid, GUID_SIZE);

	if (mappings_dir == NULL)
		return 0;

	canon_name = strdup(joystick->name);
	if (canon_name == NULL)
		return -errno;
	canonicalize_joystick_name(canon_name);
	printf("looking for %s/%s.mapping\n", mappings_dir, canon_name);
	ret = load_mapping(joystick_config->mappings, "%s/%s.mapping",
			mappings_dir, canon_name);
	if (ret != 0)
		return ret;

	printf("loaded mapping successfully\n");

	return 0;
}

/* __attribute__((printf(2, 3))) */ int load_mapping(
		struct mapping mappings[MAPPINGS_SIZE], const char *fmt, ...)
{
	int ret;
	va_list args;
	char cleanup(cleanup_string)*path = NULL;
	FILE cleanup(cleanup_file)*f = NULL;
	struct ae_config cleanup(ae_config_cleanup)config =
			AE_CONFIG_INITIALIZER;
	enum gb_button button;
	const char *button_config;
	enum control_type type;
	struct mapping *mapping;

	va_start(args, fmt);
	ret = vasprintf(&path, fmt, args);
	va_end(args);
	if (ret < 0) {
		path = NULL;
		fprintf(stderr, "vasprintf failed\n");
		return -ENOMEM;
	}
	ret = ae_config_read(&config, path);
	if (ret < 0) {
		fprintf(stderr, "ae_config_read: %s\n", strerror(-ret));
		return ret;
	}

	for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST; button++) {
		button_config = ae_config_get(&config,
				gb_button_to_str(button));
		mapping = mappings + button;
		mapping->button = button;
		type = control_type_from_string_prefix(button_config);
		mapping->control.type = type;
		switch (type) {
		case CONTROL_TYPE_BUTTON:
			sscanf(button_config, "button;%"SCNu8,
					&mapping->control.button.index);
			break;

		case CONTROL_TYPE_AXIS:
			sscanf(button_config, "axis;%"SCNu8";%"SCNi16,
					&mapping->control.axis.index,
					&mapping->control.axis.value);
			break;

		case CONTROL_TYPE_HAT:
			sscanf(button_config, "hat;%"SCNu8";%"SCNi8"\n",
					&mapping->control.hat.index,
					&mapping->control.hat.value);
			break;

		default:
			printf("Type %s not handled yet.\n",
					control_type_to_str(type));
			break;
		}
	}

	return 0;
}

char *canonicalize_joystick_name(char *name)
{
	size_t len;

	len = strlen(name);
	while (len--)
		if (!isalnum(name[len]))
			name[len] = '_';
		else
			name[len] = tolower(name[len]);

	return name;
}

const char *control_type_to_str(enum control_type type)
{
	switch (type) {
	case CONTROL_TYPE_BUTTON:
		return "button";

	case CONTROL_TYPE_AXIS:
		return "axis";

	case CONTROL_TYPE_BALL:
		return "ball";

	case CONTROL_TYPE_HAT:
		return "hat";

	default:
		return "(invalid)";
	}
}

const char *gb_button_to_str(enum gb_button button)
{
	switch (button) {
	case GB_BUTTON_UP:
		return "up";

	case GB_BUTTON_RIGHT:
		return "right";

	case GB_BUTTON_DOWN:
		return "down";

	case GB_BUTTON_LEFT:
		return "left";

	case GB_BUTTON_A:
		return "A";

	case GB_BUTTON_B:
		return "B";

	case GB_BUTTON_START:
		return "start";

	case GB_BUTTON_SELECT:
		return "select";

	default:
		return "(unknown)";
	}
}


