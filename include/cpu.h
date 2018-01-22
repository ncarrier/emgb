#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <inttypes.h>

#include "cpu_op.h"

struct gb;
bool is_opcode_undefined(uint8_t opcode);

#pragma pack(push, 1)

struct s_register {
	struct {
		union {
			struct {
				union {
					uint8_t f;
					struct {
						uint8_t zero:4; /* bits 0->3 */
						bool cf:1; /* bit 4 */
						bool hf:1; /* bit 5 */
						bool nf:1; /* bit 6 */
						bool zf:1; /* bit 7 */
					};
				};
				uint8_t a;
			};
			uint16_t af;
		};
	};
	struct {
		union {
			struct {
				uint8_t c;
				uint8_t b;
			};
			uint16_t bc;
		};
	};
	struct {
		union {
			struct {
				uint8_t e;
				uint8_t d;
			};
			uint16_t de;
		};
	};
	struct {
		union {
			struct {
				uint8_t l;
				uint8_t h;
			};
			uint16_t hl;
		};
	};
	uint16_t pc;
	uint16_t sp;
} __attribute__((__packed__));

extern const struct s_cpu_z80 instructions[256];

struct s_cpu {
	unsigned int totalTick;
	int last_tick;
	bool stopped;
	bool halted;
} __attribute__((__packed__));

#pragma pack(pop)

#endif
