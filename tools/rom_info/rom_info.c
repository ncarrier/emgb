#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "rom.h"

static const char *hexdump(const uint8_t *in, char *out, size_t size)
{
	unsigned i;
	unsigned j;
	const char nibbles[] = "0123456789abcdef";
	if (size == 0)
		return "";
	for (i = 0, j = 0; i < size; i++, j += 3) {
		out[j] = nibbles[(in[i] & 0xF0) >> 4];
		out[j + 1] = nibbles[in[i] & 0x0F];
		out[j + 2] = ((i + 1) % 8) != 0 ? ' ' : '\n';
	}
	out[j - 1] = '\0';
	return out;
}

static void usage(int status)
{
	puts("usage: rom_info rom_file_path [-d]");

	exit(status);
}

int main(int argc, char *argv[])
{
	int ret;
	struct rom rom;
	const char *filename;
	struct s_romHeader *header = &rom.romheader;
	size_t size = sizeof(*header);
	char out[3 * sizeof(*header)];
	bool dump = false;

	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0)
			usage(EXIT_FAILURE);
	} else if (argc == 3) {
		if (strcmp(argv[2], "-d") == 0)
			dump = true;
		else
			usage(EXIT_FAILURE);
	} else {
		usage(EXIT_FAILURE);
	}

	filename = argv[1];
	ret = initRom(&rom, filename);
	if (ret != 0) {
		fprintf(stderr, "initRom failed\n");
		return EXIT_FAILURE;
	}

	displayHeader(&rom.romheader);
	if (dump)
		printf("rom header content:\n%s\n",
				hexdump((uint8_t *)header, out, size));

	return EXIT_SUCCESS;
}
