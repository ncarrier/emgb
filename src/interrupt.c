#include "interrupt.h"

#include "gb.h"

void vblank(struct gb *gb)
{
	gb->interrupts.interMaster = 0;
	gb->registers.sp -= 2;
	write16bit(&gb->memory, gb->registers.sp, gb->registers.pc);
	gb->registers.pc = 0x40;

}

void lcd(struct gb *gb)
{
	gb->interrupts.interMaster = 0;
	printf("doing lcdc !\n");
	gb->registers.sp -= 2;
	write16bit(&gb->memory, gb->registers.sp, gb->registers.pc);
	gb->registers.pc = 0x48;
}

void joypad(struct gb *gb)
{
	gb->interrupts.interMaster = 0;
	gb->registers.sp -= 2;
	write16bit(&gb->memory, gb->registers.sp, gb->registers.pc);
	gb->registers.pc = 0x60;
}

void serial(struct gb *gb)
{
	gb->interrupts.interMaster = 0;
	gb->registers.sp -= 2;
	write16bit(&gb->memory, gb->registers.sp, gb->registers.pc);
	gb->registers.pc = 0x58;
}

void timer(struct gb *gb)
{
	gb->interrupts.interMaster = 0;
	gb->registers.sp -= 2;
	write16bit(&gb->memory, gb->registers.sp, gb->registers.pc);
	gb->registers.pc = 0x50;
}

void doInterupt(struct gb *gb)
{
	unsigned char inter;
	struct memory *memory;

	memory = &gb->memory;
	if (memory->register_if & INT_JOYPAD)
		gb->cpu.stopped = false;
	gb->cpu.halted = false;
	if (gb->interrupts.interMaster && memory->interrupt_enable
			&& memory->register_if) {
		inter = memory->interrupt_enable & memory->register_if;
		if (inter != 0)
			gb->cpu.halted = false;
		if (inter & INT_VBLANK) {
			memory->register_if &= ~(INT_VBLANK);
			vblank(gb);
		}
		if (inter & INT_LCDSTAT) {
			printf("LCD interrupt\n");
			memory->register_if &= ~(INT_LCDSTAT);
			lcd(gb);
		}
		if (inter & INT_TIMER) {
			timer(gb);
/*			printf("TIMER interrupt\n"); */
			memory->register_if &= ~(INT_TIMER);
		}
		if (inter & INT_JOYPAD) {
			gb->cpu.stopped = false;
			printf("JOYPAD interrupt\n");
			joypad(gb);
			memory->register_if &= ~(INT_JOYPAD);
		}
		if (inter & INT_SERIAL) {
			printf("serial interrupt\n");
			serial(gb);
			memory->register_if &= ~(INT_SERIAL);
		}
	}
}
