#ifndef __MEMORY__
#define __MEMORY__
#include <stdbool.h>
#include <inttypes.h>

#include "rom.h"
/*
 * biggest rom encountered so far is Pokemon Red-Blue 2-in-1.
 * increase this if no true.
 */
#define BIGGEST_ROM_SIZE 0x200000
#define MEMORY_ADDRESS_SPACE 0x10000
#define TOTAL_MEMORY (BIGGEST_ROM_SIZE + MEMORY_ADDRESS_SPACE - ROM_BANK_SIZE)
#define EXTRA_ROM_BANKS_SIZE (TOTAL_MEMORY - MEMORY_ADDRESS_SPACE)

#pragma pack(push, 1)

enum sprite_size {
	SPRITE_SIZE_8X8,
	SPRITE_SIZE_8X16,
};

enum tile_map_display_select {
	TILE_MAP_DISPLAY_SELECT_8,
	TILE_MAP_DISPLAY_SELECT_C,
};

enum bg_window_tile_data_select {
	BG_WINDOW_TILE_DATA_SELECT_8,
	BG_WINDOW_TILE_DATA_SELECT_0,
};

static inline uint16_t tile_map_display_select_to_addr(
		enum tile_map_display_select sel)
{
	static const uint16_t addr[] = {
			[TILE_MAP_DISPLAY_SELECT_8] = 0x9800,
			[TILE_MAP_DISPLAY_SELECT_C] = 0x9c00,
	};

	return addr[sel];
}

static inline uint16_t bg_window_tile_data_select_to_addr(
		enum bg_window_tile_data_select sel)
{
	static const uint16_t addr[] = {
			[BG_WINDOW_TILE_DATA_SELECT_8] = 0x8800,
			[BG_WINDOW_TILE_DATA_SELECT_0] = 0x8000,
	};

	return addr[sel];
}

struct lcdc {
	bool enable_bg_window_display:1;
	bool enable_sprite_display:1;
	enum sprite_size sprite_size:1;
	enum tile_map_display_select bg_tile_map_display_select:1;
	enum bg_window_tile_data_select bg_window_tile_data_select:1;
	bool enable_window_display:1;
	enum tile_map_display_select window_tile_map_display_select:1;
	bool enable_lcd:1;
};

struct spec_reg {
	/* joypad state */
	uint8_t p1;
	/* serial transfer data */
	uint8_t sb;
	/* serial io control */
	uint8_t sc;
	uint8_t padding_ff03;
	/* divide register */
	uint8_t div;
	/* timer counter */
	uint8_t tima;
	/* timer modulo */
	uint8_t tma;
	/* timer control */
	uint8_t tac;
	uint8_t padding_ff08_0e[7];
	uint8_t ifl; /*  0xff0fu */
	uint8_t nr10; /*  0xff10u */
	uint8_t nr11; /*  0xff11u */
	uint8_t nr12; /*  0xff12u */
	uint8_t nr13; /*  0xff13u */
	uint8_t nr14; /*  0xff14u */
	uint8_t padding_ff15;
	uint8_t nr21; /*  0xff16u */
	uint8_t nr22; /*  0xff17u */
	uint8_t nr23; /*  0xff18u */
	uint8_t nr24; /*  0xff19u */
	uint8_t nr30; /*  0xff1au */
	uint8_t nr31; /*  0xff1bu */
	uint8_t nr32; /*  0xff1cu */
	uint8_t nr33; /*  0xff1du */
	uint8_t nr34; /*  0xff1eu */
	uint8_t padding_ff1f;
	uint8_t nr41; /*  0xff20u */
	uint8_t nr42; /*  0xff21u */
	uint8_t nr43; /*  0xff22u */
	uint8_t nr44; /*  0xff23u */
	uint8_t nr50; /*  0xff24u */
	uint8_t nr51; /*  0xff25u */
	uint8_t nr52; /*  0xff26u */
	uint8_t padding_ff27_2f[9];
	uint8_t wpram[0x10];
	union {
		uint8_t raw; /*  0xff40u */
		struct lcdc lcdc;
	};
	uint8_t stat; /*  0xff41u */
	uint8_t scy; /*  0xff42u */
	uint8_t scx; /*  0xff43u */
	uint8_t ly; /*  0xff44u */
	uint8_t lyc; /*  0xff45u */
	uint8_t dma; /*  0xff46u */
	uint8_t bgp; /*  0xff47u */
	uint8_t obp0; /*  0xff48u */
	uint8_t obp1; /*  0xff49u */
	uint8_t wy; /*  0xff4au */
	uint8_t wx; /*  0xff4bu */
};

struct joypad;
struct timer;
/* memory mapping extracted from GBCPUman.pdf 2.5.1 */
struct memory {
	uint8_t mbc_rom_bank;
	bool rom_banking_flag;
	struct joypad *joypad;
	struct timer *timer;
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
} __attribute__((__packed__));
#pragma pack(pop)

void memory_init(struct memory *memory, struct joypad *joypad,
		struct timer *timer, long rom_size);
void write16bit(struct memory *memory, uint16_t addr, uint16_t value);
uint16_t read16bit(struct memory *memory, uint16_t addr);
uint8_t read8bit(struct memory *memory, uint16_t addr);
void write8bit(struct memory *memory, uint16_t addr, uint8_t value);
void push(struct memory *memory, uint16_t *sp, uint16_t value);
uint16_t pop(struct memory *memory, uint16_t *sp);

#endif
