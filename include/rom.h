#ifndef __ROM__
#define __ROM__
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
	unsigned size;
	uint8_t *rom;
	struct rom_header rom_header;
} __attribute__((__packed__));

#pragma pack(pop)

int rom_init(struct rom *rom, const char *file);
void rom_display_header(struct rom_header *rom_header);

#endif
