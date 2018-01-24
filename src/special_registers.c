#include "special_registers.h"
#include "utils.h"

const char *special_register_to_str(enum special_register reg)
{
	switch (reg) {
	case SPECIAL_REGISTER_P1:
		return "p1";
	case SPECIAL_REGISTER_SB:
		return "sb";
	case SPECIAL_REGISTER_SC:
		return "sc";
	case SPECIAL_REGISTER_DIV:
		return "div";
	case SPECIAL_REGISTER_TIMA:
		return "tima";
	case SPECIAL_REGISTER_TMA:
		return "tma";
	case SPECIAL_REGISTER_TAC:
		return "tac";
	case SPECIAL_REGISTER_IF:
		return "if";
	case SPECIAL_REGISTER_NR10:
		return "nr10";
	case SPECIAL_REGISTER_NR11:
		return "nr11";
	case SPECIAL_REGISTER_NR12:
		return "nr12";
	case SPECIAL_REGISTER_NR13:
		return "nr13";
	case SPECIAL_REGISTER_NR14:
		return "nr14";
	case SPECIAL_REGISTER_NR21:
		return "nr21";
	case SPECIAL_REGISTER_NR22:
		return "nr22";
	case SPECIAL_REGISTER_NR23:
		return "nr23";
	case SPECIAL_REGISTER_NR24:
		return "nr24";
	case SPECIAL_REGISTER_NR30:
		return "nr30";
	case SPECIAL_REGISTER_NR31:
		return "nr31";
	case SPECIAL_REGISTER_NR32:
		return "nr32";
	case SPECIAL_REGISTER_NR33:
		return "nr33";
	case SPECIAL_REGISTER_NR34:
		return "nr34";
	case SPECIAL_REGISTER_NR41:
		return "nr41";
	case SPECIAL_REGISTER_NR42:
		return "nr42";
	case SPECIAL_REGISTER_NR43:
		return "nr43";
	case SPECIAL_REGISTER_NR44:
		return "nr44";
	case SPECIAL_REGISTER_NR50:
		return "nr50";
	case SPECIAL_REGISTER_NR51:
		return "nr51";
	case SPECIAL_REGISTER_NR52:
		return "nr52";
	case SPECIAL_REGISTER_WPRAM0:
		return "wpram0";
	case SPECIAL_REGISTER_WPRAM1:
		return "wpram1";
	case SPECIAL_REGISTER_WPRAM2:
		return "wpram2";
	case SPECIAL_REGISTER_WPRAM3:
		return "wpram3";
	case SPECIAL_REGISTER_WPRAM4:
		return "wpram4";
	case SPECIAL_REGISTER_WPRAM5:
		return "wpram5";
	case SPECIAL_REGISTER_WPRAM6:
		return "wpram6";
	case SPECIAL_REGISTER_WPRAM7:
		return "wpram7";
	case SPECIAL_REGISTER_WPRAM8:
		return "wpram8";
	case SPECIAL_REGISTER_WPRAM9:
		return "wpram9";
	case SPECIAL_REGISTER_WPRAMA:
		return "wprama";
	case SPECIAL_REGISTER_WPRAMB:
		return "wpramb";
	case SPECIAL_REGISTER_WPRAMC:
		return "wpramc";
	case SPECIAL_REGISTER_WPRAMD:
		return "wpramd";
	case SPECIAL_REGISTER_WPRAME:
		return "wprame";
	case SPECIAL_REGISTER_WPRAMF:
		return "wpramf";
	case SPECIAL_REGISTER_LCDC:
		return "lcdc";
	case SPECIAL_REGISTER_STAT:
		return "stat";
	case SPECIAL_REGISTER_SCY:
		return "scy";
	case SPECIAL_REGISTER_SCX:
		return "scx";
	case SPECIAL_REGISTER_LY:
		return "ly";
	case SPECIAL_REGISTER_LYC:
		return "lyc";
	case SPECIAL_REGISTER_DMA:
		return "dma";
	case SPECIAL_REGISTER_BGP:
		return "bgp";
	case SPECIAL_REGISTER_OBP0:
		return "obp0";
	case SPECIAL_REGISTER_OBP1:
		return "obp1";
	case SPECIAL_REGISTER_WY:
		return "wy";
	case SPECIAL_REGISTER_WX:
		return "wx";
	case SPECIAL_REGISTER_HIGH_RAM_START:
		return "high_ram_start";
	case SPECIAL_REGISTER_HIGH_RAM_END:
		return "high_ram_end";
	case SPECIAL_REGISTER_IE:
		return "ie";
	default:
		return "(unknown)";
	}
}

enum special_register special_register_from_string(const char *str)
{
	enum special_register reg;

	if (str_matches(str, "(unknown)"))
		return SPECIAL_REGISTER_INVALID;

	for (reg = SPECIAL_REGISTER_FIRST; reg <= SPECIAL_REGISTER_LAST;
			reg++)
		if (str_matches(str, special_register_to_str(reg)))
			return reg;

	return SPECIAL_REGISTER_INVALID;
}
