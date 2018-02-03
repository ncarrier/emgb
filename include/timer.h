#ifndef __TIMER__
#define __TIMER__
#include <inttypes.h>

#include "save.h"

struct spec_reg;
struct timer {
	struct spec_reg *spec_reg;

	struct save_start save_start;
	uint32_t timer_count;
	struct save_end save_end;
};

void timer_init(struct timer *timer, struct spec_reg *spec_reg);
void timer_update(struct timer *timer, unsigned cycles);

#endif
