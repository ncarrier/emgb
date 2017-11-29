#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#define BIT0(v) ((v) & 1)
#define BIT(i, v) BIT0((v) >> (i))

#define FLAGS_ZERO (1 << 7)
#define FLAGS_NEGATIVE (1 << 6)
#define FLAGS_HALFCARRY (1 << 5)
#define FLAGS_CARRY (1 << 4)

#define FLAGS_ISZERO(f) (!!((f) & FLAGS_ZERO))
#define FLAGS_ISNEGATIVE(f) (!!((f) & FLAGS_NEGATIVE))
#define FLAGS_ISCARRY(f) (!!((f) & FLAGS_CARRY))
#define FLAGS_ISHALFCARRY(f) (!!((f) & FLAGS_HALFCARRY))

#define FLAGS_ISSET(f, x) ((f) & (x))
#define FLAGS_SET(f, x) ((f) |= (x))
#define FLAGS_CLEAR(f, x) ((f) &= ~(x))


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
