#include "timer.h"

#include "gb.h"
#include "special_registers.h"

static const unsigned frequencies_table[] = {
		[0] = 4096u,
		[1] = 262144u,
		[2] = 65536u,
		[3] = 16384u,
};

void timer_init(struct memory *memory, struct timer *time)
{
	uint8_t input_clock_select;

	input_clock_select = TAC_INPUT_CLOCK_SELECT(memory->register_tac);
	time->freq = frequencies_table[input_clock_select];
	time->timerCount = CLOCKSPEED / time->freq;
}

void updateTimer(struct gb *s_gb)
{
	struct cpu *cpu;
	uint8_t tac;
	uint8_t tima;
	uint8_t tma;
	struct memory *memory;

	memory = &s_gb->memory;
	tac = read8bit(SPECIAL_REGISTER_TAC, s_gb);
	cpu = &s_gb->cpu;

	if (!TAC_TIMER_ENABLED(tac))
		return;

	/* FOR TEST !!! - lastTick */
	s_gb->time.timerCount -= cpu->totalTick - cpu->last_tick;
	cpu->last_tick = cpu->totalTick;

	if (s_gb->time.timerCount > 0)
		return;

	s_gb->time.timerCount = CLOCKSPEED / s_gb->time.freq;
	tima = read8bit(SPECIAL_REGISTER_TIMA, s_gb);
	if (tima == 0xffu) {
		tma = read8bit(0xff06, s_gb);
		write8bit(SPECIAL_REGISTER_TIMA, tma, s_gb);
		memory->register_if |= INT_TIMER;
	} else {
		write8bit(SPECIAL_REGISTER_TIMA, tima + 1, s_gb);
	}
}
