#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rom.h"
#include "log.h"

static long get_file_size(FILE *f)
{
	int ret;
	long size;

	if (f == NULL)
		return -EINVAL;

	ret = fseek(f, 0, SEEK_END);
	if (ret != 0)
		ERR("fseek SEEK_END: %m");
	size = ftell(f);
	if (size == -1)
		ERR("fopen: %m");
	ret = fseek(f, 0, SEEK_SET);
	if (ret != 0)
		ERR("fseek SEEK_CUR: %m");

	return size;
}

static int loadRom(struct rom *rom, const char *file)
{
	unsigned int nb_read;
	FILE *f;

	f = fopen(file, "rb");
	if (f == NULL)
		ERR("Cannot open rom file");
	rom->size = get_file_size(f);
	rom->rom = malloc(rom->size * sizeof(char));
	if (rom->rom == NULL)
		ERR("Cannot alloc s_rom");
	nb_read = fread(rom->rom, sizeof(char), rom->size, f);
	if (nb_read == rom->size)
		return 0;

	return -1;
}

static void loadHeader(struct rom *rom)
{
	size_t size;

	size = HEADER_OFFSET_E - HEADER_OFFSET_S;
	memcpy(&rom->rom_header, rom->rom + 0x0100, size);
}

int rom_init(struct rom *rom, const char *filename)
{
	assert(sizeof(struct rom_header) == 80 ||
			"sizeof(s_romHeader) != 80" == NULL);

	if (loadRom(rom, filename) != 0)
		ERR("error loading rom");
	loadHeader(rom);

	return 0;
}

void rom_display_header(struct rom_header *rom_header)
{
	printf("rom name: %.*s\n", MAX_TITLE_LENGTH, rom_header->title);
	printf("cartridge type: %d\n", rom_header->cartridge_type);
	printf("rom size: %dKB\n", 32 << rom_header->rom_size);
	printf("ram size: %d\n", rom_header->ram_size << 2);
}
