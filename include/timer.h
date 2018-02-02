#ifndef __TIMER__
#define __TIMER__
#include <inttypes.h>
#include <stdio.h>

#define CLOCKSPEED 4194304
/* extract input clock select from TAC register */
#define TAC_INPUT_CLOCK_SELECT(tac) ((tac) & 0x03)
#define TAC_TIMER_ENABLED(tac) ((tac) & 0x04)

struct memory;
struct timer {
	struct memory *memory;

	/* serialized fields */
	uint32_t freq;
	int32_t timer_count;
};

void timer_init(struct timer *timer, struct memory *memory);
void timer_arm(struct timer *timer);
void timer_update(struct timer *timer, unsigned cycles);
int timer_save(const struct timer *timer, FILE *f);
int timer_restore(struct timer *timer, FILE *f);

#endif
