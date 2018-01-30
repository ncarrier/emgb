#include <stdio.h>
#include <inttypes.h>

#include "gb.h"
#include "log.h"
#include "console_debugger.h"
#include "instructions.h"

#define DEFAULT_ROMPATH "C:\\proj\\GB_test\\emgb\\roms\\Tetris.gb"

/*
 *  main loop function
 * retrieve opcode & execute it. update gpu interrupt & timer
 */

/* int imgui(void *p_s_gb); */

static void gb_loop(const char *fileName)
{
	struct gb *gb;
	uint8_t fopcode;
	const struct cpu_op *instruction;
	struct cpu *cpu;
	struct registers *registers;
	struct memory *memory;
	struct joypad *joypad;
#ifdef IMDBG
	SDL_Thread *thr;
#endif
#if EMGB_CONSOLE_DEBUGGER
	struct console_debugger debugger;
	int ret;
#endif /* EMGB_CONSOLE_DEBUGGER */

	gb = gb_init(fileName);
	memory = &gb->memory;
	cpu = &gb->cpu;
	registers = &gb->registers;
	joypad = &gb->joypad;
#if EMGB_CONSOLE_DEBUGGER
	ret = console_debugger_init(&debugger, registers, memory,
			&gb->config.config);
	if (ret < 0)
		ERR("console_debugger_init: %s", strerror(-ret));
#endif /* EMGB_CONSOLE_DEBUGGER */
	gb->stopdbg = false;
#ifdef IMDBG
	gb->stopdbg = 0;
	thr = SDL_CreateThread(imgui, "dbg", gb);
	if (thr == NULL)
		printf("cannot start imGui dbg\n");
#endif
	while (joypad_is_running(joypad)) {
#if EMGB_CONSOLE_DEBUGGER
		ret = console_debugger_update(&debugger);
		if (ret < 0)
			ERR("console_debugger_update: %s", strerror(-ret));
#endif /* EMGB_CONSOLE_DEBUGGER */
		joypad_handle_event(joypad);
		if (!cpu->stopped) {
			if (!cpu->halted) {
				fopcode = read8bit(memory, registers->pc);
				instruction = instructions_base + fopcode;
				cpu->total_tick += instruction->func(gb);
				registers->pc += instruction->size;
			}
			gpu_update(&gb->gpu);
		}
		interrupt_update(&gb->interrupts);
		timer_update(&gb->timer);
	}
#ifdef IMDBG
	if (thr != NULL) {
		gb->stopdbg = 1;
		printf("Waiting dbg thread to exit\n");
		SDL_WaitThread(thr, NULL);
	}
#endif
	console_debugger_cleanup(&debugger);
	gb_cleanup(gb);
}

int main(int ac, char **av)
{
	const char *rompath;

#if EMGB_CONSOLE_DEBUGGER
	puts("console debugger enabled");
#endif /* EMGB_CONSOLE_DEBUGGER */

	rompath = ac == 1 ? DEFAULT_ROMPATH : av[1];

	gb_loop(rompath);

	return EXIT_SUCCESS;
}
