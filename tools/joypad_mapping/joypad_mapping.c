#define _GNU_SOURCE
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <error.h>
#include <stdbool.h>

#include <SDL/SDL.h>

#define MAX_JOYSTICKS 10
#define INPUT_LENGTH 4
#define MAPPINGS_SIZE 8

struct joystick {
	SDL_Joystick *joystick;
	const char *name;
	int index;
	struct {
		int axes;
		int balls;
		int hats;
		int buttons;
	} num;
};

enum control_type {
	CONTROL_TYPE_AXIS,
	CONTROL_TYPE_BALL,
	CONTROL_TYPE_HAT,
	CONTROL_TYPE_BUTTON,
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

	if (*button != GB_BUTTON_INVALID)
		printf("Configuring button %s\n", gb_button_to_str(*button));
}

static bool handle_event(SDL_Event *event, struct mapping *mapping,
		enum gb_button *button, int joystick)
{
	if (SDL_PollEvent(event) == 0)
		return true;

	switch (event->type) {
	case SDL_JOYBUTTONDOWN:
		if (event->jbutton.which != joystick)
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
		if (event->jaxis.which != joystick || event->jaxis.value == 0)
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
		if (event->jhat.which != joystick ||
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
		const char *mappings_dir, const char *name)
{
	int ret;
	enum gb_button button;
	char __attribute__((cleanup(cleanup_string)))*path = NULL;
	char __attribute__((cleanup(cleanup_string)))*canon_name = NULL;
	FILE __attribute__((cleanup(cleanup_file)))*mapping_file = NULL;
	struct mapping *mapping;
	const char *type_name;

	canon_name = strdup(name);
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

	fprintf(mapping_file, "name=%s\n", name);
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
			fprintf(mapping_file, "%"PRIu8";%"PRIi16"\n",
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
	if (argc != 2)
		error(EXIT_FAILURE, 0, "usage: %s mappings_dir", progname);
	mappings_dir = argv[1];

	printf("%s[%jd] starting\n", progname, (intmax_t)getpid());
	ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0;
	if (ret < 0)
		error(EXIT_FAILURE, 0, "SDL_Init(SDL_INIT_JOYSTICK)");
	atexit(SDL_Quit);
	SDL_JoystickEventState(SDL_ENABLE);

	num_joysticks = SDL_NumJoysticks();
	if (num_joysticks == 0)
		error(EXIT_FAILURE, 0, "No joystick detected, aborting");

	printf("%d joysticks detected\n", num_joysticks);

	for (i = 0; i < num_joysticks; i++)
		printf("Joystick %d's name is \"%s\"\n", i,
				SDL_JoystickName(i));

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
	joystick.name = SDL_JoystickName(joystick.index);
	joystick.num.axes = SDL_JoystickNumAxes(joystick.joystick);
	joystick.num.balls = SDL_JoystickNumBalls(joystick.joystick);
	joystick.num.hats = SDL_JoystickNumHats(joystick.joystick);
	joystick.num.buttons = SDL_JoystickNumButtons(joystick.joystick);

	printf("Configuring joystick %d: %s\n", joystick.index, joystick.name);
	printf("\tAxes: %d\n", joystick.num.axes);
	printf("\tBalls: %d\n", joystick.num.balls);
	printf("\tHats: %d\n", joystick.num.hats);
	printf("\tButtons: %d\n", joystick.num.buttons);

	button = GB_BUTTON_INVALID;
	next_button(&button);
	while (handle_event(&event, mappings + button, &button, joystick.index))
		;

	/* quit if mapping hasn't completed */
	if (button != GB_BUTTON_INVALID)
		return EXIT_SUCCESS;

	/* otherwise, write to file */
	return write_mapping(mappings, mappings_dir, joystick.name);
}
