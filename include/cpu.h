#ifndef __CPU__
#define __CPU__
#include <stdbool.h>
#include <inttypes.h>

#include "cpu_op.h"

struct s_gb;
void initCpu(struct s_gb * s_gb);
bool is_opcode_undefined(uint8_t opcode);

#pragma pack(push, 1)

struct s_register
{
  struct {
    union  {
      struct {
	unsigned char f;
	unsigned char a;
      };
			unsigned short af;
    };
  };
  struct {
    union {
      struct {
	unsigned char c;
	unsigned char b;
      };
      unsigned short bc;
    };
  };
  
  struct {
    union {
      struct {
	unsigned char e;
	unsigned char d;
      };
      unsigned short de;
    };
  };
  
  struct {
    union {
      struct {
	unsigned char l;
	unsigned char h;
      };
      unsigned short hl;
    };
  };

  unsigned short	pc;
  unsigned short	sp;
  //unsigned char	flags;
};

struct extendedInstruction {
	char *disassembly;
	void(*execute)(struct s_gb *);
	//unsigned char ticks;
};

extern const struct s_cpu_z80 instructions[256];
extern const struct extendedInstruction extendedInstructions[256];

struct	       			s_cpu {
	unsigned int		totalTick;
	unsigned char		stopCpu;
	unsigned char		jmpf; // TODO remove
};

#pragma pack(pop)


#define SET_ZERO() s_gb->gb_register.f |= 0x80
#define CLEAR_ZERO() s_gb->gb_register.f &= ~(0x80)


#define SET_NEG() s_gb->gb_register.f |= 0x40
#define CLEAR_NEG() s_gb->gb_register.f &= ~(0x40)


#define SET_HALFC() s_gb->gb_register.f |= 0x20
#define CLEAR_HALFC() s_gb->gb_register.f &= ~(0x20)


#define SET_CARRY() s_gb->gb_register.f |= 0x10
#define CLEAR_CARRY() s_gb->gb_register.f &= ~(0x10)


#define ZERO_FLAG (s_gb->gb_register.f & 0x80)
#define CARRY_FLAG (s_gb->gb_register.f & 0x10)
#define NEG_FLAG (s_gb->gb_register.f & 0x40)
#define HALFC_FLAG (s_gb->gb_register.f & 0x20)

#endif
