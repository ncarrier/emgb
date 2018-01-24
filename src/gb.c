#include "gb.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

struct gb *gb_init(const char *fileName)
{
	struct gb *gb = NULL;
	long rom_size;

	gb = calloc(1, sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	gb->running = true;
	rom_size = get_file_size_from_path(fileName);
	if (rom_size < 0)
		ERR("get_file_size_from_path: %s", strerror(-rom_size));

	config_init(&gb->config);
	rom_init(&gb->rom, fileName);
	memory_init(&gb->memory, gb, gb->rom.rom_header.cartridge_type,
			rom_size);
	joypad_init(&gb->joypad, &gb->config.config, &gb->memory);
	rom_display_header(&gb->rom.rom_header);
	registers_init(&gb->registers);
	gpu_init(&gb->gpu, &gb->config.config);
	timer_init(gb);
	cpu_init(&gb->cpu);

	reset_joystick_config(&gb->joystick_config);
	config_write(&gb->config);

	return gb;
}

void gb_cleanup(struct gb *gb)
{
	cleanup_joystick_config(&gb->joystick_config);
	config_cleanup(&gb->config);
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyWindow(gb->gpu.window);
	free(gb->rom.rom);
	/* free(s_gb->gb_gpu.pixels_d); */
	free(gb->gpu.pixels);
	free(gb);

	SDL_Quit();
}

