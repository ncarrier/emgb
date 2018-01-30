#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "utils.h"

void cleanup_string(char **str)
{
	if (str == NULL || *str == NULL)
		return;

	free(*str);
	*str = NULL;
}

void cleanup_file(FILE **pfile)
{
	if (pfile == NULL || *pfile == NULL)
		return;

	fclose(*pfile);
	*pfile = NULL;
}

long get_file_size_from_path(const char *path)
{
	FILE cleanup(cleanup_file)*f = NULL;

	f = fopen(path, "rbe");
	if (f == NULL)
		return -errno;

	return get_file_size(f);
}

long get_file_size(FILE *f)
{
	int ret;
	long size;

	if (f == NULL)
		return -EINVAL;

	/* compute the size of the file */
	ret = fseek(f, 0, SEEK_END);
	if (ret == -1)
		return -errno;
	size = ftell(f);
	if (ret == -1)
		return -errno;
	ret = fseek(f, 0, SEEK_SET);
	if (ret == -1)
		return -errno;

	return size;
}

