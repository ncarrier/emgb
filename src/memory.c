#include "../include/memory.h"

#include "gb.h"
#include "special_registers.h"

static unsigned char MCB_romBanking = 1;
static unsigned char romBankingFlag;

void write16bitToAddr(unsigned short addr, unsigned short value,
		struct gb *s_gb)
{
	write8bit(addr, (unsigned char)value & 0x00ff, s_gb);
	write8bit(addr + 1, (unsigned char)((value & 0xff00) >> 8), s_gb);
}

unsigned short read16bit(unsigned short addr, struct gb *s_gb)
{
	unsigned short res = 0;

	res |= read8bit(addr, s_gb);
	res |= read8bit(addr + 1, s_gb) << 8;

	return res;
}

unsigned char read8bit(unsigned short addr, struct gb *s_gb)
{

	if (addr == 0xff44) {
		return s_gb->gpu.scanline;
	} else if (addr < 0x4000) {
		return s_gb->rom.rom[addr];
	} else if (addr >= 0x4000 && addr < 0x8000) {
		/* printf("MCB_romBanking value = %x\n", MCB_romBanking); */
		return s_gb->rom.rom[(addr - 0x4000)
				+ (MCB_romBanking * 0x4000)];
	} else if (addr >= 0x8000 && addr < 0xA000) {
		return s_gb->memory.vram[addr - 0x8000];
	} else if (addr >= 0xA000 && addr < 0xC000) {
		return  s_gb->memory.sram[addr - 0xA000];
	} else if (addr >= 0xC000 && addr < 0xE000) {
		if (addr == 0xc0b7)
			printf("read from ram 0xc0b7 %x\n",
					s_gb->memory.ram[addr - 0xC000]);
		return  s_gb->memory.ram[addr - 0xC000];
	} else if (addr >= 0xE000 && addr < 0xFE00) {
		return  s_gb->memory.ram[addr - 0xE000];
	} else if (addr >= 0xFE00 && addr < 0xFF00) {
		return  s_gb->memory.oam[addr - 0xFE00];
	} else if (addr >= 0xFF00 && addr < 0xFF80) {
		if (addr == 0xff00)
			return padState(s_gb);
		if (addr == 0xff04)
			return (unsigned char)rand();
		if (addr == 0xff0f)
			return s_gb->interrupts.interFlag;
		if (addr == 0xff41)
			printf("reading lcd stat\n");
		return  s_gb->memory.io_ports[addr - 0xFF00];
	} else if (addr >= 0xFF80 && addr < 0xFFFF) {
		return  s_gb->memory.hram[addr - 0xFF80];
	} else if (addr == 0xffff) {
		return s_gb->interrupts.interEnable;
	}
	printf("read error : addr %x\n", addr);
	exit(-2);
}

void mcbHandleBanking(unsigned short addr, unsigned char value,
		struct gb *s_gb)
{
	char low5;

	low5 = value & 0x1f;

	if (addr >= 0x2000 && addr < 0x4000) {
		if (s_gb->rom.romheader.cartridgeType == 1) {
			MCB_romBanking &= 0xe0;
			MCB_romBanking |= low5;
			printf("Lo BANK change. value => %x\n", MCB_romBanking);
			}
	} else if (addr >= 0x4000 && addr < 0x6000) {
		/* hiRom bank change */
		if (romBankingFlag > 0) {
			MCB_romBanking &= 0x1f;
			value &= 0xe0;
			MCB_romBanking |= value;
			/*
			 * printf("Hi BANK change. value => %x\n",
			 *		MCB_romBanking);
			 */

		}
	} else if (addr >= 0x6000 && addr < 0x8000) {
		/* change rom/ram bank */
		romBankingFlag = ((value & 0x01) == 0) ? 1 : 0;

	}
	if (MCB_romBanking == 0)
		MCB_romBanking = 1;
}

int write8bit(uint16_t addr, uint8_t value, struct gb *s_gb)
{
	if (addr == 0xffffu)
		puts("IE");
	if (addr < 0x8000) {
		mcbHandleBanking(addr, value, s_gb);
		return 0;
	} else if (addr >= 0x8000 && addr < 0xA000) {
		s_gb->memory.vram[addr - 0x8000] = value;
		return 0;
	} else if (addr >= 0xA000 && addr < 0xC000) {
		s_gb->memory.sram[addr - 0xA000] = value;
		return 0;
	} else if (addr >= 0xC000 && addr < 0xE000) {
		s_gb->memory.ram[addr - 0xC000] = value;
		return 0;
	} else if (addr >= 0xE000 && addr < 0xFE00) {
		s_gb->memory.ram[addr - 0xE000] = value;
		return 0;
	} else if (addr >= 0xFE00 && addr < SPECIAL_REGISTER_FIRST) {
		s_gb->memory.oam[addr - 0xFE00] = value;
		return 0;
	} else if (addr >= SPECIAL_REGISTER_FIRST
			&& addr < SPECIAL_REGISTER_HIGH_RAM_START) {
		s_gb->memory.io_ports[addr - SPECIAL_REGISTER_FIRST] = value;
		if (addr == SPECIAL_REGISTER_STAT)
			printf("writing lcd stat %x\n", value);
		ctrlIo(addr, (unsigned char *) s_gb->memory.io_ports, s_gb);
		return 0;
	} else if (addr >= SPECIAL_REGISTER_HIGH_RAM_START
			&& addr < SPECIAL_REGISTER_HIGH_RAM_END) {
		/* TODO replace the above test by an in_hram function */
		s_gb->memory.hram[addr - SPECIAL_REGISTER_HIGH_RAM_START] =
				value;
		return 0;
	} else if (addr == SPECIAL_REGISTER_IE) {
		printf("%s interEnable = %"PRIu8"\n", __func__, value);
		s_gb->interrupts.interEnable = value;
		return 0;
	}
	/* never reached */
	return -1;
}

void push(uint16_t value, struct gb *s_gb)
{
	s_gb->registers.sp -= 2;
	write16bitToAddr(s_gb->registers.sp, value, s_gb);
}

uint16_t pop(struct gb *s_gb)
{
	uint16_t value;

	value = read16bit(s_gb->registers.sp, s_gb);
	s_gb->registers.sp += 2;

	return value;
}
