#include <unistd.h>

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "memory.h"
#include "joypad.h"

#include "gb.h"
#include "utils.h"
#include "ae_config.h"
#include "config.h"

#define BUTTON_RIGHT_FLAG (1 << 0)
#define BUTTON_LEFT_FLAG (1 << 1)
#define BUTTON_UP_FLAG (1 << 2)
#define BUTTON_DOWN_FLAG (1 << 3)

#define BUTTON_A_FLAG (1 << 0)
#define BUTTON_B_FLAG (1 << 1)
#define BUTTON_SELECT_FLAG (1 << 2)
#define BUTTON_START_FLAG (1 << 3)

#define BUTTON_KEY_OR_DIR_MASK 0x04
#define BUTTON_IS_KEY(b) (!!((b) & BUTTON_KEY_OR_DIR_MASK))
#define BUTTON_TO_KEY(b) (1 << ((b) & ~BUTTON_KEY_OR_DIR_MASK))
#define BUTTON_TO_DIR(b) (1 << (b))

static void get_pad_key_from_config(SDL_Keycode *sym, struct ae_config *config,
		const char *key, SDL_Keycode default_sym)
{
	const char *key_name;
	const char *default_keyname;

	default_keyname = SDL_GetKeyName(default_sym);
	key_name = ae_config_get_default(config, key, default_keyname);
	*sym = SDL_GetKeyFromName(key_name);
	if (*sym == SDLK_UNKNOWN)
		*sym = default_sym;
}

void joypad_init(struct joypad *pad, struct ae_config *config)
{
	pad->button_key = 0x0f;
	pad->button_dir = 0x0f;
	pad->running = true;

	get_pad_key_from_config(&pad->sym_right, config, CONFIG_JOYPAD_0_RIGHT,
			CONFIG_JOYPAD_0_RIGHT_DEFAULT);
	get_pad_key_from_config(&pad->sym_left, config, CONFIG_JOYPAD_0_LEFT,
			CONFIG_JOYPAD_0_LEFT_DEFAULT);
	get_pad_key_from_config(&pad->sym_up, config, CONFIG_JOYPAD_0_UP,
			CONFIG_JOYPAD_0_UP_DEFAULT);
	get_pad_key_from_config(&pad->sym_down, config, CONFIG_JOYPAD_0_DOWN,
			CONFIG_JOYPAD_0_DOWN_DEFAULT);
	get_pad_key_from_config(&pad->sym_a, config, CONFIG_JOYPAD_0_A,
			CONFIG_JOYPAD_0_A_DEFAULT);
	get_pad_key_from_config(&pad->sym_b, config, CONFIG_JOYPAD_0_B,
			CONFIG_JOYPAD_0_B_DEFAULT);
	get_pad_key_from_config(&pad->sym_select, config,
			CONFIG_JOYPAD_0_SELECT, CONFIG_JOYPAD_0_SELECT_DEFAULT);
	get_pad_key_from_config(&pad->sym_start, config, CONFIG_JOYPAD_0_START,
			CONFIG_JOYPAD_0_START_DEFAULT);
}

static void key_down(struct gb *gb)
{
	struct joypad *pad;
	SDL_Keycode sym;
	union SDL_Event *event;
	struct spec_reg *spec_reg;

	spec_reg = &gb->memory.spec_reg;
	pad = &gb->joypad;

	event = &(gb->joypad.event);
	sym = event->key.keysym.sym;
	if (sym == SDLK_ESCAPE) {
		pad->running = false;
	} else if (sym == pad->sym_a) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_key &= ~BUTTON_A_FLAG;
	} else if (sym == pad->sym_b) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_key &= ~BUTTON_B_FLAG;
	} else if (sym == pad->sym_select) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_key &= ~BUTTON_SELECT_FLAG;
	} else if (sym == pad->sym_start) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_key &= ~BUTTON_START_FLAG;
	} else if (sym == pad->sym_down) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_dir &= ~BUTTON_DOWN_FLAG;
	} else if (sym == pad->sym_up) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_dir &= ~BUTTON_UP_FLAG;
	} else if (sym == pad->sym_left) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_dir &= ~BUTTON_LEFT_FLAG;
	} else if (sym == pad->sym_right) {
		spec_reg->ifl |= INT_JOYPAD;
		pad->button_dir &= ~BUTTON_RIGHT_FLAG;
	}
}

static void key_up(struct joypad *joypad)
{
	SDL_Keycode sym;
	union SDL_Event *event;

	event = &joypad->event;
	sym = event->key.keysym.sym;
	if (sym == joypad->sym_a) {
		joypad->button_key |= BUTTON_A_FLAG;
	} else if (sym == joypad->sym_b) {
		joypad->button_key |= BUTTON_B_FLAG;
	} else if (sym == joypad->sym_select) {
		joypad->button_key |= BUTTON_SELECT_FLAG;
	} else if (sym == joypad->sym_start) {
		joypad->button_key |= BUTTON_START_FLAG;
	} else if (sym == joypad->sym_down) {
		joypad->button_dir |= BUTTON_DOWN_FLAG;
	} else if (sym == joypad->sym_up) {
		joypad->button_dir |= BUTTON_UP_FLAG;
	} else if (sym == joypad->sym_left) {
		joypad->button_dir |= BUTTON_LEFT_FLAG;
	} else if (sym == joypad->sym_right) {
		joypad->button_dir |= BUTTON_RIGHT_FLAG;
	}
}

static void joy_device_added(struct gb *gb, uint32_t index)
{
	int ret;
	const char *joystick_name;
	struct joystick_config *joystick_config;

	joystick_config = &gb->joystick_config;
	joystick_name = SDL_JoystickNameForIndex(index);
	printf("Joystick %s (index = %"PRIi32") detected.\n",
			joystick_name, index);
	if (joystick_config->initialized) {
		printf("Skipped, already using joystick %s\n",
				joystick_config->joystick.name);
		return;
	}
	ret = init_joystick_config(joystick_config, index, gb->config.dir);
	if (ret < 0) {
		printf("No mapping for %s, create one with joypad_mapping\n",
				joystick_name);
		cleanup_joystick_config(joystick_config);
	}
}

static void joy_device_removed(struct gb *gb, const union SDL_Event *event)
{
	struct joystick_config *joystick_config;

	joystick_config = &gb->joystick_config;
	printf("Joystick SDL_JoystickID = %"PRIi32" removed.%*s\n",
			event->jdevice.which, 25, "");
	if (!joystick_config->initialized ||
			joystick_config->joystick.id != event->jdevice.which)
		return;
	printf("%s disconnected, waiting for joystick detection\n",
			joystick_config->joystick.name);

	cleanup_joystick_config(joystick_config);

	/* try to reconnect to another joystick if any */
	if (SDL_NumJoysticks() != 0)
		joy_device_added(gb, event->jdevice.which);
}

static void button_down(struct gb *gb, enum gb_button button)
{
	struct memory *memory;

	memory = &gb->memory;
	memory->spec_reg.ifl |= INT_JOYPAD;
	if (BUTTON_IS_KEY(button))
		gb->joypad.button_key &= ~BUTTON_TO_KEY(button);
	else
		gb->joypad.button_dir &= ~BUTTON_TO_DIR(button);
}

static void button_up(struct gb *gb, enum gb_button button)
{
	if (BUTTON_IS_KEY(button))
		gb->joypad.button_key |= BUTTON_TO_KEY(button);
	else
		gb->joypad.button_dir |= BUTTON_TO_DIR(button);
}

static void joy_button_action(struct gb *gb, const union SDL_Event *event,
		void (*action)(struct gb *, enum gb_button))
{
	struct joystick_config *joystick_config;
	enum gb_button button;
	struct control *control;

	joystick_config = &gb->joystick_config;
	if (!joystick_config->initialized)
		return;

	for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
			button++) {
		control = &joystick_config->mappings[button].control;
		if (control->type != CONTROL_TYPE_BUTTON)
			continue;
		if (control->button.index == event->jbutton.button)
			action(gb, button);
	}
}

static void joy_button_down(struct gb *gb, const union SDL_Event *event)
{
	joy_button_action(gb, event, button_down);
}

static void joy_button_up(struct gb *gb, const union SDL_Event *event)
{
	joy_button_action(gb, event, button_up);
}

static void joy_axis_motion(struct gb *gb, const union SDL_Event *event)
{
	struct joystick_config *joystick_config;
	enum gb_button button;
	struct control *control;

	joystick_config = &gb->joystick_config;
	if (!joystick_config->initialized)
		return;

	for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
			button++) {
		control = &joystick_config->mappings[button].control;
		if (control->type != CONTROL_TYPE_AXIS)
			continue;
		if (control->axis.index != event->jaxis.axis)
			continue;
		if (!have_same_sign(event->jaxis.value,
				control->axis.value))
			continue;

		if (abs(event->jaxis.value) > abs(control->axis.value) / 2)
			button_down(gb, button);
		else
			button_up(gb, button);

	}
}

static bool is_fullscreen(const struct SDL_WindowEvent *we)
{
	int ret;
	SDL_DisplayMode dm;
	ret = SDL_GetCurrentDisplayMode(0, &dm);
	if (ret != 0)
		return false;

	return dm.w == we->data1 && dm.h == we->data2;
}

void joypad_handle_event(struct gb *gb)
{
	union SDL_Event *event;
	struct SDL_WindowEvent *we;
	struct ae_config *conf;
	uint32_t width;
	uint32_t height;
	struct joypad *joypad;

	joypad = &gb->joypad;
	event = &joypad->event;
	if (SDL_PollEvent(event) == 0)
		return;

	conf = &gb->config.config;
	switch (event->type) {
	case SDL_QUIT: {
		printf("see u.\n");
		joypad->running = false;
		config_write(&gb->config);
		break;
	}

	case SDL_WINDOWEVENT:
		we = &event->window;
		switch (we->event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			if (is_fullscreen(we)) {
				if (gb->gpu.mouse_visible) {
					SDL_ShowCursor(SDL_DISABLE);
					gb->gpu.mouse_visible = false;
				}
			} else {
				if (!gb->gpu.mouse_visible) {
					SDL_ShowCursor(SDL_ENABLE);
					gb->gpu.mouse_visible = true;
				}
			}
			/* TODO min and max */
			height = we->data2;
			if (height < GB_H)
				height = GB_H;
			width = we->data1;
			if (width < GB_W)
				width = GB_W;
			ae_config_add_int(conf, "window_width", width);
			ae_config_add_int(conf, "window_height", height);
			config_write(&gb->config);

			break;
		case SDL_WINDOWEVENT_MOVED:
			ae_config_add_int(conf, "window_x", we->data1);
			ae_config_add_int(conf, "window_y", we->data2);
			config_write(&gb->config);

			break;
		}
		break;

	case SDL_JOYDEVICEADDED:
		joy_device_added(gb, event->jdevice.which);
		break;

	case SDL_JOYDEVICEREMOVED:
		joy_device_removed(gb, event);
		break;

	case SDL_JOYBUTTONDOWN:
		joy_button_down(gb, event);
		break;

	case SDL_JOYBUTTONUP:
		joy_button_up(gb, event);
		break;

	case SDL_JOYAXISMOTION:
		joy_axis_motion(gb, event);
		break;

	case SDL_KEYDOWN:
		key_down(gb);
		break;
	case SDL_KEYUP:
		key_up(joypad);
		break;
	}
}

unsigned char joypad_get_state(const struct joypad *pad, uint8_t register_p1)
{
	if ((register_p1 & 0x20) == 0)
		return 0xc0 | pad->button_key | 0x10;
	else if ((register_p1 & 0x10) == 0)
		return 0xc0 | pad->button_dir | 0x20;

	return 0xff;
}
