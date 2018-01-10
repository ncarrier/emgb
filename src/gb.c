#include "GB.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */
#include "instructions.h"

// main loop function
// retrieve opcode & execute it. update gpu interrupt & timer

int imgui(void *p_s_gb);

static bool print_log = false;

static void log_instruction(struct s_gb *s_gb,
		const struct s_cpu_z80 *instruction, uint16_t pc,
		uint8_t fopcode, unsigned size)
{
	unsigned i;

	printf("[%#"PRIx8"] (%#"PRIx8") %s   ", pc, fopcode,
			instructions_base[fopcode].value);
	for (i = 1; i < size; i++)
		printf("%#"PRIx8" ", read8bit(pc + i, s_gb));
	puts("");
}

static void instruction_gen(struct s_gb *s_gb)
{
	uint8_t fopcode;
	const struct s_cpu_z80 *instruction;

	fopcode = read8bit(s_gb->gb_register.pc, s_gb);
	instruction = instructions_base + fopcode;
	if (print_log)
		log_instruction(s_gb, instruction, s_gb->gb_register.pc,
				fopcode, instruction->real_size);
	instruction->func(s_gb);
	s_gb->gb_register.pc += instruction->size;
	s_gb->gb_cpu.totalTick += instruction->cycles;
}

void gb(char *fileName)
{
	struct s_gb		*s_gb = NULL;
#ifdef IMDBG
	SDL_Thread		*thr = NULL;
#endif
#if EMGB_CONSOLE_DEBUGGER
	struct console_debugger debugger;
	int ret;
#endif /* EMGB_CONSOLE_DEBUGGER */

	s_gb = initGb(fileName);
#if EMGB_CONSOLE_DEBUGGER
	ret = console_debugger_init(&debugger, &s_gb->gb_register, s_gb);
	if (ret < 0)
		ERR("console_debugger_init: %s", strerror(-ret));
#endif /* EMGB_CONSOLE_DEBUGGER */
	s_gb->stopdbg = 0;
#ifdef IMDBG
	s_gb->stopdbg = 0;
	thr = SDL_CreateThread(imgui, "dbg", s_gb);
	if (thr == NULL)
	{
		printf("cannot start imGui dbg\n");
	}
#endif
	while (s_gb->running)
	{
#if EMGB_CONSOLE_DEBUGGER
          ret = console_debugger_update(&debugger);
          if (ret < 0)
            ERR("console_debugger_update: %s", strerror(-ret));
#endif /* EMGB_CONSOLE_DEBUGGER */
	  /* debug(s_gb); */
	  handleEvent(s_gb);
	  if (s_gb->gb_cpu.stopCpu == 0)
				instruction_gen(s_gb);
	  updateGpu(s_gb);
	  doInterupt(s_gb);
	  updateTimer(s_gb);
	}
#ifdef IMDBG
	if (thr != NULL)
	{
		s_gb->stopdbg = 1;
		printf("Waiting dbg thread to exit\n");
		SDL_WaitThread(thr, NULL);
	}
#endif
	seeu(s_gb);
}
