#include <stdio.h>
#include <stdlib.h>

#include "registers.h"

void registers_init(struct registers *registers)
{
	registers->af = 0x01B0;
	registers->bc = 0x0013;
	registers->de = 0x00D8;
	registers->hl = 0x014D;
	registers->pc = 0x0100;
	registers->sp = 0xFFFA;

	/* test bit fields order */
	registers->f = 0;
	registers->zf = true;
	if (registers->f != 0x80) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->nf = true;
	if (registers->f != 0x40) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->hf = true;
	if (registers->f != 0x20) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
	registers->f = 0;
	registers->cf = true;
	if (registers->f != 0x10) {
		fprintf(stderr, "Unsupported bitfield order\n");
		exit(1);
	}
}

int registers_save(const struct registers *registers, FILE *f)
{
	size_t sret;

	sret = fwrite(registers, sizeof(*registers), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}

int registers_restore(struct registers *registers, FILE *f)
{
	size_t sret;

	sret = fread(registers, sizeof(*registers), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}
