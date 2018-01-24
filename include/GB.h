#ifndef __GB__
#define __GB__

#ifndef _WIN32
#include <linux/limits.h>
#else
#define SDL_MAIN_HANDLED
#endif

#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "rom.h"
#include "interrupt.h"
#include "gpu.h"
#include "io.h"
#include "joypad.h"
#include "timer.h"
#include "memory.h"
#include "log.h"
#include "joystick_config.h"
#include "config.h"
#include "utils.h"

#define GB_W 160
#define GB_H 144

struct s_gb {
	unsigned char running;
	struct s_timer gb_time;
	struct s_register gb_register;
	struct s_io gb_io;
	struct s_rom gb_rom;
	struct s_gpu gb_gpu;
	struct s_interupt gb_interrupts;
	struct s_memory gb_mem;
	struct s_joypad gb_pad;
	struct s_cpu gb_cpu;
	struct joystick_config joystick_config;
	unsigned char stopdbg;
	struct config config;
};

void gb(const char *fileName);
void initRegister(struct s_gb *gb);
void debug(struct s_gb *tmprom);
void RDBG(struct s_gb *gb);
void displayStack(struct s_gb *gb);

void seeu(struct s_gb *gb); /* leaving */
struct s_gb *initGb(const char *fileName); /* init */
void initRegister(struct s_gb *gb);

#endif
