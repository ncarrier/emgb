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

#define cleanup(f) __attribute((cleanup(f)))

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

	CONTROL_TYPE_BALL = CONTROL_TYPE_FIRST,
	CONTROL_TYPE_AXIS,
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

struct button_states {
	union {
		struct {
			bool up;
			bool right;
			bool down;
			bool left;
			bool a;
			bool b;
			bool start;
			bool select;
		};
		bool array[MAPPINGS_SIZE];
	};
};

struct joystick_config {
	bool initialized;
	struct joystick joystick;
	struct button_states buttons;
	struct mapping mappings[MAPPINGS_SIZE];
};

static void close_joystick_config(struct joystick_config *jc)
{
	if (jc == NULL || jc->joystick.joystick == NULL)
		return;

	SDL_JoystickClose(jc->joystick.joystick);
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

#define have_same_sign(n1, n2) (((n1) * (n2)) >= 0)

static bool controls_are_equal(const struct control *c1,
		const struct control *c2)
{
	if (c1->type != c2->type)
		return false;

	switch (c1->type) {
	case CONTROL_TYPE_AXIS:
		return c2->axis.index == c1->axis.index &&
				have_same_sign(c2->axis.value, c1->axis.value);

	case CONTROL_TYPE_BALL:
		return false; /* TODO not handled yet */

	case CONTROL_TYPE_HAT:
		return c2->hat.index == c1->hat.index &&
				have_same_sign(c2->hat.value, c1->hat.value);

	case CONTROL_TYPE_BUTTON:
		return c2->button.index == c1->button.index;

	default:
		return false;
	}
}

static bool check_mapping_is_available(
		const struct mapping mappings[MAPPINGS_SIZE],
		const struct mapping *mapping)
{
	unsigned i;
	const struct mapping *cur;

	for (i = 0; i < MAPPINGS_SIZE; i++) {
		cur = mappings + i;
		if (cur->button == GB_BUTTON_INVALID)
			continue;

		if (controls_are_equal(&mapping->control, &cur->control))
			return false;
	}

	return true;
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

static bool handle_event(struct joystick_config *joystick_config,
		SDL_Event *event, enum gb_button *button)
{
	static int flush = 10;
	int has_event;
	struct joystick *joystick;
	struct mapping mapping;
	struct mapping *mappings;

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

	joystick = &joystick_config->joystick;
	mappings = joystick_config->mappings;

	switch (event->type) {
	case SDL_JOYDEVICEADDED:
		printf("Joystick %"PRIi32" detected.\n", event->jdevice.which);
		return true;

	case SDL_JOYDEVICEREMOVED:
		if (event->jdevice.which == joystick->id)
			error(EXIT_FAILURE, 0, "Joystick removed, aborting\n");
		return true;

	case SDL_JOYBUTTONDOWN:
		if (event->jbutton.which != joystick->id)
			return true;

		mapping.button = *button;
		mapping.control.type = CONTROL_TYPE_BUTTON;
		mapping.control.button.index = event->jbutton.button;
		if (!check_mapping_is_available(mappings, &mapping)) {
			printf("Button %"PRIu8" already assigned.\n",
					event->jbutton.button);
			return true;
		}
		printf("%s mapped to button %"PRIu8"\n",
				gb_button_to_str(*button),
				event->jbutton.button);
		break;

	case SDL_JOYBUTTONUP:
		return true;

	case SDL_JOYAXISMOTION:
		if (event->jaxis.which != joystick->id ||
				event->jaxis.value == 0)
			return true;

		mapping.button = *button;
		mapping.control.type = CONTROL_TYPE_AXIS;
		mapping.control.axis.index = event->jaxis.axis;
		mapping.control.axis.value =
				event->jaxis.value > 0 ? INT16_MAX : INT16_MIN;
		if (!check_mapping_is_available(mappings, &mapping)) {
			printf("Axis %"PRIu8"%s already assigned.\n",
					event->jaxis.axis,
					event->jaxis.value > 0 ? "+" : "-");
			return true;
		}
		printf("%s mapped to axis %"PRIu8", value %"PRIi16"\n",
				gb_button_to_str(*button), event->jaxis.axis,
				event->jaxis.value);
		break;

	case SDL_JOYHATMOTION:
		if (event->jhat.which != joystick->id ||
				event->jhat.value == SDL_HAT_CENTERED)
			return true;

		mapping.button = *button;
		mapping.control.type = CONTROL_TYPE_HAT;
		mapping.control.hat.index = event->jhat.hat;
		mapping.control.hat.value = event->jhat.value;
		if (!check_mapping_is_available(mappings, &mapping)) {
			printf("Hat %"PRIu8"%s already assigned.\n",
					event->jhat.hat,
					event->jhat.value > 0 ? "+" : "-");
			return true;
		}
		printf("%s mapped to hat %"PRIu8", value %"PRIi16"\n",
				gb_button_to_str(*button), event->jhat.hat,
				event->jhat.value);
		break;

	case SDL_QUIT:
		printf("Quitting on user's request.\n");
		return false;

	default:
		printf("Unhandled event %"PRIu8"\n", event->type);
	}

	mappings[*button] = mapping;
	next_button(button);

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

static int write_mapping(const struct joystick_config *joystick_config,
		const char *mappings_dir)
{
	int ret;
	enum gb_button button;
	char cleanup(cleanup_string)*path = NULL;
	char cleanup(cleanup_string)*canon_name = NULL;
	FILE cleanup(cleanup_file)*mapping_file = NULL;
	const struct mapping *mapping;
	const char *type_name;
	const struct joystick *joystick;
	const struct mapping *mappings;

	joystick = &joystick_config->joystick;
	mappings = joystick_config->mappings;
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
	char cleanup(cleanup_string)*path = NULL;
	FILE cleanup(cleanup_file)*f = NULL;
	struct ae_config cleanup(ae_config_cleanup)config = {
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

	return 0;
}

static void reset_mappings(struct mapping mappings[MAPPINGS_SIZE])
{
	int i;

	memset(mappings, 0, sizeof(*mappings) * MAPPINGS_SIZE);

	for (i = 0; i <= GB_BUTTON_LAST; i++)
		mappings[i].button = GB_BUTTON_INVALID;
}

static void reset_joystick_config(struct joystick_config *joystick_config)
{
	memset(joystick_config, 0, sizeof(*joystick_config));
	reset_mappings(joystick_config->mappings);
}

static void cleanup_joystick_config(struct joystick_config *joystick_config)
{
	close_joystick_config(joystick_config);
	reset_joystick_config(joystick_config);
}

static int init_joystick_config(struct joystick_config *joystick_config,
		int index, const char *mappings_dir)
{
	int ret;
	struct joystick *joystick;
	char cleanup(cleanup_string)* canon_name = NULL;

	reset_joystick_config(joystick_config);

	joystick_config->initialized = true;
	joystick = &joystick_config->joystick;
	joystick->index = index;
	joystick->joystick = SDL_JoystickOpen(index);
	if (joystick->joystick == NULL)
		error(EXIT_FAILURE, 0, "SDL_JoystickOpen(%d)", index);
	joystick->name = SDL_JoystickNameForIndex(index);
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

static bool handle_test_event(const char *mappings_dir,
		struct joystick_config *joystick_config)
{
	enum gb_button button;
	int ret;
	bool has_event;
	SDL_Event event;
	const char *joystick_name;
	static struct control *control;

	has_event = SDL_PollEvent(&event);
	if (!has_event)
		return true;

	switch (event.type) {
	case SDL_JOYDEVICEADDED:
		joystick_name = SDL_JoystickNameForIndex(event.jdevice.which);
		printf("Joystick %s (index = %"PRIi32") detected.\n",
				joystick_name, event.jdevice.which);
		if (joystick_config->initialized) {
			printf("Skipped, already using joystick %s\n",
					joystick_config->joystick.name);
			break;
		}
		ret = init_joystick_config(joystick_config, event.jdevice.which,
				mappings_dir);
		if (ret < 0) {
			printf("No mapping found for %s\n", joystick_name);
			cleanup_joystick_config(joystick_config);
		}
		break;

	case SDL_JOYDEVICEREMOVED:
		printf("Joystick SDL_JoystickID = %"PRIi32" removed.%*s\n",
				event.jdevice.which, 25, "");
		if (!joystick_config->initialized)
			break;
		if (joystick_config->joystick.id != event.jdevice.which)
			break;
		printf("Waiting for joystick detection\n");

		cleanup_joystick_config(joystick_config);
		break;

	case SDL_JOYBUTTONDOWN:
		if (!joystick_config->initialized)
			break;
		for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
				button++) {
			control = &joystick_config->mappings[button].control;
			if (control->type != CONTROL_TYPE_BUTTON)
				continue;
			if (control->button.index == event.jbutton.button)
				joystick_config->buttons.array[button] = true;
		}
		break;

	case SDL_JOYBUTTONUP:
		if (!joystick_config->initialized)
			break;
		for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
				button++) {
			control = &joystick_config->mappings[button].control;
			if (control->type != CONTROL_TYPE_BUTTON)
				continue;
			if (control->button.index == event.jbutton.button)
				joystick_config->buttons.array[button] = false;
		}
		break;

	case SDL_JOYAXISMOTION:
		if (!joystick_config->initialized)
			break;
		for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
				button++) {
			control = &joystick_config->mappings[button].control;
			if (control->type != CONTROL_TYPE_AXIS)
				continue;
			if (control->axis.index != event.jaxis.axis)
				continue;
			if (!have_same_sign(event.jaxis.value,
					control->axis.value))
				continue;

			joystick_config->buttons.array[button] = abs(
					event.jaxis.value)
					> abs(control->axis.value) / 2;
		}
		break;

	case SDL_JOYHATMOTION:
		if (!joystick_config->initialized)
			break;
		printf("event %s%*s\n", event_type_to_string(event.type), 50,
						"");
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

static void log_buttons_state(const struct joystick_config *joystick_config)
{
	const struct button_states *s;

	s = &joystick_config->buttons;
	if (joystick_config->initialized)
		printf("up: %d, right %d, down %d, left %d, a %d, b %d, "
				"start %d, select %d\r", s->up, s->right,
				s->down, s->left, s->a, s->b, s->start,
				s->select);
	else
		printf("%*s\r", 50, "");
}

static int test_joypad(const char *mappings_dir)
{
	int ret;
	struct joystick_config joystick_config;

	reset_joystick_config(&joystick_config);

	printf(civis);
	atexit(restore_cursor);
	printf("Waiting for joystick detection\n");
	ret = SDL_Init(SDL_INIT_JOYSTICK);
	if (ret < 0)
		error(EXIT_FAILURE, 0, "SDL_Init(SDL_INIT_JOYSTICK)");
	atexit(SDL_Quit);

	while (handle_test_event(mappings_dir, &joystick_config))
		log_buttons_state(&joystick_config);

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
	SDL_Event event;
	enum gb_button button;
	const char *progname;
	const char *mappings_dir;
	struct joystick_config cleanup(close_joystick_config) joystick_config;

	/* mandatory for attribute cleanup not to segfault in error cases */
	reset_joystick_config(&joystick_config);
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
		value = 0;
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
	}

	ret = init_joystick_config(&joystick_config, value, NULL);
	if (ret < 0)
		error(EXIT_FAILURE, -ret, "init_joystick_config");

	printf("Configuring joystick %d, id %"PRIi32", GUID:%s: %s\n",
			joystick_config.joystick.index,
			joystick_config.joystick.id,
			joystick_config.joystick.guid,
			joystick_config.joystick.name);
	printf("\tAxes: %d\n", joystick_config.joystick.num.axes);
	printf("\tBalls: %d\n", joystick_config.joystick.num.balls);
	printf("\tHats: %d\n", joystick_config.joystick.num.hats);
	printf("\tButtons: %d\n", joystick_config.joystick.num.buttons);

	printf(civis);
	atexit(restore_cursor);
	button = GB_BUTTON_INVALID;
	next_button(&button);
	while (handle_event(&joystick_config, &event, &button))
		;

	/* quit if mapping hasn't completed */
	if (button != GB_BUTTON_INVALID)
		return EXIT_SUCCESS;

	/* otherwise, write to file */
	return write_mapping(&joystick_config, mappings_dir);
}
