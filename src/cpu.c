#include <stdbool.h>

#include "cpu.h"

void cpu_init(struct cpu *cpu)
{
	cpu->stopped = false;
	cpu->halted = false;
	cpu->totalTick = 0;
	cpu->last_tick = 0;
}
