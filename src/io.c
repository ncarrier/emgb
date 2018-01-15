#include "GB.h"

void memoryInit(struct s_gb *s_gb)
{
	memset(s_gb->gb_mem.sram, 0, 0x2000);
	memset(s_gb->gb_mem.vram, 0, 0x2000);
	memset(s_gb->gb_mem.ram, 0, 0x2000);
	memset(s_gb->gb_mem.io_ports, 0, 0x0080);

	write8bit(0xFF05,  0x00, s_gb);
	write8bit(0xFF06,  0x00, s_gb);
	write8bit(0xFF07,  0x08, s_gb);
	write8bit(0xFF10,  0x80, s_gb);
	write8bit(0xFF11,  0xBF, s_gb);
	write8bit(0xFF12,  0xF3, s_gb);
	write8bit(0xFF14,  0xBF, s_gb);
	write8bit(0xFF16,  0x3F, s_gb);
	write8bit(0xFF17,  0x00, s_gb);
	write8bit(0xFF19,  0xBF, s_gb);
	write8bit(0xFF1A,  0x7F, s_gb);
	write8bit(0xFF1B,  0xFF, s_gb);
	write8bit(0xFF1C,  0x9F, s_gb);
	write8bit(0xFF1E,  0xBF, s_gb);
	write8bit(0xFF20,  0xFF, s_gb);
	write8bit(0xFF21,  0x00, s_gb);
	write8bit(0xFF22,  0x00, s_gb);
	write8bit(0xFF23,  0xBF, s_gb);
	write8bit(0xFF24,  0x77, s_gb);
	write8bit(0xFF25,  0xF3, s_gb);
	write8bit(0xFF26,  0xF1, s_gb);
	write8bit(0xFF40,  0x91, s_gb);
	write8bit(0xFF42,  0x00, s_gb);
	write8bit(0xFF43,  0x00, s_gb);
	write8bit(0xFF45,  0x00, s_gb);
	write8bit(0xFF47,  0xFC, s_gb);
	write8bit(0xFF48,  0xFF, s_gb);
	write8bit(0xFF49,  0xFF, s_gb);
	write8bit(0xFF4A,  0x00, s_gb);
	write8bit(0xFF4B,  0x00, s_gb);
	write8bit(0xFFFF,  0x00, s_gb);
}

void updateLcdc(struct s_gb *s_gb)
{
	struct lcd *lcd;
	uint8_t lcdc;
	struct s_io *io;

	io = &s_gb->gb_io;
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

unsigned char padState(struct s_gb *s_gb)
{
	const struct s_joypad *pad;

	pad = &s_gb->gb_pad;
	if ((pad->key & 0x20) == 0)
		return 0xc0 | pad->button_key | 0x10;
	else if ((pad->key & 0x10) == 0)
		return 0xc0 | pad->button_dir | 0x20;

	return 0xff;
}

void oamTransfert(unsigned char src, struct s_gb *gb)
{
	int pos;
	unsigned short oamsrc;

	oamsrc = src << 8;
	for (pos = 0; pos <= 160; pos++)
		gb->gb_mem.oam[pos] = read8bit(oamsrc + pos, gb);
}

void ctrlIo(unsigned short addr, unsigned char *io_ports, struct s_gb *s_gb)
{
	struct s_io *io;

	io = &s_gb->gb_io;
	switch (addr) {
	case 0xff00:
		s_gb->gb_pad.key = io_ports[0x00];
		break;
	case 0xff01:
	case 0xff02:
		io->serial = io_ports[0x01];
		break;
	case 0xff04:
		/* divide counter */
		break;
	case 0xff05:
		break;
	case 0xff06:
		break;
	case 0xff07:
		io->timerCtrl = io_ports[0x07];
		initTimer(s_gb);
		printf("timer ctrl %x\n", io->timerCtrl);
		break;
	case 0xff24:
		io->voice = io_ports[0x24];
		break;
	case 0xff25:
		io->sndStereo = io_ports[0x25];
		break;
	case 0xff26:
		io->sndStat = io_ports[0x26];
		break;
	case 0xff0f:
		s_gb->gb_interrupts.interFlag = io_ports[0x0f];
		break;
	case 0xff40:
		io->lcdc = io_ports[0x40];
		updateLcdc(s_gb);
		break;
	case 0xff42:
		io->scrollY = io_ports[0x42];
		break;
	case 0xff43:
		io->scrollX = io_ports[0x43];
/*		printf("scroll x = %x\n", io->scrollX); */
		break;
	case 0xff46:
		oamTransfert(io_ports[0x46], s_gb);
		break;
	case 0xff4A:
		break;
	case 0xff4B:
		break;
	}
}
