#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_
#include <limits.h>

#include "ae_config.h"

#define CONFIG_DIR ".emgb/"

#define CONFIG_DEBUGGER_ACTIVE "debugger_active"
#define CONFIG_LINEAR_SCALING "linear_scaling"
#define CONFIG_WINDOW_WIDTH "window_width"
#define CONFIG_WINDOW_HEIGHT "window_height"
#define CONFIG_WINDOW_X "window_x"
#define CONFIG_WINDOW_Y "window_y"
#define CONFIG_COLOR_0 "color_0"
#define CONFIG_COLOR_1 "color_1"
#define CONFIG_COLOR_2 "color_2"
#define CONFIG_COLOR_3 "color_3"
#define CONFIG_JOYPAD_0_RIGHT "joypad_0_right"
#define CONFIG_JOYPAD_0_LEFT "joypad_0_left"
#define CONFIG_JOYPAD_0_UP "joypad_0_up"
#define CONFIG_JOYPAD_0_DOWN "joypad_0_down"
#define CONFIG_JOYPAD_0_A "joypad_0_a"
#define CONFIG_JOYPAD_0_B "joypad_0_b"
#define CONFIG_JOYPAD_0_SELECT "joypad_0_select"
#define CONFIG_JOYPAD_0_START "joypad_0_start"

#define CONFIG_DEBUGGER_ACTIVE_DEFAULT 0
#define CONFIG_LINEAR_SCALING_DEFAULT 0
#define CONFIG_WINDOW_WIDTH_DEFAULT 160
#define CONFIG_WINDOW_HEIGHT_DEFAULT 144
#define CONFIG_WINDOW_X_DEFAULT 300
#define CONFIG_WINDOW_Y_DEFAULT 300
#define CONFIG_COLOR_0_DEFAULT 0x00000000
#define CONFIG_COLOR_1_DEFAULT 0x00444444
#define CONFIG_COLOR_2_DEFAULT 0x00aaaaaa
#define CONFIG_COLOR_3_DEFAULT 0x00ffffff
#define CONFIG_JOYPAD_0_RIGHT_DEFAULT SDLK_RIGHT
#define CONFIG_JOYPAD_0_LEFT_DEFAULT SDLK_LEFT
#define CONFIG_JOYPAD_0_UP_DEFAULT SDLK_UP
#define CONFIG_JOYPAD_0_DOWN_DEFAULT SDLK_DOWN
#define CONFIG_JOYPAD_0_A_DEFAULT SDLK_w
#define CONFIG_JOYPAD_0_B_DEFAULT SDLK_x
#define CONFIG_JOYPAD_0_SELECT_DEFAULT SDLK_c
#define CONFIG_JOYPAD_0_START_DEFAULT SDLK_v

struct config {
	char dir[PATH_MAX];
	char *file;
	struct ae_config config;
};

int config_init(struct config *config);
int config_write(struct config *config);
void config_cleanup(struct config *config);

#endif /* INCLUDE_CONFIG_H_ */
