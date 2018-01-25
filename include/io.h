#ifndef IO_HH
#define IO_HH

struct gb;
void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb);
void oamTransfert(uint8_t src, struct gb *gb);
uint8_t padState(const struct gb *gb);

struct io {
	uint8_t scrollX;
	uint8_t scrollY;
	uint8_t winX;
	uint8_t winY;
	uint8_t serial;
	uint8_t timerCtrl;
	uint8_t sndStat;
	uint8_t sndStereo;
	uint8_t lcdc;
};

#endif
