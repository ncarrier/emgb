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

#include "timer.h"
#include "registers.h"
#include "io.h"
#include "rom.h"
#include "gpu.h"
#include "interrupt.h"
#include "memory.h"
#include "joypad.h"
#include "cpu.h"
#include "joystick_config.h"
#include "config.h"
#include "utils.h"
#include "log.h"

#define GB_W 160
#define GB_H 144

struct gb {
	struct timer time;
	struct registers registers;
	struct io io;
	struct rom rom;
	struct gpu gpu;
	struct interupts interrupts;
	struct memory memory;
	struct joypad joypad;
	struct cpu cpu;
	struct joystick_config joystick_config;
	struct config config;
	bool stopdbg;
	bool running;
};

struct gb *gb_init(const char *file_name);
void gb_cleanup(struct gb *gb);

#endif
