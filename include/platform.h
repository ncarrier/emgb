#ifndef INCLUDE_PLATFORM_H_
#define INCLUDE_PLATFORM_H_

#ifdef _WIN32
typedef int error_t;
void error(int status, int errnum, const char *format, ...);
error_t argz_create_sep(const char *str, int sep, char **argz,
		size_t *argz_len);
char *envz_get(const char *envz, size_t envz_len, const char *name);
#endif

#endif /* INCLUDE_PLATFORM_H_ */
