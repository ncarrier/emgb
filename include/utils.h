#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#define BIT0(v) ((v) & 1)
#define BIT(i, v) BIT0((v) >> (i))

static inline bool str_matches(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0;
}

static inline bool str_matches_prefix(const char *s, const char *prefix)
{
	return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* returns an adress inside string s1 */
static inline char *str_diff_chr(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	return (char *)s1;
}

#endif /* _UTILS_H */
