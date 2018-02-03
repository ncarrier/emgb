#ifndef __MEMORY__
#define __MEMORY__
#include <stdbool.h>
#include <inttypes.h>

#include "rom.h"
#include "special_registers.h"
#include "save.h"

/*
 * biggest rom encountered so far is Pokemon Red-Blue 2-in-1.
 * increase this if no true.
 */
#define BIGGEST_ROM_SIZE 0x200000
#define MEMORY_ADDRESS_SPACE 0x10000
#define TOTAL_MEMORY (BIGGEST_ROM_SIZE + MEMORY_ADDRESS_SPACE - ROM_BANK_SIZE)
#define EXTRA_ROM_BANKS_SIZE (TOTAL_MEMORY - MEMORY_ADDRESS_SPACE)

struct timer;

/* memory mapping extracted from GBCPUman.pdf 2.5.1 */
#pragma pack(push, 1)
struct memory {
	struct timer *timer;

	struct save_start save_start;
	uint8_t mbc_rom_bank;
	bool rom_banking_flag;
	union {
		struct {
			/* TODO rename fields of the two first unions */
			union {
				uint8_t rom_bank_0[ROM_BANK_SIZE];
				struct rom rom_bank_0_rom;
			};
			/* 0x4000 */
			union {
				uint8_t switchable_rom_bank[ROM_BANK_SIZE];
				struct rom switchable_rom_bank_rom;
			};
			/* 0x8000 */
			/* video ram */
			uint8_t vram[0x2000];
			/* 0xa000 */
			/* switchable ram bank */
			uint8_t sram[0x2000];
			/* 0xc000 */
			/* internal ram */
			uint8_t ram[0x2000];
			/* 0xe000 */
			/*
			 * The addresses E000-FE00 appear to access the internal
			 * RAM the same as C000-DE00. (i.e. If you write a byte
			 * to address E000 it will appear at C000 and E000.
			 * Similarly, writing a byte to C000 will appear at C000
			 * and E000.)
			 */
			uint8_t echo_internal_ram[0x1e00];
			/* 0xfe00 */
			/* sprite_attribute_memory */
			uint8_t oam[0x00a0];
			/* 0xfea0 */
			uint8_t empty_usable_for_io_1[0x0060];
			/* 0xff00 */
			union {
				uint8_t io_ports[0x4c];
				struct spec_reg spec_reg;
			};
			/* 0xff4c */
			uint8_t empty_usable_for_io_2[0x0034];
			/* 0xff80 */
			/* high_ram */
			uint8_t hram[0x7f];
			/* 0xffff */
			uint8_t interrupt_enable;
			/* 0x10000 */
			uint8_t extra_rom_banks[EXTRA_ROM_BANKS_SIZE];
		};
		uint8_t raw[TOTAL_MEMORY];
	};
	struct save_end save_end;
} __attribute__((__packed__));
#pragma pack(pop)

void memory_init(struct memory *memory, struct timer *timer);
void write16bit(struct memory *memory, uint16_t addr, uint16_t value);
uint16_t read16bit(struct memory *memory, uint16_t addr);
uint8_t read8bit(struct memory *memory, uint16_t addr);
void write8bit(struct memory *memory, uint16_t addr, uint8_t value);
void push(struct memory *memory, uint16_t *sp, uint16_t value);
uint16_t pop(struct memory *memory, uint16_t *sp);

#endif
