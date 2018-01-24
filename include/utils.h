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
