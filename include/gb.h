#ifndef __GB__
#define __GB__
#include <stdbool.h>

#include "timer.h"
#include "registers.h"
#include "gpu.h"
#include "interrupt.h"
#include "memory.h"
#include "joypad.h"
#include "cpu.h"
#include "joystick_config.h"
#include "config.h"

struct gb {
	struct timer timer;
	struct registers registers;
	struct gpu gpu;
	struct interrupts interrupts;
	struct memory memory;
	struct joypad joypad;
	struct cpu cpu;
	struct joystick_config joystick_config;
	struct config config;
	char *file;
	bool stopdbg;
	struct key_op save;
	struct key_op restore;
};

struct gb *gb_init(const char *file_name);
void gb_cleanup(struct gb *gb);

#endif
