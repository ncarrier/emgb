#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include "save.h"
#include "log.h"

void save_write_chunk(struct save_chunk *chunk, FILE *f)
{
	size_t sret;
	intptr_t len;

	len = (uintptr_t)chunk->end - (uintptr_t)chunk->start;
	if (len < 0)
		ERR("Chunk end must be after chunck start.");
	sret = fwrite(chunk->start, len, 1, f);
	if (sret != 1)
		DBG("Failed reading %s.", chunk->name);
}

void save_read_chunk(struct save_chunk *chunk, FILE *f)
{
	size_t sret;
	intptr_t len;

	len = (uintptr_t)chunk->end - (uintptr_t)chunk->start;
	if (len < 0)
		ERR("Chunk end must be after chunck start.");
	sret = fread(chunk->start, len, 1, f);
	if (sret != 1)
		DBG("Failed reading %s.", chunk->name);
}
