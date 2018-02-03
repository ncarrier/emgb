#ifndef INCLUDE_LOG_H_
#define INCLUDE_LOG_H_
#include <stdio.h>
#include <stdlib.h>

#define ERR(s, ...) do {printf("ERR:: " s " ---- %s line %d\n", ##__VA_ARGS__, \
		__FILE__, __LINE__); \
		exit(0); \
} while (0)
#define DBG(s, ...) do {fprintf(stderr, "DBG:: " s " ---- %s line %d\n", ##__VA_ARGS__, \
		__FILE__, __LINE__); \
} while (0)
#define SUCC(s) printf("OK:: %s\n", s)

#endif /* INCLUDE_LOG_H_ */
