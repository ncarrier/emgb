#ifndef INCLUDE_VIDEO_COMMON_H_
#define INCLUDE_VIDEO_COMMON_H_
#include <stdbool.h>

#define GB_W 160
#define GB_H 144
#define GB_SURF (GB_W * GB_H)

bool is_window_fullscreen(int width, int height);

#endif /* INCLUDE_VIDEO_COMMON_H_ */
