#include "timer.h"
#include "GB.h"
#include "special_registers.h"

static const unsigned frequencies_table[] = {
		[0] = 4096u,
		[1] = 262144u,
		[2] = 65536u,
		[3] = 16384u,
};

void initTimer(struct gb *s_gb)
{
	uint8_t tac;
	uint8_t input_clock_select;

	tac = read8bit(SPECIAL_REGISTER_TAC, s_gb);
	printf("TAC value %d\n", tac);
	input_clock_select = TAC_INPUT_CLOCK_SELECT(tac);

	s_gb->gb_time.freq = frequencies_table[input_clock_select];
	s_gb->gb_time.timerCount = CLOCKSPEED / s_gb->gb_time.freq;
}

void updateTimer(struct gb *s_gb)
{
	struct cpu *cpu;
	uint8_t tac;
	uint8_t tima;
	uint8_t tma;

	tac = read8bit(SPECIAL_REGISTER_TAC, s_gb);
	cpu = &s_gb->gb_cpu;

	if (!TAC_TIMER_ENABLED(tac))
		return;

	/* FOR TEST !!! - lastTick */
	s_gb->gb_time.timerCount -= cpu->totalTick - cpu->last_tick;
	cpu->last_tick = cpu->totalTick;

	if (s_gb->gb_time.timerCount > 0)
		return;

	s_gb->gb_time.timerCount = CLOCKSPEED / s_gb->gb_time.freq;
	tima = read8bit(SPECIAL_REGISTER_TIMA, s_gb);
	if (tima == 0xffu) {
		tma = read8bit(0xff06, s_gb);
		write8bit(SPECIAL_REGISTER_TIMA, tma, s_gb);
		s_gb->gb_interrupts.interFlag |= INT_TIMER;
	} else {
		write8bit(SPECIAL_REGISTER_TIMA, tima + 1, s_gb);
	}
}
