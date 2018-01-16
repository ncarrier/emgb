/* Platform specific definitions, for now, only for Windows */
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "platform.h"

#ifdef _WIN32
void error(int status, int errnum, const char *format, ...)
{
	// TODO stub
}

error_t argz_create_sep(const char *str, int sep, char **argz,
		size_t *argz_len)
{
	size_t len;
	unsigned i;

	if (str == NULL || argz == NULL || argz_len == NULL)
		return EINVAL;

	*argz = strdup(str);
	if (*argz == NULL)
		return errno;

	*argz_len = 1;
	len = strlen(str) + 1;
	for (i = 0; i < len; i++) {
		if (str[i] == sep) {
			(*argz)[i] = '\0';
			(*argz_len)++;
		}
	}

	return 0;
}

char *envz_get(const char *envz, size_t envz_len, const char *name)
{
	const char *p;
	size_t len;

	if (envz == NULL || name == NULL)
		return NULL;

	for (p = envz; envz_len > 0; p += strlen(p) + 1, envz_len--) {
		len = strlen(name);
		if (strncmp(p, name, len) == 0) {
			if (p[len + 1] == '=' || p[len + 1] == '\0')
				return (char *)(p + len + 2);
		}
	}

	return NULL;
}
#endif
