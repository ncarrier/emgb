#include "gb.h"
#include "io.h"
#include "ae_config.h"

#define GB_SURF (GB_W * GB_H)
#define COLOR_0 0x00000000
#define COLOR_1 0x00444444
#define COLOR_2 0x00aaaaaa
#define COLOR_3 0x00ffffff

static bool is_fullscreen(int width, int height)
{
	int ret;
	SDL_DisplayMode dm;
	ret = SDL_GetCurrentDisplayMode(0, &dm);
	if (ret != 0)
		return false;

	return dm.w <= width && dm.h <= height;
}

static void display_init(struct gpu *gpu, struct ae_config *conf)
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
	fullscreen = is_fullscreen(width, height);
	gpu->mouse_visible = !fullscreen;
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

int color_index_to_value(const struct gpu *gpu, int color)
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
			line = read16bit(tmpaddr, gb);
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
void renderingWindow(struct gb *gb)
{
	int y;
	int x;
	unsigned short line;
	int color;
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
	struct memory *memory;

	memory = &gb->memory;
	tile_map_sel = memory->lcdc.window_tile_map_display_select;
	tile_map = tile_map_display_select_to_addr(tile_map_sel);
	limit = tile_map + 0x400;
	baseaddr = bg_window_tile_data_select_to_addr(
			memory->lcdc.bg_window_tile_data_select);
	posx = 0;
	posy = 0;
	for (i = tile_map; i < limit; i++) {
		tileindex = read8bit(i, gb);
		tmpaddr = baseaddr + tileindex * 16;
		for (y = 0; y < 8; y++) {
			dec = 15;
			line = read16bit(tmpaddr, gb);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01)
						+ ((line >> (dec - 8)) & 0x01);
				if (color != 0) {
					gb->gpu.pixels[256
							* (memory->register_scy
									+ posy
									+ y)
							+ memory->register_scx
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

void renderingSprite(struct gb *gb)
{
	unsigned y;
	int x;
	unsigned short line;
	int color;
	int dec;
	unsigned char posy;
	unsigned char posx;
	unsigned char tileindex;
	unsigned short limit = 0xFE9F;
	int baseaddr = 0x8000;
	int tmpaddr;
	int index;
	unsigned sprite_size;

	sprite_size = gb->memory.lcdc.sprite_size == SPRITE_SIZE_8X8 ? 8 : 16;
	for (index = 0xFE00; index < limit; index += 4) {
		posy = gb->memory.oam[index - 0xFE00] - 16;
		posx = gb->memory.oam[index + 1 - 0xFE00] - 8;
		tileindex = gb->memory.oam[index + 2 - 0xFE00];
		tmpaddr = baseaddr + (tileindex * 16);
		for (y = 0; tileindex && y < sprite_size; y++) {
			dec = 15;
			line = read16bit(tmpaddr, gb);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01);
				if ((line >> (dec - 8)) & 0x01)
					color += 2;
				color = color_index_to_value(&gb->gpu,
						color);
				/*
				 * check mem corruption error -> need to
				 * refactor this
				 */
				if (GB_W * (posy + y) + posx + x < GB_SURF)
					gb->gpu.pixels[GB_W * (posy + y)
							+ (posx + x)] = color;
				dec--;

			}
			tmpaddr += 2;
		}
	}
}

static int getRealPosition(struct gb *gb)
{
	int yPos;
	int yDataLine;
	int lineOffset;
	int dataOffset;
	struct lcdc *lcdc;
	int tile_map;
	struct memory *memory;

	memory = &gb->memory;
	lcdc = &memory->lcdc;
	tile_map = tile_map_display_select_to_addr(
			lcdc->bg_tile_map_display_select);
	yPos = memory->register_scy + *gb->gpu.scanline;
	yDataLine = yPos / 8;
	/* TODO shouldn't this be %= 0x20 ? */
	if (yDataLine > 0x1f)
		yDataLine -= 0x20;
	lineOffset = yDataLine * 0x20; /* 0x20 * 8 == 0x100 == 256 (a line) */
	dataOffset = tile_map + lineOffset + memory->register_scx;

	return dataOffset;
}

static void renderingBg(struct gb *gb)
{
	unsigned short line;
	int color;
	int dec;
	int posx;
	int x;
	unsigned char tileindex;
	signed char stileindex;
	int baseaddr;
	int dataOffset;
	int index;
	int tileAddr;
	struct gpu *gpu;
	int pixel_index;

	gpu = &gb->gpu;
	baseaddr = bg_window_tile_data_select_to_addr(
			gb->memory.lcdc.bg_window_tile_data_select);
	dataOffset = getRealPosition(gb);
	posx = 0;
	for (index = 0; index < 20; index++) {
		if (baseaddr == 0x8800) {
			stileindex = (signed)(read8bit(index + dataOffset, gb));
			tileAddr = baseaddr + (stileindex + 128) * 16;
		} else {
			tileindex = read8bit(index + dataOffset, gb);
			tileAddr = baseaddr + tileindex * 16;
		}
		dec = 15;
		line = read16bit(tileAddr + (*gpu->scanline % 8) * 2, gb);
		for (x = 0; x < 8; x++) {
			color = (line >> dec) & 0x01;
			if ((line >> (dec - 8)) & 0x01)
				color += 2;
			color = color_index_to_value(gpu, color);
			pixel_index = 160 * *gpu->scanline + posx + x;
			if (pixel_index < (160 * 144))
				gpu->pixels[pixel_index] = color;
			dec--;
		}
		posx += 8;
	}
}

static void rendering(struct gb *gb)
{
	struct memory *memory;

	memory = &gb->memory;
	if (memory->lcdc.enable_bg_window_display)
		renderingBg(gb);
	if (memory->lcdc.enable_sprite_display)
		renderingSprite(gb);
}

static void display(struct gb *gb)
{
	int pitch = 0;
	void *pixels;

	pitch = 0;
	SDL_RenderClear(gb->gpu.renderer);
	SDL_LockTexture(gb->gpu.texture, NULL, &pixels, &pitch);
	memcpy(pixels, gb->gpu.pixels, GB_SURF * 4);
	SDL_UnlockTexture(gb->gpu.texture);

	SDL_RenderCopy(gb->gpu.renderer, gb->gpu.texture, NULL, NULL);
	SDL_RenderPresent(gb->gpu.renderer);

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

void gpu_init(struct gpu *gpu, struct ae_config *conf, uint8_t *register_ly)
{
	display_init(gpu, conf);

	gpu->gpuMode = HBLANK;
	gpu->scanline = register_ly;
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

void gpu_update(struct gb *gb)
{
	struct gpu *gpu;
	struct memory *memory;

	memory = &gb->memory;
	gpu = &gb->gpu;
	gpu->tick += gb->cpu.totalTick - gpu->last_tick;
	gpu->last_tick = gb->cpu.totalTick;

	switch (gpu->gpuMode) {
	case HBLANK:
		if (gpu->tick < 204)
			break;

		if (memory->lcdc.enable_lcd && *gpu->scanline < GB_H)
			rendering(gb);
		(*gpu->scanline)++;
		if (*gpu->scanline >= GB_H) {
			memory->register_if |= INT_VBLANK;
			gpu->gpuMode = VBLANK;
		}

		gpu->tick -= 204;
		break;
	case VBLANK:
		if (gpu->tick < 456)
			break;

		(*gpu->scanline)++;
		if (*gpu->scanline >= 153) {
			*gpu->scanline = 0;
			gpu->gpuMode = OAM;
		}
		gpu->tick -= 456;
		if (memory->lcdc.enable_lcd && *gpu->scanline == GB_H + 1)
			display(gb);
		break;
	case OAM:
		if (gpu->tick < 80)
			break;

		*gpu->scanline = 0;
		gpu->gpuMode = VRAM;
		gpu->tick -= 80;
		break;
	case VRAM:
		if (gpu->tick < 172)
			break;

		gpu->gpuMode = HBLANK;
		gpu->tick -= 172;
		break;
	}
}
