#include "io.h"

static void oam_transfert(struct memory *memory)
{
	int pos;
	unsigned short oamsrc;

	/*
	 * In theory, this could be implemented with a memcpy, but because of
	 * banking, it's not possible.
	 */
	oamsrc = memory->spec_reg.dma << 8;
	for (pos = 0; pos < 0xa0; pos++)
		memory->oam[pos] = read8bit(memory, oamsrc + pos);
}

void io_ctrl(struct memory *memory, struct timer *timer, uint16_t addr)
{
	switch (addr) {
	case 0xff07:
		timer_arm(timer);
		break;
	case 0xff46:
		oam_transfert(memory);
		break;
	}
}
