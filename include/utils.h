#ifndef _UTILS_H
#define _UTILS_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BIT0(v) ((v) & 1)
#define BIT(i, v) BIT0((v) >> (i))

#define SET_BIT(f, b) ((f) |= (1 << (b)))
#define CLEAR_BIT(f, b) ((f) &= ~(1 << (b)))

#define cleanup(f) __attribute((cleanup(f)))
#define have_same_sign(n1, n2) (((n1) * (n2)) >= 0)
#ifndef _container_of
#define container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member)*__mptr = (ptr); \
	(type *)((uintptr_t)__mptr - offsetof(type, member)); })
#endif /* container_of */

#define min(a, b) ({ \
	typeof((a)) _a = (a); \
	typeof((b)) _b = (b); \
	\
	_a < _b ? _a : b; \
})

#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_A) (sizeof(_A) / sizeof((_A)[0]))
#endif /* ARRAY_SIZE */

static inline bool str_matches(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0;
}

static inline bool str_matches_prefix(const char *s, const char *prefix)
{
	return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* returns an address inside string s1 */
static inline char *str_diff_chr(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	return (char *)s1;
}

void cleanup_string(char **str);
void cleanup_file(FILE **pfile);
long get_file_size_from_path(const char *path);
long get_file_size(FILE *f);

#endif /* _UTILS_H */
