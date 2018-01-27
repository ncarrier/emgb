#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <inttypes.h>

struct cpu {
	unsigned total_tick;
	int last_tick;
	bool stopped;
	bool halted;
};

void cpu_init(struct cpu *cpu);

#endif
