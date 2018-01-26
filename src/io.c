#include "io.h"

static void oam_transfert(struct memory *memory)
{
	int pos;
	unsigned short oamsrc;

	oamsrc = memory->register_dma << 8;
	for (pos = 0; pos < 0xa0; pos++)
		memory->oam[pos] = read8bit(memory, oamsrc + pos);
}

void io_ctrl(struct memory *memory, struct timer *timer, uint16_t addr)
{
	switch (addr) {
	case 0xff07:
		timer_init(memory, timer);
		break;
	case 0xff46:
		oam_transfert(memory);
		break;
	}
}
