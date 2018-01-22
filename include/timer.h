#ifndef __TIMER__
#define __TIMER__

#define CLOCKSPEED 4194304
/* extract input clock select from TAC register */
#define TAC_INPUT_CLOCK_SELECT(tac) ((tac) & 0x03)
#define TAC_TIMER_ENABLED(tac) ((tac) & 0x04)

struct gb;
void timer_init(struct gb *s_gb);
void updateTimer(struct gb *s_gb);

struct timer {
	unsigned int freq;
	unsigned char ctrl;
	unsigned char modulator;
	int timerCount;
};

#endif
