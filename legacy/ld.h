#ifndef SRC_LD_H_
#define SRC_LD_H_

#include "GB.h"
#include "gpu.h"

static inline void ld_b_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.b; }
static inline void ld_b_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.c; }
static inline void ld_b_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.d; }
static inline void ld_b_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.e; }
static inline void ld_b_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.h; }
static inline void ld_b_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.l; }
static inline void ld_b_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.b = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_b_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.b = s_gb->gb_register.a; }

static inline void ld_c_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.b; }
static inline void ld_c_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.c; }
static inline void ld_c_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.d; }
static inline void ld_c_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.e; }
static inline void ld_c_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.h; }
static inline void ld_c_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.l; }
static inline void ld_c_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.c = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_c_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.c = s_gb->gb_register.a; }

static inline void ld_d_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.b; }
static inline void ld_d_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.c; }
static inline void ld_d_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.d; }
static inline void ld_d_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.e; }
static inline void ld_d_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.h; }
static inline void ld_d_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.l; }
static inline void ld_d_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.d = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_d_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.d = s_gb->gb_register.a; }

static inline void ld_e_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.b; }
static inline void ld_e_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.c; }
static inline void ld_e_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.d; }
static inline void ld_e_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.e; }
static inline void ld_e_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.h; }
static inline void ld_e_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.l; }
static inline void ld_e_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.e = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_e_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.e = s_gb->gb_register.a; }

static inline void ld_h_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.b; }
static inline void ld_h_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.c; }
static inline void ld_h_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.d; }
static inline void ld_h_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.e; }
static inline void ld_h_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.h; }
static inline void ld_h_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.l; }
static inline void ld_h_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.h = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_h_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.h = s_gb->gb_register.a; }

static inline void ld_l_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.b; }
static inline void ld_l_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.c; }
static inline void ld_l_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.d; }
static inline void ld_l_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.e; }
static inline void ld_l_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.h; }
static inline void ld_l_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.l; }
static inline void ld_l_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.l = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_l_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.l = s_gb->gb_register.a; }

static inline void ld_hl_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.b, s_gb); }
static inline void ld_hl_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.c, s_gb); }
static inline void ld_hl_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.d, s_gb); }
static inline void ld_hl_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.e, s_gb); }
static inline void ld_hl_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.h, s_gb); }
static inline void ld_hl_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.l, s_gb); }
static inline void ld_hl_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; write8bit(s_gb->gb_register.hl, s_gb->gb_register.a, s_gb); }

static inline void ld_a_b(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.b; }
static inline void ld_a_c(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.c; }
static inline void ld_a_d(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.d; }
static inline void ld_a_e(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.e; }
static inline void ld_a_h(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.h; }
static inline void ld_a_l(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.l; }
static inline void ld_a_hl(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 8; s_gb->gb_register.a = read8bit(s_gb->gb_register.hl, s_gb); }
static inline void ld_a_a(struct s_gb *s_gb) { s_gb->gb_cpu.totalTick += 4; s_gb->gb_register.a = s_gb->gb_register.a; }

#endif /* SRC_LD_H_ */
