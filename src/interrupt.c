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
	union interrupt_flags inter;
	struct memory *memory;
	struct cpu *cpu;
	struct spec_reg *spec_reg;
	struct registers *registers;

	memory = interrupts->memory;
	cpu = interrupts->cpu;
	spec_reg = interrupts->spec_reg;
	registers = interrupts->registers;
	if (spec_reg->ifl_flags.joypad)
		cpu->stopped = false;
	if (!interrupts->inter_master && !cpu->halted)
		return;
	inter.raw = memory->interrupt_enable & spec_reg->ifl_flags.raw;
	if (inter.raw == 0)
		return;
	if (cpu->halted) {
		cpu->halted = false;
		registers->pc++;
		if (!interrupts->inter_master)
			return;
	}
	push(memory, &registers->sp, registers->pc);
	interrupts->inter_master = false;
	if (inter.vblank) {
		spec_reg->ifl_flags.vblank = false;
		registers->pc = IT_VBLANK;
	}
	if (inter.lcdstat) {
		printf("LCD interrupt\n");
		spec_reg->ifl_flags.lcdstat = false;
		registers->pc = IT_LCD;
	}
	if (inter.timer) {
		spec_reg->ifl_flags.timer = false;
		registers->pc = IT_TIMER;
	}
	if (inter.joypad) {
		printf("JOYPAD interrupt\n");
		spec_reg->ifl_flags.joypad = false;
		registers->pc = IT_JOYPAD;
	}
	if (inter.serial) {
		printf("serial interrupt\n");
		spec_reg->ifl_flags.serial = false;
		registers->pc = IT_SERIAL;
	}
}
