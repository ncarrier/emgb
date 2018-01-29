#ifndef INTERRUPT
#define INTERRUPT
#include <inttypes.h>
#include <stdio.h>

#define INT_VBLANK (1 << 0)
#define INT_LCDSTAT (1 << 1)
#define INT_TIMER (1 << 2)
#define INT_SERIAL (1 << 3)
#define INT_JOYPAD (1 << 4)

struct memory;
struct cpu;
struct spec_reg;
struct registers;

struct interrupts {
	struct memory *memory;
	struct cpu *cpu;
	struct spec_reg *spec_reg;
	struct registers *registers;

	/* serialized fields */
	uint8_t inter_master;
};

void interrupt_init(struct interrupts *interrupts, struct memory *memory,
		struct cpu *cpu, struct spec_reg *spec_reg,
		struct registers *registers);
void interrupt_update(struct interrupts *interupts);
int interrupt_save(const struct interrupts *interrupts, FILE *f);

#endif
