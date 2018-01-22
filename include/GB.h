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

struct gb {
	unsigned char running;
	struct timer gb_time;
	struct registers gb_register;
	struct io gb_io;
	struct rom gb_rom;
	struct gpu gb_gpu;
	struct interupts gb_interrupts;
	struct memory gb_mem;
	struct joypad gb_pad;
	struct cpu gb_cpu;
	struct joystick_config joystick_config;
	unsigned char stopdbg;
	struct config config;
};

void gb(const char *fileName);
void initRegister(struct gb *gb);
void debug(struct gb *tmprom);
void RDBG(struct gb *gb);
void displayStack(struct gb *gb);

void seeu(struct gb *gb); /* leaving */
struct gb *initGb(const char *fileName); /* init */
void initRegister(struct gb *gb);

#endif
