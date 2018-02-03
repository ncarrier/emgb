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

void timer_init(struct timer *timer, struct spec_reg *spec_reg)
{
	timer->spec_reg = spec_reg;
	timer->timer_count = 0;
}

void timer_update(struct timer *timer, unsigned cycles)
{
	uint8_t input_clock_sel;
	uint32_t period;

	if (!TAC_TIMER_ENABLED(timer->spec_reg->tac))
		return;

	timer->timer_count += cycles;
	input_clock_sel = TAC_INPUT_CLOCK_SELECT(timer->spec_reg->tac);
	period = periods_table[input_clock_sel];
	if (timer->timer_count < period)
		return;

	timer->timer_count -= period;
	if (timer->spec_reg->tima == 0xffu) {
		timer->spec_reg->tima = timer->spec_reg->tma;
		timer->spec_reg->ifl |= INT_TIMER;
	} else {
		timer->spec_reg->tima++;
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
