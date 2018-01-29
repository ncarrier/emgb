#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

/* all fields are serialized */
struct cpu {
	unsigned total_tick;
	bool stopped;
	bool halted;
};

void cpu_init(struct cpu *cpu);
int cpu_save(const struct cpu *cpu, FILE *f);

#endif
