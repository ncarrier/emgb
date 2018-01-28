#ifndef __TIMER__
#define __TIMER__

#define CLOCKSPEED 4194304
/* extract input clock select from TAC register */
#define TAC_INPUT_CLOCK_SELECT(tac) ((tac) & 0x03)
#define TAC_TIMER_ENABLED(tac) ((tac) & 0x04)

struct memory;
struct cpu;
struct timer {
	struct cpu *cpu;
	struct memory *memory;
	unsigned freq;
	int timer_count;
};

void timer_init(struct timer *timer, struct memory *memory, struct cpu *cpu);
void timer_arm(struct timer *timer);
void timer_update(struct timer *timer);

#endif
