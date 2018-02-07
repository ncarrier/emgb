#include <stdbool.h>
#include <stdio.h>

#include "interrupt.h"
#include "memory.h"
#include "cpu.h"
#include "registers.h"

#define IT_VBLANK 0x40
#define IT_LCD 0x48
#define IT_TIMER 0x50
#define IT_SERIAL 0x58
#define IT_JOYPAD 0x60

void interrupt_init(struct interrupts *interrupts, struct memory *memory,
		struct cpu *cpu, struct spec_reg *spec_reg,
		struct registers *registers)
{
	interrupts->memory = memory;
	interrupts->cpu = cpu;
	interrupts->spec_reg = spec_reg;
	interrupts->registers = registers;
}

void interrupt_update(struct interrupts *interrupts)
{
	unsigned char inter;
	struct memory *memory;
	struct cpu *cpu;
	struct spec_reg *spec_reg;
	struct registers *registers;

	memory = interrupts->memory;
	cpu = interrupts->cpu;
	spec_reg = interrupts->spec_reg;
	registers = interrupts->registers;
	if (spec_reg->ifl & INT_JOYPAD)
		cpu->stopped = false;
	if (!interrupts->inter_master && !cpu->halted)
		return;
	inter = memory->interrupt_enable & spec_reg->ifl & 0x1f;
	if (inter == 0)
		return;
	if (cpu->halted) {
		cpu->halted = false;
		registers->pc++;
		if (!interrupts->inter_master)
			return;
	}
	push(memory, &registers->sp, registers->pc);
	interrupts->inter_master = false;
	if (inter & INT_VBLANK) {
		registers->pc = IT_VBLANK;
		spec_reg->ifl &= ~INT_VBLANK;
	}
	if (inter & INT_LCDSTAT) {
		printf("LCD interrupt\n");
		spec_reg->ifl &= ~INT_LCDSTAT;
		registers->pc = IT_LCD;
	}
	if (inter & INT_TIMER) {
		spec_reg->ifl &= ~INT_TIMER;
		registers->pc = IT_TIMER;
	}
	if (inter & INT_JOYPAD) {
		printf("JOYPAD interrupt\n");
		spec_reg->ifl &= ~INT_JOYPAD;
		registers->pc = IT_JOYPAD;
	}
	if (inter & INT_SERIAL) {
		printf("serial interrupt\n");
		spec_reg->ifl &= ~INT_SERIAL;
		registers->pc = IT_SERIAL;
	}
}
