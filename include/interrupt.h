#ifndef INTERRUPT
#define INTERRUPT
#include "interrupt.h"
#include "memory.h"
#include "cpu.h"
#include "registers.h"


#define INT_VBLANK (1 << 0)
#define INT_LCDSTAT (1 << 1)
#define INT_TIMER (1 << 2)
#define INT_SERIAL (1 << 3)
#define INT_JOYPAD (1 << 4)

struct interrupts {
	uint8_t interMaster;
	struct memory *memory;
	struct cpu *cpu;
	struct spec_reg *spec_reg;
	struct registers *registers;
};

void interrupt_init(struct interrupts *interrupts, struct memory *memory,
		struct cpu *cpu, struct spec_reg *spec_reg,
		struct registers *registers);
void interrupt_do(struct interrupts *interupts);

#endif
