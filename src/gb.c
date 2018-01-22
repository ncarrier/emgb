#include "GB.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */
#include "instructions.h"

/*
 *  main loop function
 * retrieve opcode & execute it. update gpu interrupt & timer
 */

/* int imgui(void *p_s_gb); */

void gb(const char *fileName)
{
	struct gb *gb;
	uint8_t fopcode;
	const struct cpu_op *instruction;
	struct cpu *cpu;
	struct registers *registers;
#ifdef IMDBG
	SDL_Thread *thr;
#endif
#if EMGB_CONSOLE_DEBUGGER
	struct console_debugger debugger;
	int ret;
#endif /* EMGB_CONSOLE_DEBUGGER */

	gb = gb_init(fileName);
	cpu = &gb->cpu;
	registers = &gb->registers;
#if EMGB_CONSOLE_DEBUGGER
	ret = console_debugger_init(&debugger, registers, gb, &gb->config.config);
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
	while (gb->running) {
#if EMGB_CONSOLE_DEBUGGER
		ret = console_debugger_update(&debugger);
		if (ret < 0)
			ERR("console_debugger_update: %s", strerror(-ret));
#endif /* EMGB_CONSOLE_DEBUGGER */
		handleEvent(gb);
		if (!cpu->stopped) {
			if (!cpu->halted) {
				fopcode = read8bit(registers->pc, gb);
				instruction = instructions_base + fopcode;
				cpu->totalTick += instruction->func(gb);
				registers->pc += instruction->size;
			}
			updateGpu(gb);
		}
		doInterupt(gb);
		updateTimer(gb);
	}
#ifdef IMDBG
	if (thr != NULL) {
		gb->stopdbg = 1;
		printf("Waiting dbg thread to exit\n");
		SDL_WaitThread(thr, NULL);
	}
#endif
	gb_cleanup(gb);
}

void gb_cleanup(struct gb *s_gb)
{
	config_cleanup(&s_gb->config);
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyWindow(s_gb->gpu.window);
	free(s_gb->rom.rom);
	/* free(s_gb->gb_gpu.pixels_d); */
	free(s_gb->gpu.pixels);
	free(s_gb);

	SDL_Quit();
}

