#ifndef IO_HH
#define IO_HH

struct gb;
void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb);
void oamTransfert(uint8_t src, struct gb *gb);
uint8_t padState(const struct gb *gb);
void updateLcdc(struct gb *gb);

struct lcd {
	uint8_t LcdIsOn;
	uint16_t WindowTileMapSelect;
	uint16_t BgTileMapSelect;
	uint8_t WindowIsOn;
	uint16_t BgWindowTileData;
	uint8_t SpriteSize;
	uint8_t SpriteIsOn;
	uint8_t BgWindowDisplay;
};

struct io {
	uint8_t scrollX;
	uint8_t scrollY;
	uint8_t winX;
	uint8_t winY;
	uint8_t serial;
	uint8_t timerCtrl;
	uint8_t sndStat;
	uint8_t sndStereo;
	uint8_t voice;
	uint8_t lcdc;

	struct lcd lcd;
};

#endif
