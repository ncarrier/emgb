#include "gb.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

struct gb *gb_init(const char *file)
{
	struct gb *gb = NULL;
	long rom_size;

	gb = calloc(1, sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	gb->running = true;
	rom_size = get_file_size_from_path(file);
	if (rom_size < 0)
		ERR("get_file_size_from_path: %s", strerror(-rom_size));

	config_init(&gb->config);
	joypad_init(&gb->joypad, &gb->config.config);
	memory_init(&gb->memory, &gb->joypad, &gb->timer, rom_size);
	rom_init(&gb->memory.rom_bank_0_rom,
			&gb->memory.switchable_rom_bank_rom,
			gb->memory.extra_rom_banks, file);
	rom_display_header(&gb->memory.rom_bank_0_rom.rom_header);
	registers_init(&gb->registers);
	cpu_init(&gb->cpu);
	gpu_init(&gb->gpu, &gb->cpu, &gb->memory, &gb->config.config);
	timer_init(&gb->timer, &gb->memory, &gb->cpu);

	reset_joystick_config(&gb->joystick_config);
	config_write(&gb->config);

	return gb;
}

void gb_cleanup(struct gb *gb)
{
	cleanup_joystick_config(&gb->joystick_config);
	gpu_cleanup(&gb->gpu);
	config_cleanup(&gb->config);
	free(gb);

	SDL_Quit();
}

