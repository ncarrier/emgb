#ifndef __MEMORY__
#define __MEMORY__

#include <inttypes.h>

int write8bit(uint16_t addr, uint8_t value, struct gb *s_gb);
void mcbHandleBanking(unsigned short addr, unsigned char value,
		struct gb *s_gb);
unsigned char read8bit(unsigned short addr, struct gb *s_gb);
unsigned short read16bit(unsigned short addr, struct gb *s_gb);
void write16bitToAddr(unsigned short addr, unsigned short value,
		struct gb *s_gb);
void push(uint16_t value, struct gb *s_gb);
uint16_t pop(struct gb *s_gb);

struct s_memory {
	unsigned char sram[0x2000];
	unsigned char vram[0x2000];
	unsigned char ram[0x2000];
	unsigned char oam[0x00FF];
	unsigned char io_ports[0x80];
	unsigned char hram[0x80];
};

#endif
