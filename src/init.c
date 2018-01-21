#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "GB.h"
#include "special_registers.h"

#define DEFAULT_CONFIG "linear_scaling=0\n" \
	"window_x=300\n" \
	"window_y=300\n" \
	"window_width=160\n" \
	"window_height=144\n" \
	"joypad_0_right=1073741903\n" \
	"joypad_0_left=1073741904\n" \
	"joypad_0_up=1073741906\n" \
	"joypad_0_down=1073741905\n" \
	"joypad_0_a=119\n" \
	"joypad_0_b=120\n" \
	"joypad_0_select=99\n" \
	"joypad_0_start=118\n" \
	"debugger_active=0\n" \
	"color_0=0x00000000\n" \
	"color_1=0x00444444\n" \
	"color_2=0x00aaaaaa\n" \
	"color_3=0x00ffffff\n"

static void initCpu(struct s_gb *gb_s)
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

static void init_config(struct s_gb *gb)
{
	int ret;

	snprintf(gb->config_dir_path, PATH_MAX, "%s/" CONFIG_DIR,
			getenv("HOME"));
	ret = mkdir(gb->config_dir_path
#ifndef _WIN32
			, 0755
#endif
			);
	if (ret < 0 && errno != EEXIST)
		ERR("mkdir: %m");

	ret = ae_config_read(&gb->config, "%s/config", gb->config_dir_path);
	if (ret == 0)
		return;
	if (ret != -ENOENT)
		ERR("ae_config_read: %s", strerror(-ret));

	ret = ae_config_read_from_string(&gb->config, DEFAULT_CONFIG);
	if (ret != 0)
		ERR("ae_config_read: %s", strerror(-ret));
}

struct s_gb *initGb(const char *fileName)
{
	struct s_gb *s_gb = NULL;

	s_gb = malloc(sizeof(*s_gb));
	if (s_gb == NULL)
		ERR("Cannot allocate s_gb");
	init_config(s_gb);
	ae_config_write(&s_gb->config, "/dev/stdout");
	init_joypad(&s_gb->gb_pad, &s_gb->config);
	initRom(&s_gb->gb_rom, fileName);
	displayHeader(&s_gb->gb_rom.romheader);
	initRegister(s_gb);
	initDisplay(s_gb);
	initGpu(s_gb);
	initTimer(s_gb);
	initCpu(s_gb);
	reset_joystick_config(&s_gb->joystick_config);

	return s_gb;
}

void	initRegister(struct s_gb *s_gb)
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
