#include "../include/memory.h"

#include "gb.h"
#include "special_registers.h"

static void mcbHandleBanking(struct memory *memory, uint16_t addr,
		uint8_t value)
{
	char low5;

	low5 = value & 0x1f;
	if (addr >= 0x2000 && addr < 0x4000) {
		if (memory->cartridge_type == 1) {
			memory->mcb_rom_banking &= 0xe0;
			memory->mcb_rom_banking |= low5;
			printf("Lo BANK change. value => %x\n",
					memory->mcb_rom_banking);
		}
	} else if (addr >= 0x4000 && addr < 0x6000) {
		/* hiRom bank change */
		if (memory->rom_banking_flag) {
			memory->mcb_rom_banking &= 0x1f;
			value &= 0xe0;
			memory->mcb_rom_banking |= value;
			/*
			 * printf("Hi BANK change. value => %x\n",
			 *		MCB_romBanking);
			 */

		}
	} else if (addr >= 0x6000 && addr < 0x8000) {
		/* change rom/ram bank */
		memory->rom_banking_flag = !(value & 0x01);

	}
	if (memory->mcb_rom_banking == 0)
		memory->mcb_rom_banking = 1;
}

void memory_init(struct memory *memory, struct gb *gb, uint8_t cartridge_type)
{
	memset(memory, 0, sizeof(*memory));
	memory->cartridge_type = cartridge_type;
	memory->mcb_rom_banking = 1;

	write8bit(0xFF05,  0x00, gb);
	write8bit(0xFF06,  0x00, gb);
	write8bit(0xFF07,  0x08, gb);
	write8bit(0xFF10,  0x80, gb);
	write8bit(0xFF11,  0xBF, gb);
	write8bit(0xFF12,  0xF3, gb);
	write8bit(0xFF14,  0xBF, gb);
	write8bit(0xFF16,  0x3F, gb);
	write8bit(0xFF17,  0x00, gb);
	write8bit(0xFF19,  0xBF, gb);
	write8bit(0xFF1A,  0x7F, gb);
	write8bit(0xFF1B,  0xFF, gb);
	write8bit(0xFF1C,  0x9F, gb);
	write8bit(0xFF1E,  0xBF, gb);
	write8bit(0xFF20,  0xFF, gb);
	write8bit(0xFF21,  0x00, gb);
	write8bit(0xFF22,  0x00, gb);
	write8bit(0xFF23,  0xBF, gb);
	write8bit(0xFF24,  0x77, gb);
	write8bit(0xFF25,  0xF3, gb);
	write8bit(0xFF26,  0xF1, gb);
	write8bit(0xFF40,  0x91, gb);
	write8bit(0xFF42,  0x00, gb);
	write8bit(0xFF43,  0x00, gb);
	write8bit(0xFF45,  0x00, gb);
	write8bit(0xFF47,  0xFC, gb);
	write8bit(0xFF48,  0xFF, gb);
	write8bit(0xFF49,  0xFF, gb);
	write8bit(0xFF4A,  0x00, gb);
	write8bit(0xFF4B,  0x00, gb);
	write8bit(0xFFFF,  0x00, gb);
}

void write16bitToAddr(uint16_t addr, uint16_t value, struct gb *gb)
{
	write8bit(addr, value & 0x00ffu, gb);
	write8bit(addr + 1, (value & 0xff00u) >> 8, gb);
}

uint16_t read16bit(uint16_t addr, struct gb *gb)
{
	uint16_t res;

	res = read8bit(addr, gb);
	res |= read8bit(addr + 1, gb) << 8;

	return res;
}

uint8_t read8bit(uint16_t addr, struct gb *gb)
{
	struct memory *memory;

	memory = &s_gb->memory;
	if (addr == 0xff44) {
		return s_gb->gpu.scanline;
	} else if (addr < 0x4000) {
		return s_gb->rom.rom[addr];
	} else if (addr >= 0x4000 && addr < 0x8000) {
		/* printf("MCB_romBanking value = %x\n", MCB_romBanking); */
		return s_gb->rom.rom[(addr - 0x4000)
				+ (memory->mcb_rom_banking * 0x4000)];
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

int write8bit(uint16_t addr, uint8_t value, struct gb *gb)
{
	if (addr == 0xffffu)
		puts("IE");
	if (addr < 0x8000) {
		mcbHandleBanking(&s_gb->memory, addr, value);
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

void push(uint16_t value, struct gb *gb)
{
	gb->registers.sp -= 2;
	write16bitToAddr(gb->registers.sp, value, gb);
}

uint16_t pop(struct gb *gb)
{
	uint16_t value;

	value = read16bit(gb->registers.sp, gb);
	gb->registers.sp += 2;

	return value;
}
