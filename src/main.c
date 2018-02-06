#include <stdio.h>
#include <inttypes.h>

#include "gb.h"
#include "log.h"
#include "console_debugger.h"
#include "instructions.h"

/*
 *  main loop function
 * retrieve opcode & execute it. update gpu interrupt & timer
 */

/* int imgui(void *p_s_gb); */

static void gb_loop(const char *file_name)
{
	struct gb *gb;
	uint8_t fopcode;
	const struct cpu_op *instruction;
	struct cpu *cpu;
	struct registers *registers;
	struct memory *memory;
	struct joypad *joypad;
	unsigned cycles = 0;
#ifdef IMDBG
	SDL_Thread *thr;
#endif
#if EMGB_CONSOLE_DEBUGGER
	struct console_debugger debugger;
	int ret;
#endif /* EMGB_CONSOLE_DEBUGGER */

	gb = gb_init(file_name);
	memory = &gb->memory;
	cpu = &gb->cpu;
	registers = &gb->registers;
	joypad = &gb->joypad;
#if EMGB_CONSOLE_DEBUGGER
	ret = console_debugger_init(&debugger, registers, memory,
			&gb->config.config, file_name);
	if (ret < 0)
		ERR("console_debugger_init: %s", strerror(-ret));
	console_debugger_register_key_action(&debugger, &gb->save);
	console_debugger_register_key_action(&debugger, &gb->restore);
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
				cycles = instruction->func(gb);
				cpu->total_tick += cycles;
				registers->pc += instruction->size;
			}
			gpu_update(&gb->gpu);
		}
		interrupt_update(&gb->interrupts);
		timer_update(&gb->timer, cycles);
		memory_update(&gb->memory, cycles);
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
	const char *file_name;

#if EMGB_CONSOLE_DEBUGGER
	puts("console debugger enabled");
#endif /* EMGB_CONSOLE_DEBUGGER */

	file_name = ac == 1 ? "-" : av[1];

	gb_loop(file_name);

	return EXIT_SUCCESS;
}
