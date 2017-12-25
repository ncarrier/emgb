#define _GNU_SOURCE
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <error.h>
#include <stdbool.h>
#include <errno.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include "ae_config.h"

#define MAX_JOYSTICKS 10
#define INPUT_LENGTH 4
#define MAPPINGS_SIZE 8
#define GUID_SIZE 33

struct joystick {
	SDL_Joystick *joystick;
	const char *name;
	char guid[GUID_SIZE];
	int index;
	SDL_JoystickID id;
	struct {
		int axes;
		int balls;
		int hats;
		int buttons;
	} num;
};

enum control_type {
	CONTROL_TYPE_FIRST,

	CONTROL_TYPE_AXIS = CONTROL_TYPE_FIRST,
	CONTROL_TYPE_BALL,
	CONTROL_TYPE_HAT,
	CONTROL_TYPE_BUTTON,

	CONTROL_TYPE_LAST = CONTROL_TYPE_BUTTON,
	CONTROL_TYPE_INVALID
};

enum gb_button {
	GB_BUTTON_FIRST,

	GB_BUTTON_UP = GB_BUTTON_FIRST,
	GB_BUTTON_RIGHT,
	GB_BUTTON_DOWN,
	GB_BUTTON_LEFT,
	GB_BUTTON_A,
	GB_BUTTON_B,
	GB_BUTTON_START,
	GB_BUTTON_SELECT,

	GB_BUTTON_LAST = GB_BUTTON_SELECT,
	GB_BUTTON_INVALID
};

struct control {
	enum control_type type;
	union {
		struct {
			uint8_t index;
			int16_t value;
		} axis;
		struct {
			uint8_t index;
			uint8_t value;
		} hat;
		struct {
			uint8_t index;
		} button;
	};
};

struct mapping {
	enum gb_button button;
	struct control control;
};

static void close_joystick(struct joystick *joystick)
{
	if (joystick == NULL || joystick->joystick == NULL)
		return;

	SDL_JoystickClose(joystick->joystick);
}

static const char *gb_button_to_str(enum gb_button button)
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

static void next_button(enum gb_button *button)
{
	if (*button == GB_BUTTON_INVALID)
		*button = GB_BUTTON_FIRST;
	else
		(*button)++;

	if (*button != GB_BUTTON_INVALID) {
		printf("Configuring button %s: ", gb_button_to_str(*button));
		fflush(stdout);
	}
}

static const char *event_type_to_string(uint32_t type)
{
	switch (type) {
	case SDL_QUIT:
		return "SDL_QUIT";
	case SDL_APP_TERMINATING:
		return "SDL_APP_TERMINATING";
	case SDL_APP_LOWMEMORY:
		return "SDL_APP_LOWMEMORY";
	case SDL_APP_WILLENTERBACKGROUND:
		return "SDL_APP_WILLENTERBACKGROUND";
	case SDL_APP_DIDENTERBACKGROUND:
		return "SDL_APP_DIDENTERBACKGROUND";
	case SDL_APP_WILLENTERFOREGROUND:
		return "SDL_APP_WILLENTERFOREGROUND";
	case SDL_APP_DIDENTERFOREGROUND:
		return "SDL_APP_DIDENTERFOREGROUND";
	case SDL_WINDOWEVENT:
		return "SDL_WINDOWEVENT";
	case SDL_SYSWMEVENT:
		return "SDL_SYSWMEVENT";
	case SDL_KEYDOWN:
		return "SDL_KEYDOWN";
	case SDL_KEYUP:
		return "SDL_KEYUP";
	case SDL_TEXTEDITING:
		return "SDL_TEXTEDITING";
	case SDL_TEXTINPUT:
		return "SDL_TEXTINPUT";
	case SDL_KEYMAPCHANGED:
		return "SDL_KEYMAPCHANGED";
	case SDL_MOUSEMOTION:
		return "SDL_MOUSEMOTION";
	case SDL_MOUSEBUTTONDOWN:
		return "SDL_MOUSEBUTTONDOWN";
	case SDL_MOUSEBUTTONUP:
		return "SDL_MOUSEBUTTONUP";
	case SDL_MOUSEWHEEL:
		return "SDL_MOUSEWHEEL";
	case SDL_JOYAXISMOTION:
		return "SDL_JOYAXISMOTION";
	case SDL_JOYBALLMOTION:
		return "SDL_JOYBALLMOTION";
	case SDL_JOYHATMOTION:
		return "SDL_JOYHATMOTION";
	case SDL_JOYBUTTONDOWN:
		return "SDL_JOYBUTTONDOWN";
	case SDL_JOYBUTTONUP:
		return "SDL_JOYBUTTONUP";
	case SDL_JOYDEVICEADDED:
		return "SDL_JOYDEVICEADDED";
	case SDL_JOYDEVICEREMOVED:
		return "SDL_JOYDEVICEREMOVED";
	case SDL_CONTROLLERAXISMOTION:
		return "SDL_CONTROLLERAXISMOTION";
	case SDL_CONTROLLERBUTTONDOWN:
		return "SDL_CONTROLLERBUTTONDOWN";
	case SDL_CONTROLLERBUTTONUP:
		return "SDL_CONTROLLERBUTTONUP";
	case SDL_CONTROLLERDEVICEADDED:
		return "SDL_CONTROLLERDEVICEADDED";
	case SDL_CONTROLLERDEVICEREMOVED:
		return "SDL_CONTROLLERDEVICEREMOVED";
	case SDL_CONTROLLERDEVICEREMAPPED:
		return "SDL_CONTROLLERDEVICEREMAPPED";
	case SDL_FINGERDOWN:
		return "SDL_FINGERDOWN";
	case SDL_FINGERUP:
		return "SDL_FINGERUP";
	case SDL_FINGERMOTION:
		return "SDL_FINGERMOTION";
	case SDL_DOLLARGESTURE:
		return "SDL_DOLLARGESTURE";
	case SDL_DOLLARRECORD:
		return "SDL_DOLLARRECORD";
	case SDL_MULTIGESTURE:
		return "SDL_MULTIGESTURE";
	case SDL_CLIPBOARDUPDATE:
		return "SDL_CLIPBOARDUPDATE";
	case SDL_DROPFILE:
		return "SDL_DROPFILE";
	case SDL_DROPTEXT:
		return "SDL_DROPTEXT";
	case SDL_DROPBEGIN:
		return "SDL_DROPBEGIN";
	case SDL_DROPCOMPLETE:
		return "SDL_DROPCOMPLETE";
	case SDL_AUDIODEVICEADDED:
		return "SDL_AUDIODEVICEADDED";
	case SDL_AUDIODEVICEREMOVED:
		return "SDL_AUDIODEVICEREMOVED";
	case SDL_RENDER_TARGETS_RESET:
		return "SDL_RENDER_TARGETS_RESET";
	case SDL_RENDER_DEVICE_RESET:
		return "SDL_RENDER_DEVICE_RESET";
	case SDL_USEREVENT:
		return "SDL_USEREVENT";
	case SDL_LASTEVENT:
		return "SDL_LASTEVENT";
	default:
		return "(unknown)";
	}
}

static bool handle_event(SDL_Event *event, struct mapping *mapping,
		enum gb_button *button, struct joystick *joystick)
{
	static int flush = 10;
	int has_event;

	has_event = SDL_PollEvent(event);
	/*
	 * some spurious events are generated at startup, don't know why, but by
	 * flushing them, things seem to work
	 */
	if (flush != 0) {
		flush--;
		return true;
	}
	if (has_event == 0)
		return true;

	switch (event->type) {
	case SDL_JOYDEVICEADDED:
		printf("Joystick %"PRIi32" detected.\n", event->jdevice.which);
		break;

	case SDL_JOYDEVICEREMOVED:
		if (event->jdevice.which == joystick->id)
			error(EXIT_FAILURE, 0, "Joystick removed, aborting\n");
		break;

	case SDL_JOYBUTTONDOWN:
		if (event->jbutton.which != joystick->id)
			return true;

		printf("%s mapped to button %"PRIu8"\n",
				gb_button_to_str(*button),
				event->jbutton.button);
		mapping->button = *button;
		mapping->control.type = CONTROL_TYPE_BUTTON;
		mapping->control.button.index = event->jbutton.button;
		next_button(button);
		break;

	case SDL_JOYBUTTONUP:
		return true;

	case SDL_JOYAXISMOTION:
		if (event->jaxis.which != joystick->id ||
				event->jaxis.value == 0)
			return true;

		printf("%s mapped to axis %"PRIu8", value %"PRIi16"\n",
				gb_button_to_str(*button), event->jaxis.axis,
				event->jaxis.value);
		mapping->button = *button;
		mapping->control.type = CONTROL_TYPE_AXIS;
		mapping->control.axis.index = event->jaxis.axis;
		mapping->control.axis.value = event->jaxis.value;
		next_button(button);
		break;

	case SDL_JOYHATMOTION:
		if (event->jhat.which != joystick->id ||
				event->jhat.value == SDL_HAT_CENTERED)
			return true;

		printf("%s mapped to hat %"PRIu8", value %"PRIi16"\n",
				gb_button_to_str(*button), event->jhat.hat,
				event->jhat.value);
		mapping->button = *button;
		mapping->control.type = CONTROL_TYPE_HAT;
		mapping->control.hat.index = event->jhat.hat;
		mapping->control.hat.value = event->jhat.value;
		next_button(button);
		break;

	case SDL_QUIT:
		printf("Quitting on user's request.\n");
		return false;

	default:
		printf("Unhandled event %"PRIu8"\n", event->type);
	}

	return *button != GB_BUTTON_INVALID;
}

static void cleanup_file(FILE **pfile)
{
	if (pfile == NULL || *pfile == NULL)
		return;

	fclose(*pfile);
	*pfile = NULL;
}

static void cleanup_string(char **str)
{
	if (str == NULL || *str == NULL)
		return;

	free(*str);
	*str = NULL;
}

static const char *control_type_to_str(enum control_type type)
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

static char *canonicalize_joystick_name(char *name)
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

static int write_mapping(struct mapping mappings[MAPPINGS_SIZE],
		const char *mappings_dir, const struct joystick *joystick)
{
	int ret;
	enum gb_button button;
	char __attribute__((cleanup(cleanup_string)))*path = NULL;
	char __attribute__((cleanup(cleanup_string)))*canon_name = NULL;
	FILE __attribute__((cleanup(cleanup_file)))*mapping_file = NULL;
	struct mapping *mapping;
	const char *type_name;

	canon_name = strdup(joystick->name);
	if (canon_name == NULL) {
		perror("strdup");
		return EXIT_FAILURE;
	}
	canonicalize_joystick_name(canon_name);
	ret = asprintf(&path, "%s/%s.mapping", mappings_dir, canon_name);
	if (ret < 0) {
		fprintf(stderr, "asprintf failed\n");
		return EXIT_FAILURE;
	}

	mapping_file = fopen(path, "wbe");
	if (mapping_file == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	fprintf(mapping_file, "name=%s\n", joystick->name);
	fprintf(mapping_file, "guid=%s\n", joystick->guid);
	for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST; button++) {
		mapping = mappings + button;
		type_name = control_type_to_str(mapping->control.type);
		fprintf(mapping_file, "%s=%s;", gb_button_to_str(button),
				type_name);
		switch (mapping->control.type) {
		case CONTROL_TYPE_BUTTON:
			fprintf(mapping_file, "%"PRIu8"\n",
					mapping->control.button.index);
			break;

		case CONTROL_TYPE_AXIS:
			fprintf(mapping_file, "%"PRIu8";%"PRIi16"\n",
					mapping->control.axis.index,
					mapping->control.axis.value);
			break;

		case CONTROL_TYPE_HAT:
			fprintf(mapping_file, "%"PRIu8";%"PRIi8"\n",
					mapping->control.hat.index,
					mapping->control.hat.value);
			break;

		default:
			fprintf(stderr, "Unhandled control type %s\n",
					type_name);
		}
	}

	return EXIT_SUCCESS;
}

const char civis[] = { 0x1b, 0x5b, 0x3f, 0x32, 0x35, 0x6c, 0};
const char cnorm[] = { 0x1b, 0x5b, 0x3f, 0x31, 0x32, 0x6c, 0x1b, 0x5b, 0x3f,
		0x32, 0x35, 0x68, 0 };

static void restore_cursor(void)
{
	printf(cnorm);
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

static /* __attribute__((printf(2, 3))) */ int load_mapping(
		struct mapping mappings[MAPPINGS_SIZE], const char *fmt, ...)
{
	int ret;
	va_list args;
	char __attribute__((cleanup(cleanup_string)))*path = NULL;
	FILE __attribute__((cleanup(cleanup_file)))*f = NULL;
	struct ae_config __attribute__((cleanup(ae_config_cleanup)))config = {
			.argz = NULL,
			.len = 0,
	};
	enum gb_button button;
	const char *button_config;
	enum control_type type;
	struct mapping *mapping;

	va_start(args, fmt);
	ret = vasprintf(&path, fmt, args);
	va_end(args);
	if (ret < 0) {
		path = NULL;
		error(EXIT_FAILURE, 0, "vasprintf");
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

	struct joystick joystick = {
			.name = "dummy",
			.guid = {
					[0] = '\0'
			},
	};
	write_mapping(mappings, ".", &joystick);
	return 0;
}

static bool handle_test_event(const char *mappings_dir,
		struct mapping mappings[MAPPINGS_SIZE])
{
	int ret;
	bool has_event;
	SDL_Event event;
	const char *joystick_name;
	char __attribute__((cleanup(cleanup_string)))*canon_name = NULL;

	has_event = SDL_PollEvent(&event);
	if (!has_event)
		return true;

	switch (event.type) {
	case SDL_JOYDEVICEADDED:
		joystick_name = SDL_JoystickNameForIndex(event.jdevice.which);
		canon_name = strdup(joystick_name);
		if (canon_name == NULL)
			error(EXIT_FAILURE, errno, "strdup");
		printf("Joystick %s (index = %"PRIi32") detected.\n",
				canon_name, event.jdevice.which);
		canonicalize_joystick_name(canon_name);
		printf("looking for %s/%s.mapping\n", mappings_dir, canon_name);
		ret = load_mapping(mappings, "%s/%s.mapping", mappings_dir,
				canon_name);
		if (ret == 0) {
			printf("loaded mapping successfully\n");
		} else {
			printf("No mapping found for %s\n", joystick_name);
		}
		break;

	case SDL_JOYDEVICEREMOVED:
		printf("Joystick id = %"PRIi32" removed.\n",
				event.jdevice.which);
		break;

	case SDL_JOYBUTTONDOWN:
		break;

	case SDL_JOYBUTTONUP:
		return true;

	case SDL_JOYAXISMOTION:
		break;

	case SDL_JOYHATMOTION:
		break;

	case SDL_QUIT:
		printf("Quitting on user's request.\n");
		return false;

	default:
		printf("Unhandled event %s\n",
				event_type_to_string(event.type));
	}

	return true;
}

static int test_joypad(const char *mappings_dir)
{
	int i;
	int ret;
	struct mapping mappings[MAPPINGS_SIZE];

	for (i = 0; i <= GB_BUTTON_LAST; i++)
		mappings[i].button = GB_BUTTON_INVALID;
	printf("Waiting for joystick detection\n");
	ret = SDL_Init(SDL_INIT_JOYSTICK);
	if (ret < 0)
		error(EXIT_FAILURE, 0, "SDL_Init(SDL_INIT_JOYSTICK)");
	atexit(SDL_Quit);

	while (handle_test_event(mappings_dir, mappings))
		;

	return EXIT_SUCCESS;
}

__attribute__((noreturn))
static void usage(const char *progname)
{
	error(EXIT_FAILURE, 0, "usage: %s [--test] mappings_dir", progname);

	/* useless but otherwise, it seems gcc can't detect we don't return */
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int i;
	int ret;
	int num_joysticks;
	char input[INPUT_LENGTH];
	long value;
	char *endptr;
	struct joystick __attribute__((cleanup(close_joystick))) joystick = {
			.joystick = NULL
	};
	SDL_Event event;
	enum gb_button button;
	struct mapping mappings[MAPPINGS_SIZE];
	const char *progname;
	const char *mappings_dir;

	progname = basename(argv[0]);
	printf("%s[%jd] starting\n", progname, (intmax_t)getpid());
	if (argc == 3) {
		if (strcmp(argv[1], "--test") != 0)
			usage(progname);

		mappings_dir = argv[2];
		return test_joypad(mappings_dir);
	}
	if (argc != 2 || strcmp(argv[1], "--test") == 0)
		usage(progname);

	mappings_dir = argv[1];

	ret = SDL_Init(SDL_INIT_JOYSTICK);
	if (ret < 0)
		error(EXIT_FAILURE, 0, "SDL_Init(SDL_INIT_JOYSTICK)");
	atexit(SDL_Quit);

	num_joysticks = SDL_NumJoysticks();
	if (num_joysticks == 0)
		error(EXIT_FAILURE, 0, "No joystick detected, aborting");

	printf("%d joysticks detected\n", num_joysticks);

	for (i = 0; i < num_joysticks; i++)
		printf("Joystick %d's name is \"%s\"\n", i,
				SDL_JoystickNameForIndex(i));

	if (num_joysticks == 1) {
		joystick.index = 0;
	} else {
		printf("choose which one you want to configure [0 - %d]:\n > ",
				num_joysticks - 1);
		if (fgets(input, INPUT_LENGTH, stdin) == NULL)
			error(EXIT_FAILURE, 0, "Short input, aborting");

		/* right strip white spaces (especially, newlines) */
		i = strlen(input) - 1;
		while (isspace(input[i]))
			input[i--] = '\0';

		value = strtol(input, &endptr, 10);
		if (*endptr != '\0')
			error(EXIT_FAILURE, 0, "Invalid input \"%s\", aborting",
					input);
		if (value < 0 || value >= num_joysticks)
			error(EXIT_FAILURE, 0,
					"Index %ld out of range, aborting",
					value);
		joystick.index = value;
	}

	joystick.joystick = SDL_JoystickOpen(joystick.index);
	if (joystick.joystick == NULL)
		error(EXIT_FAILURE, 0, "SDL_JoystickOpen(%d)", joystick.index);
	printf("SDL_JoystickGetAttached(%d) = %d\n", joystick.index,
			SDL_JoystickGetAttached(joystick.joystick));
	joystick.name = SDL_JoystickNameForIndex(joystick.index);
	joystick.num.axes = SDL_JoystickNumAxes(joystick.joystick);
	joystick.num.balls = SDL_JoystickNumBalls(joystick.joystick);
	joystick.num.hats = SDL_JoystickNumHats(joystick.joystick);
	joystick.num.buttons = SDL_JoystickNumButtons(joystick.joystick);
	joystick.id = SDL_JoystickInstanceID(joystick.joystick);
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick.joystick),
			joystick.guid, GUID_SIZE);

	printf("Configuring joystick %d, id %"PRIi32", GUID:%s: %s\n",
			joystick.index, joystick.id, joystick.guid,
			joystick.name);
	printf("\tAxes: %d\n", joystick.num.axes);
	printf("\tBalls: %d\n", joystick.num.balls);
	printf("\tHats: %d\n", joystick.num.hats);
	printf("\tButtons: %d\n", joystick.num.buttons);

	printf(civis);
	atexit(restore_cursor);
	button = GB_BUTTON_INVALID;
	next_button(&button);
	while (handle_event(&event, mappings + button, &button, &joystick))
		;

	/* quit if mapping hasn't completed */
	if (button != GB_BUTTON_INVALID)
		return EXIT_SUCCESS;

	/* otherwise, write to file */
	return write_mapping(mappings, mappings_dir, &joystick);
}
