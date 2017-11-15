#include <assert.h>

#include <SDL.h>

#include "rom.h"
#include "log.h"

static int loadRom(struct s_rom *rom, const char *romfile)
{
	unsigned int nb_read;

	SDL_RWops *rw = SDL_RWFromFile(romfile, "rb");
	if (rw == NULL)
		ERR("Cannot open rom file");
	rom->size = (unsigned int)SDL_RWsize(rw);
	//printf("rom size = %d\n", s_rom->size);
	rom->rom = malloc(rom->size * sizeof(char));
	if (rom->rom == NULL)
		ERR("Cannot alloc s_rom");
	nb_read = SDL_RWread(rw, rom->rom, sizeof(char), rom->size);
	if (nb_read == rom->size)
		return 0;
	return -1;
}

static void loadHeader(struct s_rom *rom)
{
	size_t size;

	size = HEADER_OFFSET_E - HEADER_OFFSET_S;
	memcpy(&rom->romheader, rom->rom + 0x0100, size);
}

void displayHeader(struct s_romHeader *romheader)
{
	printf("rom name: %.*s\n", MAX_TITLE_LENGTH, romheader->title);
	printf("cartridge type: %d\n", romheader->cartridgeType);
	printf("rom size: %dKB\n", 32 << romheader->romSize);
	printf("ram size: %d\n", romheader->ramSize << 2);
}

int initRom(struct s_rom *rom, const char *filename)
{
	assert(sizeof(struct s_romHeader) == 80 || "sizeof(s_romHeader) != 80" == NULL);

	if (loadRom(rom, filename) != 0)
		ERR("error loading rom");
	loadHeader(rom);
	return 0;
}
