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

static void interrupt_handle(struct memory *memory,
		struct interrupts *interrupts, struct registers *registers,
		uint16_t address)
{
	interrupts->inter_master = false;
	registers->sp -= 2;
	write16bit(memory, registers->sp, registers->pc);
	registers->pc = address;
}

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
	cpu->halted = false;
	if (!interrupts->inter_master || !memory->interrupt_enable
			|| !spec_reg->ifl)
		return;
	inter = memory->interrupt_enable & spec_reg->ifl;
	if (inter != 0)
		cpu->halted = false;
	if (inter & INT_VBLANK) {
		spec_reg->ifl &= ~INT_VBLANK;
		interrupt_handle(memory, interrupts, registers, IT_VBLANK);
	}
	if (inter & INT_LCDSTAT) {
		printf("LCD interrupt\n");
		spec_reg->ifl &= ~INT_LCDSTAT;
		interrupt_handle(memory, interrupts, registers, IT_LCD);
	}
	if (inter & INT_TIMER) {
		interrupt_handle(memory, interrupts, registers, IT_TIMER);
		/*			printf("TIMER interrupt\n"); */
		spec_reg->ifl &= ~INT_TIMER;
	}
	if (inter & INT_JOYPAD) {
		cpu->stopped = false;
		printf("JOYPAD interrupt\n");
		interrupt_handle(memory, interrupts, registers, IT_JOYPAD);
		spec_reg->ifl &= ~INT_JOYPAD;
	}
	if (inter & INT_SERIAL) {
		printf("serial interrupt\n");
		interrupt_handle(memory, interrupts, registers, IT_SERIAL);
		spec_reg->ifl &= ~INT_SERIAL;
	}
}

int interrupt_save(const struct interrupts *interrupts, FILE *f)
{
	size_t sret;

	sret = fwrite(&interrupts->inter_master,
			sizeof(interrupts->inter_master), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}

int interrupt_restore(struct interrupts *interrupts, FILE *f)
{
	size_t sret;

	sret = fread(&interrupts->inter_master,
			sizeof(interrupts->inter_master), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}
