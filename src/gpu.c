#include "gpu.h"
#include "cpu.h"
#include "memory.h"
#include "ae_config.h"
#include "io.h"
#include "log.h"
#include "config.h"
#include "video_common.h"
#include "interrupt.h"

#define GB_SURF (GB_W * GB_H)
#define COLOR_0 0x00000000
#define COLOR_1 0x00444444
#define COLOR_2 0x00aaaaaa
#define COLOR_3 0x00ffffff

static void gpu_init_display(struct gpu *gpu, struct ae_config *conf)
{
	bool fullscreen;
	int width;
	int height;
	int x;
	int y;

#ifdef EMGB_CONSOLE_DEBUGGER
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif /* EMGB_CONSOLE_DEBUGGER */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	if (ae_config_get_int(conf, CONFIG_LINEAR_SCALING,
			CONFIG_LINEAR_SCALING_DEFAULT) == 1)
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	width = ae_config_get_int(conf, CONFIG_WINDOW_WIDTH,
			CONFIG_WINDOW_WIDTH_DEFAULT);
	height = ae_config_get_int(conf, CONFIG_WINDOW_HEIGHT,
			CONFIG_WINDOW_HEIGHT_DEFAULT);
	fullscreen = is_window_fullscreen(width, height);
	x = ae_config_get_int(conf, CONFIG_WINDOW_X, CONFIG_WINDOW_X_DEFAULT);
	y = ae_config_get_int(conf, CONFIG_WINDOW_Y, CONFIG_WINDOW_Y_DEFAULT);
	gpu->window = SDL_CreateWindow("GB", x, y, width, height,
			SDL_WINDOW_RESIZABLE);
	if (gpu->window == NULL)
		ERR("cannot create SDL windows");
	if (fullscreen) {
		SDL_SetWindowFullscreen(gpu->window,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_ShowCursor(SDL_DISABLE);
	}
	gpu->renderer = SDL_CreateRenderer(gpu->window, -1,
			SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
	if (gpu->renderer == NULL)
		ERR("cannot create SDL renderer");
	SDL_RenderSetLogicalSize(gpu->renderer, GB_W, GB_H);
	gpu->texture = SDL_CreateTexture(gpu->renderer,
			SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
			GB_W, GB_H);
	if (gpu->texture == NULL)
		ERR("cannot create SDL texture");
	gpu->pixels = calloc(GB_SURF, sizeof(*gpu->pixels));
	if (gpu->pixels == NULL)
		ERR("cannot alloc pixels");
}

static uint32_t color_index_to_value(const struct gpu *gpu, uint32_t color)
{
	switch (color) {
	case 3:
		return gpu->color_0;
	case 1:
		return gpu->color_1;
	case 2:
		return gpu->color_2;
	case 0:
		return gpu->color_3;
	default:
		return gpu->color_3;
	}
}

/*
void displayAll(struct gb *gb)
{
	int index;
	int y, x;
	unsigned short line;
	int color;
	int dec;
	int posx;
	int posy;

	posx = 0;
	posy = 0;
	for (index = 0x8000; index < 0x9800; index += 0x10) {
		int tmpaddr = index;
		for (y = 0; y < 8; y++) {
			dec = 15;
			line = read16bit(memory, tmpaddr);
			for (x = 0; x < 8; x++) {
				color = (line >> dec) & 0x01;
				if ((line >> (dec - 8)) & 0x01)
					color += 2;
				color = color_index_to_value(color);
				gb->gb_gpu.pixels_d[(256 * (posy + y))
						+ posx + x] = color;
				dec--;
			}
			tmpaddr += 2;
		}
		if (posx + 8 >= 8 * 16) {
			posy += 8;
			posx = 0;
		} else {
			posx += 8;
		}
	}
}
*/

/* never called ? to remove ? TODO */
static __attribute((unused)) void gpu_render_window(struct gpu *gpu,
		struct memory *memory)
{
	int y;
	int x;
	uint16_t line;
	uint32_t color;
	int dec;
	int posx;
	int posy;
	unsigned short limit;
	int baseaddr;
	int i;
	unsigned char tileindex;
	int tmpaddr;
	enum tile_map_display_select tile_map_sel;
	uint16_t tile_map;

	tile_map_sel = memory->spec_reg.lcdc.window_tile_map_display_select;
	tile_map = tile_map_display_select_to_addr(tile_map_sel);
	limit = tile_map + 0x400;
	baseaddr = bg_window_tile_data_select_to_addr(
			memory->spec_reg.lcdc.bg_window_tile_data_select);
	posx = 0;
	posy = 0;
	for (i = tile_map; i < limit; i++) {
		tileindex = read8bit(memory, i);
		tmpaddr = baseaddr + tileindex * 16;
		for (y = 0; y < 8; y++) {
			dec = 15;
			line = read16bit(memory, tmpaddr);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01)
						+ ((line >> (dec - 8)) & 0x01);
				if (color != 0) {
					gpu->pixels[256 * (memory->spec_reg.scy
							+ posy + y)
							+ memory->spec_reg.scx
							+ posx + x] = (color
							* 0xff) | B;
				}
				dec--;
			}
			tmpaddr += 2;
		}
		if ((posx + 8) > 124) {
			posy += 8;
			posx = 0;
		} else {
			posx += 8;
		}
	}
}

static void gpu_render_sprite(struct gpu *gpu, struct memory *memory)
{
	unsigned y;
	int x;
	uint16_t line;
	uint32_t color;
	int dec;
	unsigned char posy;
	unsigned char posx;
	unsigned char tileindex;
	unsigned short limit = 0xFE9F;
	int baseaddr = 0x8000;
	int tmpaddr;
	int index;
	unsigned sprite_size;

	sprite_size = memory->spec_reg.lcdc.sprite_size;
	sprite_size = sprite_size == SPRITE_SIZE_8X8 ? 8 : 16;
	for (index = 0xFE00; index < limit; index += 4) {
		posy = memory->oam[index - 0xFE00] - 16;
		posx = memory->oam[index + 1 - 0xFE00] - 8;
		tileindex = memory->oam[index + 2 - 0xFE00];
		tmpaddr = baseaddr + (tileindex * 16);
		for (y = 0; tileindex && y < sprite_size; y++) {
			dec = 15;
			line = read16bit(memory, tmpaddr);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01);
				if ((line >> (dec - 8)) & 0x01)
					color += 2;
				color = color_index_to_value(gpu, color);
				/*
				 * check mem corruption error -> need to
				 * refactor this
				 */
				if (GB_W * (posy + y) + posx + x < GB_SURF)
					gpu->pixels[GB_W * (posy + y)
							+ (posx + x)] = color;
				dec--;

			}
			tmpaddr += 2;
		}
	}
}

static int gpu_get_real_position(struct gpu *gpu, struct memory *memory)
{
	int y;
	int y_data_line;
	int line_offset;
	int data_offset;
	struct lcdc *lcdc;
	int tile_map;

	lcdc = &memory->spec_reg.lcdc;
	tile_map = tile_map_display_select_to_addr(
			lcdc->bg_tile_map_display_select);
	y = memory->spec_reg.scy + memory->spec_reg.ly;
	y_data_line = y / 8;
	/* 0x20 * 8 == 0x100 == 256 (a line) */
	/* TODO shouldn't this be %= 0x20 ? */
	if (y_data_line > 0x1f)
		y_data_line -= 0x20;
	line_offset = y_data_line * 0x20;
	data_offset = tile_map + line_offset + memory->spec_reg.scx;

	return data_offset;
}

static void gpu_render_bg(struct gpu *gpu, struct memory *memory)
{
	unsigned short line;
	int color;
	int dec;
	int posx;
	int x;
	unsigned char tileindex;
	signed char stileindex;
	int baseaddr;
	int data_offset;
	int index;
	int tile_addr;
	int pixel_index;
	struct spec_reg *spec_reg;

	spec_reg = &memory->spec_reg;
	baseaddr = bg_window_tile_data_select_to_addr(
			spec_reg->lcdc.bg_window_tile_data_select);
	data_offset = gpu_get_real_position(gpu, memory);
	posx = 0;
	for (index = 0; index < 20; index++) {
		if (baseaddr == 0x8800) {
			stileindex = (signed) (read8bit(memory,
					index + data_offset));
			tile_addr = baseaddr + (stileindex + 128) * 16;
		} else {
			tileindex = read8bit(memory, index + data_offset);
			tile_addr = baseaddr + tileindex * 16;
		}
		dec = 15;
		line = read16bit(memory, tile_addr + (spec_reg->ly % 8) * 2);
		for (x = 0; x < 8; x++) {
			color = (line >> dec) & 0x01;
			if ((line >> (dec - 8)) & 0x01)
				color += 2;
			color = color_index_to_value(gpu, color);
			pixel_index = 160 * spec_reg->ly + posx + x;
			if (pixel_index < (160 * 144))
				gpu->pixels[pixel_index] = color;
			dec--;
		}
		posx += 8;
	}
}

static void gpu_rendering(struct gpu *gpu, struct memory *memory)
{
	if (memory->spec_reg.lcdc.enable_bg_window_display)
		gpu_render_bg(gpu, memory);
	if (memory->spec_reg.lcdc.enable_sprite_display)
		gpu_render_sprite(gpu, memory);
}

static void gpu_display(struct gpu *gpu)
{
	int pitch = 0;
	void *pixels;

	pitch = 0;
	SDL_RenderClear(gpu->renderer);
	SDL_LockTexture(gpu->texture, NULL, &pixels, &pitch);
	memcpy(pixels, gpu->pixels, GB_SURF * 4);
	SDL_UnlockTexture(gpu->texture);

	SDL_RenderCopy(gpu->renderer, gpu->texture, NULL, NULL);
	SDL_RenderPresent(gpu->renderer);

	/* step 2 debug */

	/* memset(s_gb->gb_gpu.pixels_d, 0x00ff0000, 256 * 256 * \
	 * sizeof(Uint32));
	 */
	/* SDL_RenderClear(s_gb->gb_gpu.renderer_d); */
	/* SDL_LockTexture(s_gb->gb_gpu.texture_d, NULL, &pixels, &pitch); */
	/* memcpy(s_gb->gb_gpu.pixels_d, pixels, 256 * 256 * 4); */

	/* displayAll(s_gb); */
	/* memcpy(pixels, s_gb->gb_gpu.pixels_d, 256 * 256 * 4); */
	/* SDL_UnlockTexture(s_gb->gb_gpu.texture_d); */

	/* SDL_RenderCopy(s_gb->gb_gpu.renderer_d, s_gb->gb_gpu.texture_d, \
	 * NULL, NULL);
	 */
	/* SDL_RenderPresent(s_gb->gb_gpu.renderer_d); */
}

void gpu_init(struct gpu *gpu, struct cpu *cpu, struct memory *memory,
		struct ae_config *conf)
{
	gpu_init_display(gpu, conf);

	gpu->cpu = cpu;
	gpu->memory = memory;
	gpu->mode = HBLANK;
	gpu->tick = 0;
	gpu->last_tick = 0;
	gpu->color_0 = ae_config_get_int(conf, CONFIG_COLOR_0,
			CONFIG_COLOR_0_DEFAULT);
	gpu->color_1 = ae_config_get_int(conf, CONFIG_COLOR_1,
			CONFIG_COLOR_1_DEFAULT);
	gpu->color_2 = ae_config_get_int(conf, CONFIG_COLOR_2,
			CONFIG_COLOR_2_DEFAULT);
	gpu->color_3 = ae_config_get_int(conf, CONFIG_COLOR_3,
			CONFIG_COLOR_3_DEFAULT);
}

void gpu_update(struct gpu *gpu)
{
	struct memory *memory;
	struct spec_reg *spec_reg;

	memory = gpu->memory;
	spec_reg = &memory->spec_reg;
	gpu->tick += gpu->cpu->total_tick - gpu->last_tick;
	gpu->last_tick = gpu->cpu->total_tick;

	switch (gpu->mode) {
	case HBLANK:
		if (gpu->tick < 204)
			break;

		if (spec_reg->lcdc.enable_lcd && spec_reg->ly < GB_H)
			gpu_rendering(gpu, memory);
		spec_reg->ly++;
		if (spec_reg->ly >= GB_H) {
			spec_reg->ifl |= INT_VBLANK;
			gpu->mode = VBLANK;
		}

		gpu->tick -= 204;
		break;
	case VBLANK:
		if (gpu->tick < 456)
			break;

		spec_reg->ly++;
		if (spec_reg->ly >= 153) {
			spec_reg->ly = 0;
			gpu->mode = OAM;
		}
		gpu->tick -= 456;
		if (spec_reg->lcdc.enable_lcd && spec_reg->ly == GB_H + 1)
			gpu_display(gpu);
		break;
	case OAM:
		if (gpu->tick < 80)
			break;

		spec_reg->ly = 0;
		gpu->mode = VRAM;
		gpu->tick -= 80;
		break;
	case VRAM:
		if (gpu->tick < 172)
			break;

		gpu->mode = HBLANK;
		gpu->tick -= 172;
		break;
	}
}

void gpu_cleanup(struct gpu *gpu)
{
	/* SDL_DestroyWindow(s_gb->gb_gpu.window_d); */
	SDL_DestroyTexture(gpu->texture);
	SDL_DestroyRenderer(gpu->renderer);
	SDL_DestroyWindow(gpu->window);
	SDL_Quit();
	/* free(s_gb->gb_gpu.pixels_d); */
	free(gpu->pixels);
}
