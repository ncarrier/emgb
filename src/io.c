#include "gb.h"

unsigned char padState(const struct gb *gb)
{
	const struct joypad *pad;
	const struct memory *memory;

	memory = &gb->memory;
	pad = &gb->joypad;
	if ((memory->register_p1 & 0x20) == 0)
		return 0xc0 | pad->button_key | 0x10;
	else if ((memory->register_p1 & 0x10) == 0)
		return 0xc0 | pad->button_dir | 0x20;

	return 0xff;
}

void oamTransfert(unsigned char src, struct gb *gb)
{
	int pos;
	unsigned short oamsrc;

	oamsrc = src << 8;
	for (pos = 0; pos < 0xa0; pos++)
		gb->memory.oam[pos] = read8bit(oamsrc + pos, gb);
}

void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb)
{
	struct io *io;

	io = &gb->io;
	switch (addr) {
	case 0xff07:
		io->timerCtrl = io_ports[0x07];
		timer_init(gb);
		printf("timer ctrl %x\n", io->timerCtrl);
		break;
	case 0xff42:
		io->scrollY = io_ports[0x42];
		break;
	case 0xff43:
		io->scrollX = io_ports[0x43];
/*		printf("scroll x = %x\n", io->scrollX); */
		break;
	case 0xff46:
		oamTransfert(io_ports[0x46], gb);
		break;
	case 0xff4A:
		break;
	case 0xff4B:
		break;
	}
}
