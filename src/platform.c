/* Platform specific definitions, for now, only for Windows */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "platform.h"

#ifdef _WIN32
void error(int status, int errnum, const char *format, ...)
{
	va_list arg;

	fflush(stdout);
	fprintf(stderr, "TODO_PROGRAM_NAME_HERE: ");
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
	if (errnum != 0)
		fprintf(stderr, ": %s", strerror(errnum));
	fputs("\n", stderr);

	if (status != 0)
		exit(status);
}

error_t argz_create_sep(const char *str, int sep, char **argz,
		size_t *argz_len)
{
	size_t len;
	unsigned i;

	if (str == NULL || argz == NULL || argz_len == NULL)
		/* codecheck_ignore[USE_NEGATIVE_ERRNO] */
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
