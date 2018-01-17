#include "GB.h"
#include "gpu.h"

int getRealPosition(struct s_gb *s_gb)
{
	int yPos;
	int yDataLine;
	int lineOffset;
	int dataOffset;
	struct s_io *io;

	io = &s_gb->gb_io;
	yPos = io->scrollY + s_gb->gb_gpu.scanline;
	yDataLine = yPos / 8;
	if (yDataLine > 0x1f)
		yDataLine -= 0x20;
	lineOffset = yDataLine * 32;
	dataOffset = io->lcd.BgTileMapSelect + lineOffset + io->scrollX;

	return dataOffset;
}

void renderingBg(struct s_gb *gb)
{
	unsigned short line;
	int color;
	int dec;
	int posx;
	int x;
	unsigned char tileindex;
	signed char stileindex;
	int baseaddr;
	int dataOffset;
	int index;
	int tileAddr;
	struct s_io *io;
	struct s_gpu *gpu;
	int pixel_index;

	gpu = &gb->gb_gpu;
	io = &gb->gb_io;
	baseaddr = io->lcd.BgWindowTileData;
	dataOffset = getRealPosition(gb);
	posx = 0;
	for (index = 0; index < 20; index++) {
		if (io->lcd.BgWindowTileData == 0x8800) {
			stileindex = (signed)(read8bit(index + dataOffset, gb));
			tileAddr = baseaddr + (stileindex + 128) * 16;
		} else {
			tileindex = read8bit(index + dataOffset, gb);
			tileAddr = baseaddr + tileindex * 16;
		}
		dec = 15;
		line = read16bit(tileAddr + (gpu->scanline % 8) * 2, gb);
		for (x = 0; x < 8; x++) {
			color = (line >> dec) & 0x01;
			if ((line >> (dec - 8)) & 0x01)
				color += 2;
			color = color_index_to_value(color);
			pixel_index = 160 * gpu->scanline + posx + x;
			if (pixel_index < (160 * 144))
				gpu->pixels[pixel_index] = color;
			dec--;
		}
		posx += 8;
	}
}
