#ifndef __MEMORY__
#define __MEMORY__
#include <stdbool.h>
#include <inttypes.h>

/*
 * biggest rom encountered so far is Pokemon Red-Blue 2-in-1.
 * increase this if no true.
 */
#define ROM_BANK_SIZE 0x4000
#define BIGGEST_ROM_SIZE 0x200000
#define MEMORY_ADDRESS_SPACE 0x10000
#define TOTAL_MEMORY (BIGGEST_ROM_SIZE + MEMORY_ADDRESS_SPACE - ROM_BANK_SIZE)
#define EXTRA_ROM_BANKS_SIZE (TOTAL_MEMORY - MEMORY_ADDRESS_SPACE)
#include <inttypes.h>

#define MAX_TITLE_LENGTH 16
#define LOGO_LENGTH 48
#define HEADER_OFFSET_S 0x0100
#define HEADER_OFFSET_E 0x014F

#pragma pack(push, 1)

struct rom_header {
	uint32_t entry_point;
	uint8_t logo[LOGO_LENGTH];
	char title[MAX_TITLE_LENGTH];
	uint16_t manufacturer_mode;
	uint8_t cgb_flag;
	uint8_t cartridge_type;
	uint8_t rom_size;
	uint8_t ram_size;
	uint8_t dest_code; /* 00 JPN 01 N-JPN */
	uint8_t old_licensee_code;
	uint8_t game_version;
	uint8_t header_checksum;
	uint16_t glb_checksum;
} __attribute__((__packed__));

struct rom {
	union {
		uint8_t data[ROM_BANK_SIZE];
		struct {
			uint8_t padding[HEADER_OFFSET_S];
			struct rom_header rom_header;
		};
	};
} __attribute__((__packed__));

#pragma pack(pop)

int rom_init(struct rom *rom, struct rom *switchable_rom_bank,
		uint8_t *extra_rom_banks, const char *filename);
void rom_display_header(struct rom_header *rom_header);

/*
 * TODO must be before the gb.h header inclusion, because of an inverted
 * dependency. Fix that ASAP
 */
#pragma pack(push, 1)
/* memory mapping extracted from GBCPUman.pdf 2.5.1 */
struct memory {
	uint8_t mcb_rom_banking;
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
				struct {
					uint8_t register_p1;
					uint8_t serial;
				};
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
} __attribute__((__packed__));
#pragma pack(pop)

#include "gb.h"

void memory_init(struct memory *memory, struct gb *gb, long rom_size);
void write16bitToAddr(uint16_t addr, uint16_t value, struct gb *gb);
uint16_t read16bit(uint16_t addr, struct gb *gb);
uint8_t read8bit(uint16_t addr, struct gb *gb);
void write8bit(uint16_t addr, uint8_t value, struct gb *gb);
void push(uint16_t value, struct gb *gb);
uint16_t pop(struct gb *gb);

#endif
