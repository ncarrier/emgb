#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <inttypes.h>

#include "cpu_op.h"

struct gb;
bool is_opcode_undefined(uint8_t opcode);

extern const struct cpu_op instructions[256];

#pragma pack(push, 1)

struct cpu {
	unsigned int totalTick;
	int last_tick;
	bool stopped;
	bool halted;
} __attribute__((__packed__));

#pragma pack(pop)

#endif
