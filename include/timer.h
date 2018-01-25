#ifndef __TIMER__
#define __TIMER__

#define CLOCKSPEED 4194304
/* extract input clock select from TAC register */
#define TAC_INPUT_CLOCK_SELECT(tac) ((tac) & 0x03)
#define TAC_TIMER_ENABLED(tac) ((tac) & 0x04)

struct gb;
struct memory;
struct timer {
	unsigned int freq;
	unsigned char ctrl;
	unsigned char modulator;
	int timerCount;
};

void timer_init(struct memory *memory, struct timer *time);
void updateTimer(struct gb *s_gb);

#endif
