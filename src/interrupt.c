#include "interrupt.h"

#include "gb.h"

void vblank(struct gb *gb_s)
{
	gb_s->interrupts.interMaster = 0;
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x40;

}

void lcd(struct gb *gb_s)
{
	gb_s->interrupts.interMaster = 0;
	printf("doing lcdc !\n");
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x48;
}

void joypad(struct gb *gb_s)
{
	gb_s->interrupts.interMaster = 0;
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x60;
}

void serial(struct gb *gb_s)
{
	gb_s->interrupts.interMaster = 0;
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x58;
}

void timer(struct gb *gb_s)
{
	gb_s->interrupts.interMaster = 0;
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x50;
}

void doInterupt(struct gb *gb_s)
{
	unsigned char inter;
	struct memory *memory;

	memory = &gb_s->memory;
	if (memory->register_if & INT_JOYPAD)
		gb_s->cpu.stopped = false;
	gb_s->cpu.halted = false;
	if (gb_s->interrupts.interMaster && memory->interrupt_enable
			&& memory->register_if) {
		inter = memory->interrupt_enable & memory->register_if;
		if (inter != 0)
			gb_s->cpu.halted = false;
		if (inter & INT_VBLANK) {
			memory->register_if &= ~(INT_VBLANK);
			vblank(gb_s);
		}
		if (inter & INT_LCDSTAT) {
			printf("LCD interrupt\n");
			memory->register_if &= ~(INT_LCDSTAT);
			lcd(gb_s);
		}
		if (inter & INT_TIMER) {
			timer(gb_s);
/*			printf("TIMER interrupt\n"); */
			memory->register_if &= ~(INT_TIMER);
		}
		if (inter & INT_JOYPAD) {
			gb_s->cpu.stopped = false;
			printf("JOYPAD interrupt\n");
			joypad(gb_s);
			memory->register_if &= ~(INT_JOYPAD);
		}
		if (inter & INT_SERIAL) {
			printf("serial interrupt\n");
			serial(gb_s);
			memory->register_if &= ~(INT_SERIAL);
		}
	}
}
