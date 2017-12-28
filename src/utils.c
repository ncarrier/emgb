#include <stdlib.h>

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
