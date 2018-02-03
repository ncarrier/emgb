#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rom.h"
#include "utils.h"
#include "log.h"

static int read_file_size_unknown(struct rom *rom,
		struct rom *switchable_rom_bank, uint8_t *extra_rom_banks,
		FILE *f)
{
	unsigned i;
	unsigned sret;

	for (i = 0; i < ROM_BANK_SIZE; i++) {
		sret = fread(rom->data + i, sizeof(*rom->data), 1, f);
		if (sret == 0) {
			if (ferror(f))
				ERR("fread rom bank 0 error");
			else
				return 0;
		}
	}
	if (switchable_rom_bank != NULL)
		memcpy(switchable_rom_bank->data, rom->data, i);
	if (extra_rom_banks != NULL) {
		for (i = 0; i < ((0x200000 + 0x10000 - 0x4000) - 0x10000); i++) {
			sret = fread(extra_rom_banks + i,
					sizeof(*extra_rom_banks), 1, f);
			if (sret == 0) {
				if (ferror(f))
					ERR("fread extra rom banks error");
				else
					break;
			}
		}
	}

	return 0;
}

int rom_init(struct rom *rom, struct rom *switchable_rom_bank,
		uint8_t *extra_rom_banks, const char *file)
{
	size_t sret;
	FILE cleanup(cleanup_file)*f_destroy = NULL;
	FILE *f;
	long size;
	unsigned long rom_bank_0_size;

	if (str_matches(file, "-")) {
		f = stdin;
	} else {
		f_destroy = fopen(file, "rb");
		if (f_destroy == NULL)
			ERR("Cannot open rom file");
		f = f_destroy;
	}
	size = get_file_size(f);
	if (size < 0)
		return read_file_size_unknown(rom, switchable_rom_bank,
				extra_rom_banks, f);

	rom_bank_0_size = min(size, ROM_BANK_SIZE);
	sret = fread(rom->data, sizeof(*rom->data), rom_bank_0_size, f);
	if (sret != rom_bank_0_size)
		ERR("fread rom bank 0 error");
	if (switchable_rom_bank != NULL)
		memcpy(switchable_rom_bank->data, rom->data, rom_bank_0_size);
	if (extra_rom_banks != NULL) {
		size -= rom_bank_0_size;
		sret = fread(extra_rom_banks, sizeof(*extra_rom_banks), size,
				f);
		if (sret != (unsigned long)size)
			ERR("fread rom bank 0 error");
	}

	return 0;
}

void rom_display_header(struct rom_header *rom_header)
{
	printf("rom name: %.*s\n", MAX_TITLE_LENGTH, rom_header->title);
	printf("cartridge type: %d\n", rom_header->cartridge_type);
	printf("rom size: %dKB\n", 32 << rom_header->rom_size);
	printf("ram size: %d\n", rom_header->ram_size << 2);
}
