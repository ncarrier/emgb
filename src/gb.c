#include "gb.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

struct gb *gb_init(const char *fileName)
{
	struct gb *gb = NULL;

	gb = calloc(1, sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	gb->running = true;

	config_init(&gb->config);
	rom_init(&gb->rom, fileName);
	memory_init(&gb->memory, gb, gb->rom.romheader.cartridgeType);
	joypad_init(&gb->joypad, &gb->config.config);
	displayHeader(&gb->rom.romheader);
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
	config_cleanup(&gb->config);
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyWindow(gb->gpu.window);
	free(gb->rom.rom);
	/* free(s_gb->gb_gpu.pixels_d); */
	free(gb->gpu.pixels);
	free(gb);

	SDL_Quit();
}

