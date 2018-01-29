#include <stdbool.h>

#include "cpu.h"

void cpu_init(struct cpu *cpu)
{
	cpu->stopped = false;
	cpu->halted = false;
	cpu->total_tick = 0;
}

int cpu_save(const struct cpu *cpu, FILE *f)
{
	size_t sret;
	uint8_t bool_value;

	sret = fwrite(&cpu->total_tick, sizeof(cpu->total_tick), 1, f);
	if (sret != 1)
		return -1;
	bool_value = cpu->stopped;
	sret = fwrite(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;
	bool_value = cpu->halted;
	sret = fwrite(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}

int cpu_restore(struct cpu *cpu, FILE *f)
{
	size_t sret;
	uint8_t bool_value;

	sret = fread(&cpu->total_tick, sizeof(cpu->total_tick), 1, f);
	if (sret != 1)
		return -1;
	sret = fread(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;
	cpu->stopped = bool_value;
	sret = fread(&bool_value, sizeof(bool_value), 1, f);
	if (sret != 1)
		return -1;
	cpu->halted = bool_value;

	return 0;
}
