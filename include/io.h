#ifndef IO_HH
#define IO_HH
#include <inttypes.h>

#include "memory.h"
#include "timer.h"

void io_ctrl(struct memory *memory, struct timer *timer, uint16_t addr);

#endif
