#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "joypad.h"
#include "special_registers.h"
#include "rom.h"
#include "io.h"

static void mcb_handle_banking(struct memory *memory, uint16_t addr,
		uint8_t value)
{
	char low5;

	low5 = value & 0x1f;
	if (addr >= 0x2000 && addr < 0x4000) {
		if (memory->rom_bank_0_rom.rom_header.cartridge_type == 1) {
			memory->mbc_rom_bank &= 0xe0;
			memory->mbc_rom_bank |= low5;
			/*
			 * printf("Lo BANK change. value => %x\n",
			 *		memory->mcb_rom_banking);
			*/
		}
	} else if (addr >= 0x4000 && addr < 0x6000) {
		/* hiRom bank change */
		if (memory->rom_banking_flag) {
			memory->mbc_rom_bank &= 0x1f;
			value &= 0xe0;
			memory->mbc_rom_bank |= value;
			/*
			 * printf("Hi BANK change. value => %x\n",
			 *		MCB_romBanking);
			 */

		}
	} else if (addr >= 0x6000 && addr < 0x8000) {
		/* change rom/ram bank */
		memory->rom_banking_flag = !(value & 0x01);

	}
	if (memory->mbc_rom_bank == 0)
		memory->mbc_rom_bank = 1;
}

void memory_init(struct memory *memory, struct timer *timer)
{
	memset(memory, 0, sizeof(*memory));
	memory->mbc_rom_bank = 1;
	memory->timer = timer;

	write8bit(memory, 0xFF05, 0x00);
	write8bit(memory, 0xFF06, 0x00);
	write8bit(memory, 0xFF07, 0x08);
	write8bit(memory, 0xFF10, 0x80);
	write8bit(memory, 0xFF11, 0xBF);
	write8bit(memory, 0xFF12, 0xF3);
	write8bit(memory, 0xFF14, 0xBF);
	write8bit(memory, 0xFF16, 0x3F);
	write8bit(memory, 0xFF17, 0x00);
	write8bit(memory, 0xFF19, 0xBF);
	write8bit(memory, 0xFF1A, 0x7F);
	write8bit(memory, 0xFF1B, 0xFF);
	write8bit(memory, 0xFF1C, 0x9F);
	write8bit(memory, 0xFF1E, 0xBF);
	write8bit(memory, 0xFF20, 0xFF);
	write8bit(memory, 0xFF21, 0x00);
	write8bit(memory, 0xFF22, 0x00);
	write8bit(memory, 0xFF23, 0xBF);
	write8bit(memory, 0xFF24, 0x77);
	write8bit(memory, 0xFF25, 0xF3);
	write8bit(memory, 0xFF26, 0xF1);
	write8bit(memory, 0xFF40, 0x91);
	write8bit(memory, 0xFF42, 0x00);
	write8bit(memory, 0xFF43, 0x00);
	write8bit(memory, 0xFF45, 0x00);
	write8bit(memory, 0xFF47, 0xFC);
	write8bit(memory, 0xFF48, 0xFF);
	write8bit(memory, 0xFF49, 0xFF);
	write8bit(memory, 0xFF4A, 0x00);
	write8bit(memory, 0xFF4B, 0x00);
	write8bit(memory, 0xFFFF, 0x00);
}

void write16bit(struct memory *memory, uint16_t addr, uint16_t value)
{
	write8bit(memory, addr, value & 0x00ffu);
	write8bit(memory, addr + 1, (value & 0xff00u) >> 8);
}

uint16_t read16bit(struct memory *memory, uint16_t addr)
{
	uint16_t res;

	res = read8bit(memory, addr);
	res |= read8bit(memory, addr + 1) << 8;

	return res;
}

static void refresh_memory(struct memory *mem, uint16_t addr)
{
	if (addr == SPECIAL_REGISTER_DIV)
		/* TODO, doesn't correspond to the documentation */
		mem->spec_reg.div = rand();
}

static bool in_switchable_rom_bank(uint16_t addr)
{
	return addr >= 0x4000 && addr < 0x8000;
}

uint8_t read8bit(struct memory *mem, uint16_t addr)
{
	refresh_memory(mem, addr);
	if (in_switchable_rom_bank(addr) && mem->mbc_rom_bank != 0)
		return mem->extra_rom_banks[(mem->mbc_rom_bank - 2)
				* ROM_BANK_SIZE + addr];

	return mem->raw[addr];
}

void write8bit(struct memory *memory, uint16_t addr, uint8_t value)
{
	if (addr < 0x8000)
		mcb_handle_banking(memory, addr, value);
	else if (addr >= 0xFF00 && addr < 0xFF4C)
		io_ctrl(memory, memory->timer, addr);
	memory->raw[addr] = value;
}

void push(struct memory *memory, uint16_t *sp, uint16_t value)
{
	*sp -= 2;
	write16bit(memory, *sp, value);
}

uint16_t pop(struct memory *memory, uint16_t *sp)
{
	uint16_t value;

	value = read16bit(memory, *sp);
	*sp += 2;

	return value;
}
