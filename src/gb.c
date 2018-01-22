#include "gb.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

static void initCpu(struct gb *gb_s)
{
	struct registers *registers;

	registers = &gb_s->registers;
	/* test bit fields order */
	registers->f = 0;
	gb_s->registers.zf = true;
	if (registers->f != 0x80) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->registers.nf = true;
	if (registers->f != 0x40) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->registers.hf = true;
	if (registers->f != 0x20) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->registers.cf = true;
	if (registers->f != 0x10) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
}

static void initRegister(struct gb *s_gb)
{
	struct registers *registers;
	struct cpu *cpu;
	struct joypad *pad;

	registers = &s_gb->registers;
	registers->af = 0x01B0;
	registers->bc = 0x0013;
	registers->de = 0x00D8;
	registers->hl = 0x014D;
	registers->pc = 0x0100;
	registers->sp = 0xFFFA;

	cpu = &s_gb->cpu;
	cpu->stopped = false;
	cpu->halted = false;
	cpu->totalTick = 0;
	cpu->last_tick = 0;

	s_gb->gpu.gpuMode = HBLANK;

	memoryInit(s_gb);

	pad = &s_gb->gb_pad;
	pad->button_key = 0x0f;
	pad->button_dir = 0x0f;

	s_gb->running = true;
}

struct gb *gb_init(const char *fileName)
{
	struct gb *s_gb = NULL;

	s_gb = malloc(sizeof(*s_gb));
	if (s_gb == NULL)
		ERR("Cannot allocate s_gb");
	config_init(&s_gb->config);
	init_joypad(&s_gb->gb_pad, &s_gb->config.config);
	initRom(&s_gb->rom, fileName);
	displayHeader(&s_gb->rom.romheader);
	initRegister(s_gb);
	initDisplay(s_gb);
	initGpu(s_gb);
	initTimer(s_gb);
	initCpu(s_gb);
	reset_joystick_config(&s_gb->joystick_config);
	config_write(&s_gb->config);

	return s_gb;
}

void gb_cleanup(struct gb *s_gb)
{
	config_cleanup(&s_gb->config);
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyWindow(s_gb->gpu.window);
	free(s_gb->rom.rom);
	/* free(s_gb->gb_gpu.pixels_d); */
	free(s_gb->gpu.pixels);
	free(s_gb);

	SDL_Quit();
}

