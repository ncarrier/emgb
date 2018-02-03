#ifndef __TIMER__
#define __TIMER__
#include <inttypes.h>
#include <stdio.h>

/* extract input clock select from TAC register */
/* TODO replace with bit fields */
#define TAC_INPUT_CLOCK_SELECT(tac) ((tac) & 0x03)
#define TAC_TIMER_ENABLED(tac) ((tac) & 0x04)

struct spec_reg;
struct timer {
	struct spec_reg *spec_reg;

	/* serialized fields */
	uint32_t timer_count;
};

void timer_init(struct timer *timer, struct spec_reg *spec_reg);
void timer_update(struct timer *timer, unsigned cycles);
int timer_save(const struct timer *timer, FILE *f);
int timer_restore(struct timer *timer, FILE *f);

#endif
