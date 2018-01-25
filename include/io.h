#ifndef IO_HH
#define IO_HH

struct gb;
void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb);

#endif
