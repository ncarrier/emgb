#ifndef INCLUDE_JOYSTICK_CONFIG_H_
#define INCLUDE_JOYSTICK_CONFIG_H_
#include <stdbool.h>
#include <inttypes.h>

#include <SDL2/SDL_joystick.h>

#define MAX_JOYSTICKS 10
#define MAPPINGS_SIZE 8
#define GUID_SIZE 33

struct joystick {
	SDL_Joystick *joystick;
	char *name;
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

/* values must stay 0 -> 3 for right -> down and 0 + 4 -> 3 + 4 for others */
enum gb_button {
	GB_BUTTON_FIRST = 0,

	GB_BUTTON_RIGHT = GB_BUTTON_FIRST,
	GB_BUTTON_LEFT,
	GB_BUTTON_UP,
	GB_BUTTON_DOWN,
	GB_BUTTON_A,
	GB_BUTTON_B,
	GB_BUTTON_SELECT,
	GB_BUTTON_START,

	GB_BUTTON_LAST = GB_BUTTON_START,
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

void close_joystick_config(struct joystick_config *jc);
void cleanup_joystick_config(struct joystick_config *joystick_config);
void reset_joystick_config(struct joystick_config *joystick_config);
int init_joystick_config(struct joystick_config *joystick_config,
		int index, const char *mappings_dir);
/* __attribute__((printf(2, 3))) */
int load_mapping(struct mapping mappings[MAPPINGS_SIZE], const char *fmt, ...);
char *canonicalize_joystick_name(char *name);
const char *control_type_to_str(enum control_type type);
const char *gb_button_to_str(enum gb_button button);

#endif /* INCLUDE_JOYSTICK_CONFIG_H_ */
