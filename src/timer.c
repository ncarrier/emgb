#include <stdlib.h>
#include <inttypes.h>

#include "timer.h"
#include "memory.h"
#include "cpu.h"
#include "interrupt.h"
#include "special_registers.h"

static const uint32_t periods_table[] = {
		[0] = 1024u,
		[1] = 16u,
		[2] = 64u,
		[3] = 256u,
};

void timer_init(struct timer *timer, struct memory *memory)
{
	timer->memory = memory;
	timer->timer_count = 0;
}

void timer_update(struct timer *timer, unsigned cycles)
{
	struct memory *memory;
	uint8_t input_clock_sel;
	uint32_t period;

	memory = timer->memory;
	if (!TAC_TIMER_ENABLED(memory->spec_reg.tac))
		return;

	timer->timer_count += cycles;
	input_clock_sel = TAC_INPUT_CLOCK_SELECT(timer->memory->spec_reg.tac);
	period = periods_table[input_clock_sel];
	if (timer->timer_count < period)
		return;

	timer->timer_count -= period;
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

	sret = fwrite(&timer->timer_count, sizeof(timer->timer_count), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}

int timer_restore(struct timer *timer, FILE *f)
{
	size_t sret;

	sret = fread(&timer->timer_count, sizeof(timer->timer_count), 1, f);
	if (sret != 1)
		return -1;

	return 0;
}
