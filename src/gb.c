#include "GB.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

// main loop function
// retrieve opcode & execute it. update gpu interupt & timer

int imgui(void *p_s_gb);



void gb(char *fileName)
{
	unsigned char	fopcode = 0;
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
	    {
	      fopcode = read8bit(s_gb->gb_register.pc, s_gb); s_gb->gb_register.pc += 1; //retrieve func opcode
	      s_gb->gb_cpu.gb_cpu_z80[fopcode].func(s_gb); //call opcode func pointer
	      if (s_gb->gb_cpu.jmpf == 0) //if jmp opcode was called, no need to incr PC
		s_gb->gb_register.pc += s_gb->gb_cpu.gb_cpu_z80[fopcode].size;
	      s_gb->gb_cpu.jmpf = 0;
	    }
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
