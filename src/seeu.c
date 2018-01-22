#include "GB.h"
#include "config.h"

void seeu(struct gb *s_gb)
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
