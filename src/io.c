#include "gb.h"

static void oam_transfert(struct gb *gb)
{
	int pos;
	unsigned short oamsrc;

	oamsrc = gb->memory.register_dma << 8;
	for (pos = 0; pos < 0xa0; pos++)
		gb->memory.oam[pos] = read8bit(oamsrc + pos, gb);
}

void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb)
{
	switch (addr) {
	case 0xff07:
		timer_init(&gb->memory, &gb->time);
		break;
	case 0xff46:
		oam_transfert(gb);
		break;
	case 0xff4A:
		break;
	case 0xff4B:
		break;
	}
}
