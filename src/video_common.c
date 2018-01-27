#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include "video_common.h"

/* TODO factor with gpu */
bool is_window_fullscreen(int width, int height)
{
	int ret;
	SDL_DisplayMode dm;

	ret = SDL_GetCurrentDisplayMode(0, &dm);
	if (ret != 0)
		return false;

	return dm.w <= width && dm.h <= height;
}
