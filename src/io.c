#include "gb.h"

static void oam_transfert(struct memory *memory)
{
	int pos;
	unsigned short oamsrc;

	oamsrc = memory->register_dma << 8;
	for (pos = 0; pos < 0xa0; pos++)
		memory->oam[pos] = read8bit(memory, oamsrc + pos);
}

void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb)
{
	switch (addr) {
	case 0xff07:
		timer_init(&gb->memory, &gb->time);
		break;
	case 0xff46:
		oam_transfert(&gb->memory);
		break;
	}
}
