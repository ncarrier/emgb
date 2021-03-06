#ifndef INCLUDE_CPU_OP_H_
#define INCLUDE_CPU_OP_H_
#include <inttypes.h>

struct gb;
typedef unsigned (*cpufunc)(struct gb *);

struct cpu_op {
	uint8_t opcode;
	char *value;
	cpufunc func;
	uint8_t size;
	uint8_t real_size;
	const char *doc;
	unsigned cycles;
	unsigned cycles_cond;
};

#endif /* INCLUDE_CPU_OP_H_ */
