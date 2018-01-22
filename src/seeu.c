#include "GB.h"
#include "config.h"

void seeu(struct gb *s_gb)
{
	config_cleanup(&s_gb->config);
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyWindow(s_gb->gb_gpu.window);
	free(s_gb->gb_rom.rom);
	/* free(s_gb->gb_gpu.pixels_d); */
	free(s_gb->gb_gpu.pixels);
	free(s_gb);

	SDL_Quit();
}

void displayStack(struct gb *s_gb)
{
	unsigned short value;

	value = s_gb->gb_register.sp + 10;
	while (value >= s_gb->gb_register.sp) {
		printf("sp <%x> value <%x>\n", value, read16bit(value, s_gb));
		value -= 2;
	}
}

void RDBG(struct gb *s_gb)
{
	/*
	printf("----------\npc value : %s pc: %x\nsp: %x\nopcode: %x\n"
			"----------\n",
			instructions[read8bit(s_gb->gb_register.pc,
					s_gb)].value,
			s_gb->gb_register.pc, s_gb->gb_register.sp,
			read8bit(s_gb->gb_register.pc, s_gb));

	printf("register a %x f %x af -- %x\n", s_gb->gb_register.a,
			s_gb->gb_register.f, s_gb->gb_register.af);
	printf("register b %x c %x cb -- %x\n", s_gb->gb_register.b,
			s_gb->gb_register.c, s_gb->gb_register.bc);
	printf("register d %x e %x de -- %x\n", s_gb->gb_register.d,
			s_gb->gb_register.e, s_gb->gb_register.de);
	printf("register h %x l %x hl -- %x\n", s_gb->gb_register.h,
			s_gb->gb_register.l, s_gb->gb_register.hl);
	printf("flag Zero %x\n", s_gb->gb_register.f >> 7);
	printf("flag Substraction %x\n", (s_gb->gb_register.f >> 6) & 1);
	printf("flag Half-carry %x\n", (s_gb->gb_register.f >> 5) & 1);
	printf("flag  Carry %x\n", (s_gb->gb_register.f >> 4) & 1);
	*/

	displayStack(s_gb);
	/* printf("----end-----"); */
	/* printf("DEBUG\n"); */
	/* getchar(); */
}

void debug(struct gb *s_gb)
{
	static int i;

	if (s_gb->gb_register.pc >= 0x1400)
		i++;
		/* getchar(); */
	/*
	 * if (i > 0)
	 *	RDBG(s_gb);
	 */
}
