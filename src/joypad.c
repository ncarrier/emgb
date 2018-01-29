#include <unistd.h>
#include <errno.h>

#include "log.h"
#include "joypad.h"
#include "special_registers.h"
#include "joystick_config.h"
#include "memory.h"
#include "interrupt.h"
#include "video_common.h"
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

void joypad_init(struct joypad *pad, struct config *config,
		struct spec_reg *spec_reg,
		struct joystick_config *joystick_config)
{
	int width;
	int height;
	struct ae_config *aec;

	aec = &config->config;
	pad->config = config;
	pad->spec_reg = spec_reg;
	pad->joystick_config = joystick_config;
	pad->running = true;
	pad->button_key = 0x0f;
	pad->button_dir = 0x0f;

	width = ae_config_get_int(aec, CONFIG_WINDOW_WIDTH,
			CONFIG_WINDOW_WIDTH_DEFAULT);
	height = ae_config_get_int(aec, CONFIG_WINDOW_HEIGHT,
			CONFIG_WINDOW_HEIGHT_DEFAULT);
	pad->mouse_visible = !is_window_fullscreen(width, height);

	get_pad_key_from_config(&pad->sym_right, aec, CONFIG_JOYPAD_0_RIGHT,
			CONFIG_JOYPAD_0_RIGHT_DEFAULT);
	get_pad_key_from_config(&pad->sym_left, aec, CONFIG_JOYPAD_0_LEFT,
			CONFIG_JOYPAD_0_LEFT_DEFAULT);
	get_pad_key_from_config(&pad->sym_up, aec, CONFIG_JOYPAD_0_UP,
			CONFIG_JOYPAD_0_UP_DEFAULT);
	get_pad_key_from_config(&pad->sym_down, aec, CONFIG_JOYPAD_0_DOWN,
			CONFIG_JOYPAD_0_DOWN_DEFAULT);
	get_pad_key_from_config(&pad->sym_a, aec, CONFIG_JOYPAD_0_A,
			CONFIG_JOYPAD_0_A_DEFAULT);
	get_pad_key_from_config(&pad->sym_b, aec, CONFIG_JOYPAD_0_B,
			CONFIG_JOYPAD_0_B_DEFAULT);
	get_pad_key_from_config(&pad->sym_select, aec, CONFIG_JOYPAD_0_SELECT,
			CONFIG_JOYPAD_0_SELECT_DEFAULT);
	get_pad_key_from_config(&pad->sym_start, aec, CONFIG_JOYPAD_0_START,
			CONFIG_JOYPAD_0_START_DEFAULT);
}

int joypad_register_key_op(struct joypad *joypad, const struct key_op *key_op)
{
	unsigned i;

	for (i = 0; i < KEY_OP_MAX; i++)
		if (joypad->key_op[i] == NULL)
			break;

	if (i == KEY_OP_MAX)
		ERR("No memory left for registering key op %s",
				SDL_GetKeyName(key_op->sym));
	joypad->key_op[i] = key_op;

	return 0;
}

static void key_down(struct joypad *joypad)
{
	SDL_Keycode sym;
	struct spec_reg *spec_reg;

	spec_reg = joypad->spec_reg;
	sym = joypad->event.key.keysym.sym;
	if (sym == SDLK_ESCAPE) {
		joypad->running = false;
	} else if (sym == joypad->sym_a) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_key &= ~BUTTON_A_FLAG;
	} else if (sym == joypad->sym_b) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_key &= ~BUTTON_B_FLAG;
	} else if (sym == joypad->sym_select) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_key &= ~BUTTON_SELECT_FLAG;
	} else if (sym == joypad->sym_start) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_key &= ~BUTTON_START_FLAG;
	} else if (sym == joypad->sym_down) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_dir &= ~BUTTON_DOWN_FLAG;
	} else if (sym == joypad->sym_up) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_dir &= ~BUTTON_UP_FLAG;
	} else if (sym == joypad->sym_left) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_dir &= ~BUTTON_LEFT_FLAG;
	} else if (sym == joypad->sym_right) {
		spec_reg->ifl |= INT_JOYPAD;
		joypad->button_dir &= ~BUTTON_RIGHT_FLAG;
	}
}

static void key_up(struct joypad *joypad)
{
	unsigned i;
	SDL_Keycode sym;
	union SDL_Event *event;
	const struct key_op *key_op;

	event = &joypad->event;
	sym = event->key.keysym.sym;
	for (i = 0; i < KEY_OP_MAX; i++) {
		key_op = joypad->key_op[i];
		if (key_op == NULL)
			break;
		if (key_op->sym == sym) {
			key_op->action(key_op);
			return;
		}
	}
	if (sym == joypad->sym_a)
		joypad->button_key |= BUTTON_A_FLAG;
	else if (sym == joypad->sym_b)
		joypad->button_key |= BUTTON_B_FLAG;
	else if (sym == joypad->sym_select)
		joypad->button_key |= BUTTON_SELECT_FLAG;
	else if (sym == joypad->sym_start)
		joypad->button_key |= BUTTON_START_FLAG;
	else if (sym == joypad->sym_down)
		joypad->button_dir |= BUTTON_DOWN_FLAG;
	else if (sym == joypad->sym_up)
		joypad->button_dir |= BUTTON_UP_FLAG;
	else if (sym == joypad->sym_left)
		joypad->button_dir |= BUTTON_LEFT_FLAG;
	else if (sym == joypad->sym_right)
		joypad->button_dir |= BUTTON_RIGHT_FLAG;
}

static void joy_device_added(struct joypad *joypad, uint32_t index)
{
	int ret;
	const char *joystick_name;
	struct joystick_config *joystick_config;

	joystick_config = joypad->joystick_config;
	joystick_name = SDL_JoystickNameForIndex(index);
	printf("Joystick %s (index = %"PRIi32") detected.\n",
			joystick_name, index);
	if (joystick_config->initialized) {
		printf("Skipped, already using joystick %s\n",
				joystick_config->joystick.name);
		return;
	}
	ret = init_joystick_config(joystick_config, index, joypad->config->dir);
	if (ret < 0) {
		printf("No mapping for %s, create one with joypad_mapping\n",
				joystick_name);
		cleanup_joystick_config(joystick_config);
	}
}

static void joy_device_removed(struct joypad *joypad)
{
	const union SDL_Event *event;
	struct joystick_config *joystick_config;

	event = &joypad->event;
	joystick_config = joypad->joystick_config;
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
		joy_device_added(joypad, event->jdevice.which);
}

static void button_down(struct joypad *joypad, enum gb_button button)
{
	joypad->spec_reg->ifl |= INT_JOYPAD;
	if (BUTTON_IS_KEY(button))
		joypad->button_key &= ~BUTTON_TO_KEY(button);
	else
		joypad->button_dir &= ~BUTTON_TO_DIR(button);
}

static void button_up(struct joypad *joypad, enum gb_button button)
{
	if (BUTTON_IS_KEY(button))
		joypad->button_key |= BUTTON_TO_KEY(button);
	else
		joypad->button_dir |= BUTTON_TO_DIR(button);
}

static void joy_button_action(struct joypad *joypad,
		void (*action)(struct joypad *, enum gb_button))
{
	struct joystick_config *joystick_config;
	enum gb_button button;
	struct control *control;

	joystick_config = joypad->joystick_config;
	if (!joystick_config->initialized)
		return;

	for (button = GB_BUTTON_FIRST; button <= GB_BUTTON_LAST;
			button++) {
		control = &joystick_config->mappings[button].control;
		if (control->type != CONTROL_TYPE_BUTTON)
			continue;
		if (control->button.index == joypad->event.jbutton.button)
			action(joypad, button);
	}
}

static void joy_button_down(struct joypad *joypad)
{
	joy_button_action(joypad, button_down);
}

static void joy_button_up(struct joypad *joypad)
{
	joy_button_action(joypad, button_up);
}

static void joy_axis_motion(struct joypad *joypad)
{
	struct joystick_config *joystick_config;
	enum gb_button button;
	struct control *control;
	const union SDL_Event *event;

	joystick_config = joypad->joystick_config;
	event = &joypad->event;
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
			button_down(joypad, button);
		else
			button_up(joypad, button);

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

static unsigned char joypad_get_state(const struct joypad *pad,
		uint8_t register_p1)
{
	if ((register_p1 & 0x20) == 0)
		return 0xc0 | pad->button_key | 0x10;
	else if ((register_p1 & 0x10) == 0)
		return 0xc0 | pad->button_dir | 0x20;

	return 0xff;
}

void joypad_handle_event(struct joypad *joypad)
{
	union SDL_Event *event;
	struct SDL_WindowEvent *we;
	struct ae_config *aec;
	struct config *config;
	uint32_t width;
	uint32_t height;

	joypad->spec_reg->p1 = joypad_get_state(joypad, joypad->spec_reg->p1);

	event = &joypad->event;
	if (SDL_PollEvent(event) == 0)
		return;

	config = joypad->config;
	aec = &config->config;
	switch (event->type) {
	case SDL_QUIT: {
		printf("see u.\n");
		joypad->running = false;
		config_write(joypad->config);
		break;
	}

	case SDL_WINDOWEVENT:
		we = &event->window;
		switch (we->event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			if (is_fullscreen(we)) {
				if (joypad->mouse_visible) {
					SDL_ShowCursor(SDL_DISABLE);
					joypad->mouse_visible = false;
				}
			} else {
				if (!joypad->mouse_visible) {
					SDL_ShowCursor(SDL_ENABLE);
					joypad->mouse_visible = true;
				}
			}
			height = we->data2;
			if (height < GB_H)
				height = GB_H;
			width = we->data1;
			if (width < GB_W)
				width = GB_W;
			ae_config_add_int(aec, "window_width", width);
			ae_config_add_int(aec, "window_height", height);
			config_write(config);
			break;

		case SDL_WINDOWEVENT_MOVED:
			ae_config_add_int(aec, "window_x", we->data1);
			ae_config_add_int(aec, "window_y", we->data2);
			config_write(config);
			break;
		}
		break;

	case SDL_JOYDEVICEADDED:
		joy_device_added(joypad, event->jdevice.which);
		break;

	case SDL_JOYDEVICEREMOVED:
		joy_device_removed(joypad);
		break;

	case SDL_JOYBUTTONDOWN:
		joy_button_down(joypad);
		break;

	case SDL_JOYBUTTONUP:
		joy_button_up(joypad);
		break;

	case SDL_JOYAXISMOTION:
		joy_axis_motion(joypad);
		break;

	case SDL_KEYDOWN:
		key_down(joypad);
		break;
	case SDL_KEYUP:
		key_up(joypad);
		break;
	}
}

int joypad_save(struct joypad *joypad, FILE *f)
{
	size_t sret;
	uint8_t bool_value;

	bool_value = joypad->running;
	sret = fwrite(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;
	bool_value = joypad->mouse_visible;
	sret = fwrite(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;
	sret = fwrite(&joypad->button_key, sizeof(joypad->button_key), 1, f);
	if (sret != 1)
		return -1;
	sret = fwrite(&joypad->button_dir, sizeof(joypad->button_dir), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}
