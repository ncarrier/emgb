#include "joypad.h"
#include "GB.h"

#define MAPPINGS_DIR "~/.emgb/"

#define BUTTON_RIGHT_FLAG (1 << 0)
#define BUTTON_LEFT_FLAG (1 << 1)
#define BUTTON_UP_FLAG (1 << 2)
#define BUTTON_DOWN_FLAG (1 << 3)

/* TODO check A and B aren't swapped with a real game boy */
#define BUTTON_A_FLAG (1 << 0)
#define BUTTON_B_FLAG (1 << 1)
#define BUTTON_SELECT_FLAG (1 << 2)
#define BUTTON_START_FLAG (1 << 3)

#define BUTTON_KEY_OR_DIR_MASK 0x04
#define BUTTON_IS_KEY(b) (!!((b) & BUTTON_KEY_OR_DIR_MASK))
#define BUTTON_TO_KEY(b) (1 << ((b) & ~BUTTON_KEY_OR_DIR_MASK))
#define BUTTON_TO_DIR(b) (1 << (b))

void keyDown(struct s_gb * gb_s)
{
	gb_s->gb_cpu.stopCpu = 0;
	switch (gb_s->gb_gpu.event.key.keysym.sym)
	{
		gb_s->gb_cpu.stopCpu = 0;
	case SDLK_ESCAPE:
		gb_s->running = 0;
		break;
	case SDLK_w:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~BUTTON_DOWN_FLAG;
		break;
	case SDLK_x:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~BUTTON_UP_FLAG;
		break;
	case SDLK_c:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~BUTTON_LEFT_FLAG;
		break;
	case SDLK_v:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~BUTTON_RIGHT_FLAG;
		break;
	case SDLK_DOWN:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~BUTTON_START_FLAG;
		break;
	case SDLK_UP:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~BUTTON_SELECT_FLAG;
		break;
	case SDLK_LEFT:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~BUTTON_B_FLAG;
		break;
	case SDLK_RIGHT:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~BUTTON_A_FLAG;
		break;
	}
	return;
}

void keyUp(struct s_gb * gb_s)
{
	switch (gb_s->gb_gpu.event.key.keysym.sym)
	{
	case SDLK_w:
		gb_s->gb_pad.button_key |= BUTTON_DOWN_FLAG;
		break;
	case SDLK_x:
		gb_s->gb_pad.button_key |= BUTTON_UP_FLAG;
		break;
	case SDLK_c:
		gb_s->gb_pad.button_key |= BUTTON_LEFT_FLAG;
		break;
	case SDLK_v:
		gb_s->gb_pad.button_key |= BUTTON_RIGHT_FLAG;
		break;
	case SDLK_DOWN:
		gb_s->gb_pad.button_dir |= BUTTON_START_FLAG;
		break;
	case SDLK_UP:
		gb_s->gb_pad.button_dir |= BUTTON_SELECT_FLAG;
		break;
	case SDLK_LEFT:
		gb_s->gb_pad.button_dir |= BUTTON_B_FLAG;
		break;
	case SDLK_RIGHT:
		gb_s->gb_pad.button_dir |= BUTTON_A_FLAG;
		break;
	}
}

static void joy_device_added(struct s_gb *gb, const union SDL_Event *event)
{
	int ret;
	const char *joystick_name;
	struct joystick_config *joystick_config;

	joystick_config = &gb->joystick_config;
	joystick_name = SDL_JoystickNameForIndex(event->jdevice.which);
	printf("Joystick %s (index = %"PRIi32") detected.\n",
			joystick_name, event->jdevice.which);
	if (joystick_config->initialized) {
		printf("Skipped, already using joystick %s\n",
				joystick_config->joystick.name);
		return;
	}
	ret = init_joystick_config(joystick_config,
			event->jdevice.which, gb->config_dir_path);
	if (ret < 0) {
		printf("No mapping found for %s, use joypad_mapping to "
				"create one\n", joystick_name);
		cleanup_joystick_config(joystick_config);
	}
}

static void joy_device_removed(struct s_gb *gb, const union SDL_Event *event)
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
}

/*
 * TODO factor the joy_button_down and joy_button_up, this should be done when
 * button flags have been properly broken up using bit fields
 */
static void joy_button_down(struct s_gb *gb, const union SDL_Event *event)
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
		if (control->button.index == event->jbutton.button) {
			gb->gb_interrupts.interFlag |= INT_JOYPAD;
			gb->gb_cpu.stopCpu = 0;
			if (BUTTON_IS_KEY(button))
				gb->gb_pad.button_key &= ~BUTTON_TO_KEY(button);
			else
				gb->gb_pad.button_dir &= ~BUTTON_TO_DIR(button);
		}
	}
}

static void joy_button_up(struct s_gb *gb, const union SDL_Event *event)
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
		if (control->button.index == event->jbutton.button) {
			if (BUTTON_IS_KEY(button))
				gb->gb_pad.button_key |= BUTTON_TO_KEY(button);
			else
				gb->gb_pad.button_dir |= BUTTON_TO_DIR(button);
		}
	}
}

void handleEvent(struct s_gb *gb_s)
{
	union SDL_Event *event;

	event = &(gb_s->gb_gpu.event);
	if (SDL_PollEvent(event) == 0)
		return;

	switch (gb_s->gb_gpu.event.type) {
	case SDL_QUIT: {
		printf("see u.\n");
		gb_s->running = 0;
		break;
	}

	case SDL_JOYDEVICEADDED:
		joy_device_added(gb_s, event);
		break;

	case SDL_JOYDEVICEREMOVED:
		joy_device_removed(gb_s, event);
		break;

	case SDL_JOYBUTTONDOWN:
		joy_button_down(gb_s, event);
		break;

	case SDL_JOYBUTTONUP:
		joy_button_up(gb_s, event);
		break;

	case SDL_KEYDOWN:
		keyDown(gb_s);
		break;
	case SDL_KEYUP:
		keyUp(gb_s);
		break;
	}
}
