#include "gb.h"

void updateLcdc(struct gb *gb)
{
	struct lcd *lcd;
	uint8_t lcdc;
	struct io *io;

	io = &gb->io;
	lcd = &io->lcd;
	lcdc = io->lcdc;
	if (lcdc & 0x01)
		lcd->BgWindowDisplay = 1;
	else
		lcd->BgWindowDisplay = 0;

	if (lcdc & 0x02)
		lcd->SpriteIsOn = 1;
	else
		lcd->SpriteIsOn = 0;

	if (lcdc & 0x04)
		lcd->SpriteSize = 16;
	else
		lcd->SpriteSize = 8;

	if (lcdc & 0x08)
		lcd->BgTileMapSelect = 0x9c00;
	else
		lcd->BgTileMapSelect = 0x9800;

	if (lcdc & 0x10)
		lcd->BgWindowTileData = 0x8000;
	else
		lcd->BgWindowTileData = 0x8800;

	if (lcdc & 0x20)
		lcd->WindowIsOn = 1;
	else
		lcd->WindowIsOn = 0;

	if (lcdc & 0x40)
		lcd->WindowTileMapSelect = 0x9c00;
	else
		lcd->WindowTileMapSelect = 0x9800;

	if (lcdc & 0x80)
		lcd->LcdIsOn = 1;
	else
		lcd->LcdIsOn = 0;

	printf("lcd->BgTileMapSelect %x lcd->BgWindowTileData %x\n",
			lcd->BgTileMapSelect, lcd->BgWindowTileData);
	printf("display windows ? %x\n", lcd->WindowIsOn);

}

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
	case 0xff40:
		io->lcdc = io_ports[0x40];
		updateLcdc(gb);
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
