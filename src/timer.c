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

void timer_update(struct timer *timer, struct gb *gb, struct memory *memory,
		struct cpu *cpu)
{
	if (!TAC_TIMER_ENABLED(memory->register_tac))
		return;

	/* FOR TEST !!! - lastTick */
	timer->timerCount -= cpu->totalTick - cpu->last_tick;
	cpu->last_tick = cpu->totalTick;

	if (timer->timerCount > 0)
		return;

	timer->timerCount = CLOCKSPEED / timer->freq;
	if (memory->register_tima == 0xffu) {
		write8bit(SPECIAL_REGISTER_TIMA, memory->register_tma, gb);
		memory->register_if |= INT_TIMER;
	} else {
		write8bit(SPECIAL_REGISTER_TIMA, memory->register_tima + 1, gb);
	}
}
