#ifndef IO_HH
#define IO_HH
#include <inttypes.h>

struct memory;
struct timer;
void io_ctrl(struct memory *memory, struct timer *timer, uint16_t addr);

#endif
