#ifndef __GB__
#define __GB__

#ifndef _WIN32
#include <linux/limits.h>
#else
#define SDL_MAIN_HANDLED
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
	bool running;
	struct timer time;
	struct registers registers;
	struct io io;
	struct rom rom;
	struct gpu gpu;
	struct interupts interrupts;
	struct memory gb_mem;
	struct joypad gb_pad;
	struct cpu cpu;
	struct joystick_config joystick_config;
	bool stopdbg;
	struct config config;
};

void gb(const char *file_name);
void gb_cleanup(struct gb *gb);
struct gb *gb_init(const char *file_name);
void initRegister(struct gb *gb);

#endif
