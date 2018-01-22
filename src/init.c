#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "GB.h"
#include "special_registers.h"

static void initCpu(struct gb *gb_s)
{
	struct s_register *registers;

	registers = &gb_s->gb_register;
	/* test bit fields order */
	registers->f = 0;
	gb_s->gb_register.zf = true;
	if (registers->f != 0x80) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->gb_register.nf = true;
	if (registers->f != 0x40) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->gb_register.hf = true;
	if (registers->f != 0x20) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	gb_s->gb_register.cf = true;
	if (registers->f != 0x10) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
}

struct gb *initGb(const char *fileName)
{
	struct gb *s_gb = NULL;

	s_gb = malloc(sizeof(*s_gb));
	if (s_gb == NULL)
		ERR("Cannot allocate s_gb");
	config_init(&s_gb->config);
	init_joypad(&s_gb->gb_pad, &s_gb->config.config);
	initRom(&s_gb->gb_rom, fileName);
	displayHeader(&s_gb->gb_rom.romheader);
	initRegister(s_gb);
	initDisplay(s_gb);
	initGpu(s_gb);
	initTimer(s_gb);
	initCpu(s_gb);
	reset_joystick_config(&s_gb->joystick_config);
	config_write(&s_gb->config);

	return s_gb;
}

void	initRegister(struct gb *s_gb)
{
	struct s_register *registers;
	struct s_cpu *cpu;
	struct s_joypad *pad;

	registers = &s_gb->gb_register;
	registers->af = 0x01B0;
	registers->bc = 0x0013;
	registers->de = 0x00D8;
	registers->hl = 0x014D;
	registers->pc = 0x0100;
	registers->sp = 0xFFFA;

	cpu = &s_gb->gb_cpu;
	cpu->stopped = false;
	cpu->halted = false;
	cpu->totalTick = 0;
	cpu->last_tick = 0;

	s_gb->gb_gpu.gpuMode = HBLANK;

	memoryInit(s_gb);

	pad = &s_gb->gb_pad;
	pad->button_key = 0x0f;
	pad->button_dir = 0x0f;

	s_gb->running = 1;
}
