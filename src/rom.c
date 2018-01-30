#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rom.h"
#include "utils.h"
#include "log.h"

int rom_init(struct rom *rom, struct rom *switchable_rom_bank,
		uint8_t *extra_rom_banks, const char *file)
{
	unsigned int nb_read;
	FILE cleanup(cleanup_file)*f = NULL;
	long size;
	long rom_bank_0_size;

	f = fopen(file, "rb");
	if (f == NULL)
		ERR("Cannot open rom file");
	size = get_file_size(f);
	if (size < 0)
		ERR("get_file_size: %s", strerror(-size));
	rom_bank_0_size = min(size, ROM_BANK_SIZE);
	nb_read = fread(rom->data, sizeof(*rom->data), rom_bank_0_size, f);
	if (nb_read != rom_bank_0_size)
		ERR("fread rom bank 0 error");
	if (switchable_rom_bank != NULL)
		memcpy(switchable_rom_bank->data, rom->data, rom_bank_0_size);
	if (extra_rom_banks != NULL) {
		size -= rom_bank_0_size;
		nb_read = fread(extra_rom_banks, sizeof(*extra_rom_banks), size,
				f);
		if (nb_read != size)
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
