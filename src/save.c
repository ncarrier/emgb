#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include "save.h"
#include "log.h"

int save_write_chunk(FILE *f, struct save_start *start,
		const struct save_end *end)
{
	size_t sret;
	intptr_t len;

	len = (uintptr_t)end - (uintptr_t)start;
	if (len < 0)
		return -EINVAL;
	sret = fwrite(start, len, 1, f);
	if (sret != 1)
		return -EIO;

	return 0;
}

int save_read_chunk(FILE *f, struct save_start *start,
		const struct save_end *end)
{
	size_t sret;
	intptr_t len;

	len = (uintptr_t)end - (uintptr_t)start;
	if (len < 0)
		return -EINVAL;
	sret = fread(start, len, 1, f);
	if (sret != 1)
		return -EIO;

	return 0;
}
