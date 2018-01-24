#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rom.h"
#include "utils.h"
#include "log.h"

static int load_rom(struct rom *rom, const char *file)
{
	unsigned int nb_read;
	FILE *f;
	long size;

	f = fopen(file, "rb");
	if (f == NULL)
		ERR("Cannot open rom file");
	size = get_file_size(f);
	if (size < 0)
		ERR("get_file_size: %s", strerror(-size));
	rom->rom = malloc(size * sizeof(*rom->rom));
	if (rom->rom == NULL)
		ERR("Cannot alloc s_rom");
	nb_read = fread(rom->rom, sizeof(char), size, f);
	if (nb_read == size)
		return 0;

	return -1;
}

static void load_header(struct rom *rom)
{
	size_t size;

	size = HEADER_OFFSET_E - HEADER_OFFSET_S;
	memcpy(&rom->rom_header, rom->rom + 0x0100, size);
}

int rom_init(struct rom *rom, const char *filename)
{
	assert(sizeof(struct rom_header) == 80 ||
			"sizeof(s_romHeader) != 80" == NULL);

	if (load_rom(rom, filename) != 0)
		ERR("error loading rom");
	load_header(rom);

	return 0;
}

void rom_display_header(struct rom_header *rom_header)
{
	printf("rom name: %.*s\n", MAX_TITLE_LENGTH, rom_header->title);
	printf("cartridge type: %d\n", rom_header->cartridge_type);
	printf("rom size: %dKB\n", 32 << rom_header->rom_size);
	printf("ram size: %d\n", rom_header->ram_size << 2);
}
