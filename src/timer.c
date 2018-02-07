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

	if (!timer->spec_reg->tac.enabled)
		return;

	timer->timer_count += cycles;
	input_clock_sel = timer->spec_reg->tac.ics;
	period = periods_table[input_clock_sel];
	if (timer->timer_count < period)
		return;

	timer->timer_count -= period;
	if (timer->spec_reg->tima == 0xffu) {
		timer->spec_reg->tima = timer->spec_reg->tma;
		timer->spec_reg->ifl_flags.timer = true;
	} else {
		timer->spec_reg->tima++;
	}
}
