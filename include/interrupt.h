#ifndef INTERRUPT
#define INTERRUPT
#include <inttypes.h>
#include <stdbool.h>

#include "save.h"

struct memory;
struct cpu;
struct spec_reg;
struct registers;

struct interrupts {
	struct memory *memory;
	struct cpu *cpu;
	struct spec_reg *spec_reg;
	struct registers *registers;

	struct save_start save_start;
	bool inter_master;
	struct save_end save_end;
};

void interrupt_init(struct interrupts *interrupts, struct memory *memory,
		struct cpu *cpu, struct spec_reg *spec_reg,
		struct registers *registers);
void interrupt_update(struct interrupts *interupts);

#endif
