#include "gb.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

static void cpu_init(struct gb *gb)
{
	struct cpu *cpu;

	cpu = &gb->cpu;
	cpu->stopped = false;
	cpu->halted = false;
	cpu->totalTick = 0;
	cpu->last_tick = 0;
}

static void registers_init(struct registers *registers)
{
	registers->af = 0x01B0;
	registers->bc = 0x0013;
	registers->de = 0x00D8;
	registers->hl = 0x014D;
	registers->pc = 0x0100;
	registers->sp = 0xFFFA;

	/* test bit fields order */
	registers->f = 0;
	registers->zf = true;
	if (registers->f != 0x80) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->nf = true;
	if (registers->f != 0x40) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->hf = true;
	if (registers->f != 0x20) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->cf = true;
	if (registers->f != 0x10) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
}

struct gb *gb_init(const char *fileName)
{
	struct gb *gb = NULL;

	gb = malloc(sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	gb->running = true;

	config_init(&gb->config);
	memory_init(gb);
	joypad_init(&gb->joypad, &gb->config.config);
	rom_init(&gb->rom, fileName);
	displayHeader(&gb->rom.romheader);
	registers_init(&gb->registers);
	display_init(gb);
	gpu_init(gb);
	timer_init(gb);
	cpu_init(gb);

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

