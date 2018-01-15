#ifndef INCLUDE_CPU_OP_H_
#define INCLUDE_CPU_OP_H_
#include <inttypes.h>

struct s_gb;
typedef unsigned (*cpufunc)(struct s_gb *);

struct s_cpu_z80 {
	unsigned char opcode;
	char *value;
	cpufunc func;
	uint8_t size;
	uint8_t real_size;
	char *doc;
	unsigned cycles;
	unsigned cycles_cond;
};

#endif /* INCLUDE_CPU_OP_H_ */
