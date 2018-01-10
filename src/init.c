#include "GB.h"
#include "special_registers.h"

static void initCpu(struct s_gb * gb_s)
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

struct s_gb *initGb(char *fileName)
{
	struct s_gb *s_gb = NULL;

	s_gb = malloc(sizeof(*s_gb));
	if (s_gb == NULL)
		ERR("Cannot allocate s_gb");
	initRom(&s_gb->gb_rom, fileName);
	displayHeader(&s_gb->gb_rom.romheader);
	initRegister(s_gb);
	initDisplay(s_gb);
	initGpu(s_gb);
	initTimer(s_gb);
	initCpu(s_gb);
	reset_joystick_config(&s_gb->joystick_config);
	snprintf(s_gb->config_dir_path, PATH_MAX, "%s/.emgb/", getenv("HOME"));

	return s_gb;
}

void	initRegister(struct s_gb *s_gb)
{
	s_gb->gb_register.af = 0x01B0;
	s_gb->gb_register.bc = 0x0013;
	s_gb->gb_register.de = 0x00D8;
	s_gb->gb_register.hl = 0x014D;
	s_gb->gb_register.pc = 0x0100;
	s_gb->gb_register.sp = 0xFFFA;

	s_gb->gb_cpu.stopCpu = 0;
	s_gb->gb_cpu.totalTick = 0;
	s_gb->gb_cpu.last_tick = 0;
	s_gb->gb_cpu.jmpf = 0;

	s_gb->gb_gpu.gpuMode = HBLANK;

	memoryInit(s_gb);


	s_gb->gb_pad.button_key = 0x0f;
	s_gb->gb_pad.button_dir = 0x0f;

	s_gb->running = 1;
	SET_ZERO();
	SET_HALFC();
	SET_CARRY();
	CLEAR_NEG();
}
