#include <stdio.h>

#include "GB.h"

#define DEFAULT_ROMPATH "C:\\proj\\GB_test\\emgb\\roms\\Tetris.gb"

int main(int ac, char **av)
{
	const char *rompath;

#if EMGB_CONSOLE_DEBUGGER
	puts("console debugger enabled");
#endif /* EMGB_CONSOLE_DEBUGGER */

	rompath = ac == 1 ? DEFAULT_ROMPATH : av[1];

	gb(rompath);

	return EXIT_SUCCESS;
}
