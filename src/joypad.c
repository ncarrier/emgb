#include "joypad.h"
#include "GB.h"

#define MAPPINGS_DIR "~/.emgb/"

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
		gb_s->gb_pad.button_key &= ~(1 << 3);
		break;
	case SDLK_x:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~(1 << 2);
		break;
	case SDLK_c:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~(1 << 1);
		break;
	case SDLK_v:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_key &= ~(1 << 0);
		break;
	case SDLK_DOWN:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~(1 << 3);
		break;
	case SDLK_UP:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~(1 << 2);
		break;
	case SDLK_LEFT:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~(1 << 1);
		break;
	case SDLK_RIGHT:
		gb_s->gb_interrupts.interFlag |= INT_JOYPAD;
		gb_s->gb_pad.button_dir &= ~(1 << 0);
		break;
	}
	return;
}

void keyUp(struct s_gb * gb_s)
{
	switch (gb_s->gb_gpu.event.key.keysym.sym)
	{
	case SDLK_w:
		gb_s->gb_pad.button_key |= (1 << 3);
		break;
	case SDLK_x:
		gb_s->gb_pad.button_key |= (1 << 2);
		break;
	case SDLK_c:
		gb_s->gb_pad.button_key |= (1 << 1);
		break;
	case SDLK_v:
		gb_s->gb_pad.button_key |= (1 << 0);
		break;
	case SDLK_DOWN:
		gb_s->gb_pad.button_dir |= (1 << 3);
		break;
	case SDLK_UP:
		gb_s->gb_pad.button_dir |= (1 << 2);
		break;
	case SDLK_LEFT:
		gb_s->gb_pad.button_dir |= (1 << 1);
		break;
	case SDLK_RIGHT:
		gb_s->gb_pad.button_dir |= (1 << 0);
		break;
	}
}

void handleEvent(struct s_gb *gb_s)
{
	int ret;
	const char *joystick_name;
	SDL_Event *event;
	struct joystick_config *joystick_config;

	joystick_config = &gb_s->joystick_config;
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
		joystick_name = SDL_JoystickNameForIndex(event->jdevice.which);
		printf("Joystick %s (index = %"PRIi32") detected.\n",
				joystick_name, event->jdevice.which);
		if (joystick_config->initialized) {
			printf("Skipped, already using joystick %s\n",
					joystick_config->joystick.name);
			break;
		}
		ret = init_joystick_config(joystick_config,
				event->jdevice.which, gb_s->config_dir_path);
		if (ret < 0) {
			printf("No mapping found for %s, use joypad_mapping to "
					"create one\n", joystick_name);
			cleanup_joystick_config(joystick_config);
		}
		break;

	case SDL_JOYDEVICEREMOVED:
		printf("Joystick SDL_JoystickID = %"PRIi32" removed.%*s\n",
				event->jdevice.which, 25, "");
		if (!joystick_config->initialized)
			break;
		if (joystick_config->joystick.id != event->jdevice.which)
			break;
		printf("Waiting for joystick detection\n");

		cleanup_joystick_config(joystick_config);
		break;

	case SDL_KEYDOWN:
		keyDown(gb_s);
		break;
	case SDL_KEYUP:
		keyUp(gb_s);
		break;
	}
}
