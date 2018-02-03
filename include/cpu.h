#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <inttypes.h>

#include "save.h"

struct cpu {
	struct save_start save_start;
	unsigned total_tick;
	bool stopped;
	bool halted;
	struct save_end save_end;
};

void cpu_init(struct cpu *cpu);

#endif
