#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <bzlib.h>

#include "log.h"
#include "bzip2.h"

#define BUFFER_SIZE 0x400

enum direction {
	READ,
	WRITE,
};

struct bzip2_file {
	int error;
	FILE *file;
	BZFILE *bz;
	char buffer[BUFFER_SIZE];
	bool eof;
	enum direction direction;
};

static const cookie_io_functions_t bzip2_func;

static int bzip2_close(void *c)
{
	struct bzip2_file *bz2_c_file = c;

	if (bz2_c_file->bz != NULL) {
		if (bz2_c_file->direction == READ)
			BZ2_bzReadClose(&bz2_c_file->error, bz2_c_file->bz);
		else
			BZ2_bzWriteClose(&bz2_c_file->error, bz2_c_file->bz, 0,
					NULL, NULL);
	}
	if (bz2_c_file->file != NULL)
		fclose(bz2_c_file->file);
	memset(bz2_c_file, 0, sizeof(*bz2_c_file));
	free(bz2_c_file);

	return 0;
}

FILE *bzip2_fopen(const char *path, const char *mode)
{
	int old_errno;
	struct bzip2_file *bz2_c_file;

	bz2_c_file = calloc(1, sizeof(*bz2_c_file));
	if (bz2_c_file == NULL) {
		old_errno = errno;
		ERR("calloc: %m");
		errno = old_errno;
		return NULL;
	}
	bz2_c_file->file = fopen(path, mode);
	if (bz2_c_file->file == NULL) {
		old_errno = errno;
		ERR("cushions_fopen: %m");
		goto err;
	}
	if (mode[0] == 'r') {
		bz2_c_file->direction = READ;
		bz2_c_file->bz = BZ2_bzReadOpen(&bz2_c_file->error,
				bz2_c_file->file, 0, 0, NULL, 0);
		if (bz2_c_file->bz == NULL) {
			old_errno = EIO;
			ERR("BZ2_bzReadOpen error %s(%d)",
					BZ2_bzerror(bz2_c_file->bz,
							&bz2_c_file->error),
					bz2_c_file->error);
			goto err;
		}
	} else {
		bz2_c_file->direction = WRITE;
		bz2_c_file->bz = BZ2_bzWriteOpen(&bz2_c_file->error,
				bz2_c_file->file, 9, 0, 0);
		if (bz2_c_file->bz == NULL) {
			old_errno = EIO;
			ERR("BZ2_bzWriteOpen error %s(%d)",
					BZ2_bzerror(bz2_c_file->bz,
							&bz2_c_file->error),
					bz2_c_file->error);
			goto err;
		}
	}

	return fopencookie(bz2_c_file, mode, bzip2_func);
err:

	bzip2_close(bz2_c_file);

	errno = old_errno;
	return NULL;
}

static ssize_t bzip2_read(void *c, char *buf, size_t size)
{
	int ret;
	struct bzip2_file *bz2_c_file = c;

	if (bz2_c_file->eof)
		return 0;

	ret = BZ2_bzRead(&bz2_c_file->error, bz2_c_file->bz, buf, size);
	if (bz2_c_file->error < BZ_OK) {
		ERR("BZ2_bzRead error %s(%d)",
				BZ2_bzerror(bz2_c_file->bz, &bz2_c_file->error),
				bz2_c_file->error);
		errno = EIO;
		return -1;
	}
	if (bz2_c_file->error == BZ_STREAM_END)
		bz2_c_file->eof = true;

	return ret;
}

static ssize_t bzip2_write(void *cookie, const char *buf, size_t size)
{
	struct bzip2_file *bz2_c_file = cookie;

	if (bz2_c_file->eof)
		return 0;

	/* buf parameter of BZ2_bzWrite isn't const, don't know why... */
	BZ2_bzWrite(&bz2_c_file->error, bz2_c_file->bz, (void *)buf, size);
	if (bz2_c_file->error < BZ_OK) {
		ERR("BZ2_bzWrite error %s(%d)",
				BZ2_bzerror(bz2_c_file->bz, &bz2_c_file->error),
				bz2_c_file->error);
		errno = EIO;
		return 0;
	}
	if (bz2_c_file->error == BZ_STREAM_END) {
		bz2_c_file->eof = true;
		return 0;
	}

	return size;
}

static const cookie_io_functions_t bzip2_func = {
	.read  = bzip2_read,
	.write = bzip2_write,
	.close = bzip2_close
};
