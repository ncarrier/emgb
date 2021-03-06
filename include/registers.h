#ifndef INCLUDE_REGISTERS_H_
#define INCLUDE_REGISTERS_H_
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#pragma pack(push, 1)
/* all fields are serialized */
struct registers {
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
#pragma pack(pop)

void registers_init(struct registers *registers);
int registers_save(const struct registers *registers, FILE *f);
int registers_restore(struct registers *registers, FILE *f);

#endif /* INCLUDE_REGISTERS_H_ */
