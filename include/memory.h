#ifndef __MEMORY__
#define __MEMORY__
#include <stdbool.h>
#include <inttypes.h>

/*
 * TODO must be before the gb.h header inclusion, because of an inverted
 * dependency. Fix that ASAP
 */
struct memory {
	unsigned char sram[0x2000];
	unsigned char vram[0x2000];
	unsigned char ram[0x2000];
	unsigned char oam[0x00FF];
	unsigned char io_ports[0x80];
	unsigned char hram[0x80];
	uint8_t mcb_rom_banking;
	bool rom_banking_flag;
	uint8_t cartridge_type;
};

#include "gb.h"

void memory_init(struct memory *memory, struct gb *gb, uint8_t cartridge_type);
int write8bit(uint16_t addr, uint8_t value, struct gb *s_gb);
unsigned char read8bit(unsigned short addr, struct gb *s_gb);
unsigned short read16bit(unsigned short addr, struct gb *s_gb);
void write16bitToAddr(unsigned short addr, unsigned short value,
		struct gb *s_gb);
void push(uint16_t value, struct gb *gb);
uint16_t pop(struct gb *gb);

#endif
