#include <stdlib.h>
#include <inttypes.h>

#include "timer.h"
#include "memory.h"
#include "cpu.h"
#include "interrupt.h"
#include "special_registers.h"

static const uint32_t frequencies_table[] = {
		[0] = 4096u,
		[1] = 262144u,
		[2] = 65536u,
		[3] = 16384u,
};

void timer_init(struct timer *timer, struct memory *memory)
{
	timer->memory = memory;
	timer_arm(timer);
}

void timer_arm(struct timer *timer)
{
	uint8_t input_clock_sel;

	/*
	 * when memory is initialized, timer will be armed because it's
	 * registers are modified, but the timer itself is not initialized yet
	 * because it is done AFTER memory has been initialized, thus this NULL
	 * guard
	 */
	if (timer->memory == NULL)
		return;

	input_clock_sel = TAC_INPUT_CLOCK_SELECT(timer->memory->spec_reg.tac);
	timer->freq = frequencies_table[input_clock_sel];
	timer->timer_count = CLOCKSPEED / timer->freq;
}

void timer_update(struct timer *timer, unsigned cycles)
{
	struct memory *memory;

	memory = timer->memory;
	if (!TAC_TIMER_ENABLED(memory->spec_reg.tac))
		return;

	timer->timer_count -= cycles;

	if (timer->timer_count > 0)
		return;

	timer->timer_count = CLOCKSPEED / timer->freq;
	if (memory->spec_reg.tima == 0xffu) {
		write8bit(memory, SPECIAL_REGISTER_TIMA, memory->spec_reg.tma);
		memory->spec_reg.ifl |= INT_TIMER;
	} else {
		write8bit(memory, SPECIAL_REGISTER_TIMA,
				memory->spec_reg.tima + 1);
	}
}

int timer_save(const struct timer *timer, FILE *f)
{
	size_t sret;

	sret = fwrite(&timer->freq, sizeof(timer->freq), 1, f);
	if (sret != 1)
		return -1;
	sret = fwrite(&timer->timer_count, sizeof(timer->timer_count), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}

int timer_restore(struct timer *timer, FILE *f)
{
	size_t sret;

	sret = fread(&timer->freq, sizeof(timer->freq), 1, f);
	if (sret != 1)
		return -1;
	sret = fread(&timer->timer_count, sizeof(timer->timer_count), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}
