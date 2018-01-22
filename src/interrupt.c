#include "interrupt.h"

#include "GB.h"

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
	debug(gb_s);
	gb_s->interrupts.interMaster = 0;
	gb_s->registers.sp -= 2;
	write16bitToAddr(gb_s->registers.sp, gb_s->registers.pc, gb_s);
	gb_s->registers.pc = 0x50;
}

void doInterupt(struct gb *gb_s)
{
	unsigned char inter;

	if (gb_s->interrupts.interFlag & INT_JOYPAD)
		gb_s->cpu.stopped = false;
	gb_s->cpu.halted = false;
	if (gb_s->interrupts.interMaster && gb_s->interrupts.interEnable
			&& gb_s->interrupts.interFlag) {
		inter = gb_s->interrupts.interEnable
				& gb_s->interrupts.interFlag;
		if (inter != 0)
			gb_s->cpu.halted = false;
		if (inter & INT_VBLANK) {
			gb_s->interrupts.interFlag &= ~(INT_VBLANK);
			vblank(gb_s);
		}
		if (inter & INT_LCDSTAT) {
			printf("LCD interrupt\n");
			gb_s->interrupts.interFlag &= ~(INT_LCDSTAT);
			lcd(gb_s);
		}
		if (inter & INT_TIMER) {
			timer(gb_s);
/*			printf("TIMER interrupt\n"); */
			gb_s->interrupts.interFlag &= ~(INT_TIMER);
		}
		if (inter & INT_JOYPAD) {
			gb_s->cpu.stopped = false;
			printf("JOYPAD interrupt\n");
			joypad(gb_s);
			gb_s->interrupts.interFlag &= ~(INT_JOYPAD);
		}
		if (inter & INT_SERIAL) {
			printf("serial interrupt\n");
			serial(gb_s);
			gb_s->interrupts.interFlag &= ~(INT_SERIAL);
		}
	}
}
