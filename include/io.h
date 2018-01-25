#ifndef IO_HH
#define IO_HH

struct gb;
void ctrlIo(uint16_t addr, uint8_t *io_ports, struct gb *gb);
void oamTransfert(uint8_t src, struct gb *gb);
uint8_t padState(const struct gb *gb);

#endif
