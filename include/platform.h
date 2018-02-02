#ifndef INCLUDE_PLATFORM_H_
#define INCLUDE_PLATFORM_H_
#include <stdbool.h>
#include <assert.h>

_Static_assert(sizeof(bool) == 1, "sizeof bool isn't 1, emgb won't work");

#ifdef _WIN32
/* codecheck_ignore[NEW_TYPEDEFS] */
typedef int error_t;
void error(int status, int errnum, const char *format, ...);
error_t argz_create_sep(const char *str, int sep, char **argz,
		size_t *argz_len);
char *envz_get(const char *envz, size_t envz_len, const char *name);
#endif

#endif /* INCLUDE_PLATFORM_H_ */
