#include "io.h"
#include "GB.h"
#include "ae_config.h"

#define GB_W 160
#define GB_H 144
#define GB_SURF (GB_W * GB_H)
#define COLOR_0 0x00101010
#define COLOR_1 0x00585858
#define COLOR_2 0x00a0a0a0
#define COLOR_3 0x00e8e8e8

static bool is_fullscreen(int width, int height)
{
	int ret;
	SDL_DisplayMode dm;
	ret = SDL_GetCurrentDisplayMode(0, &dm);
	if (ret != 0)
		return false;

	return dm.w <= width && dm.h <= height;
}

void initDisplay(struct s_gb *gb)
{
	struct s_gpu *gpu;
	bool fullscreen;
	int width;
	int height;

	gpu = &gb->gb_gpu;
#ifdef EMGB_CONSOLE_DEBUGGER
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif /* EMGB_CONSOLE_DEBUGGER */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	if (ae_config_get_int(&gb->config, "linear_scaling", 1) == 1)
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	width = ae_config_get_int(&gb->config, "window_width", GB_W);
	height = ae_config_get_int(&gb->config, "window_height", GB_H);
	fullscreen = is_fullscreen(width, height);
	gpu->mouse_visible = true;
	gpu->window = SDL_CreateWindow("GB",
			ae_config_get_int(&gb->config, "window_x", 300),
			ae_config_get_int(&gb->config, "window_y", 300),
			width, height, SDL_WINDOW_RESIZABLE);
	if (gpu->window == NULL)
		ERR("cannot create SDL windows");
	if (fullscreen)
		SDL_SetWindowFullscreen(gpu->window,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
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
	gpu->pixels = malloc(sizeof(Uint32) * GB_SURF);
	if (gpu->pixels == NULL)
		ERR("cannot alloc pixels");
}

int color_index_to_value(int color)
{
	switch (color) {
	case 3:
		return COLOR_0;
	case 1:
		return COLOR_1;
	case 2:
		return COLOR_2;
	case 0:
		return COLOR_3;
	default:
		return COLOR_3;
	}
}

/*
void displayAll(struct s_gb *gb)
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

void renderingWindow(struct s_gb *gb)
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
	struct s_io *io;
	struct lcd *lcd;

	io = &gb->gb_io;
	lcd = &io->lcd;
	limit = lcd->WindowTileMapSelect + 0x400;
	baseaddr = lcd->BgWindowTileData;
	posx = 0;
	posy = 0;
	for (i = lcd->WindowTileMapSelect; i < limit; i++) {
		tileindex = read8bit(i, gb);
		tmpaddr = baseaddr + tileindex * 16;
		for (y = 0; y < 8; y++) {
			dec = 15;
			line = read16bit(tmpaddr, gb);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01)
						+ ((line >> (dec - 8)) & 0x01);
				if (color != 0) {
					gb->gb_gpu.pixels[256
							* (io->scrollY + posy
									+ y)
							+ io->scrollX + posx
									+ x] =
							(color * 0xff) | B;
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

void renderingSprite(struct s_gb *gb)
{
	int y;
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

	for (index = 0xFE00; index < limit; index += 4) {
		posy = gb->gb_mem.oam[index - 0xFE00] - 16;
		posx = gb->gb_mem.oam[index + 1 - 0xFE00] - 8;
		tileindex = gb->gb_mem.oam[index + 2 - 0xFE00];
		tmpaddr = baseaddr + (tileindex * 16);
		for (y = 0; tileindex && y < gb->gb_io.lcd.SpriteSize; y++) {
			dec = 15;
			line = read16bit(tmpaddr, gb);
			for (x = 0; x < 8; x++) {
				color = ((line >> dec) & 0x01);
				if ((line >> (dec - 8)) & 0x01)
					color += 2;
				color = color_index_to_value(color);
				/*
				 * check mem corruption error -> need to
				 * refactor this
				 */
				if (GB_W * (posy + y) + posx + x < GB_SURF)
					gb->gb_gpu.pixels[GB_W * (posy + y)
							+ (posx + x)] = color;
				dec--;

			}
			tmpaddr += 2;
		}
	}
}

void rendering(struct s_gb *gb)
{
	if (gb->gb_io.lcd.BgWindowDisplay == 1)
		renderingBg(gb);
	if (gb->gb_io.lcd.SpriteIsOn == 1)
		renderingSprite(gb);
}

static void display(struct s_gb *gb)
{
	int pitch = 0;
	void *pixels;

	pitch = 0;
	SDL_RenderClear(gb->gb_gpu.renderer);
	SDL_LockTexture(gb->gb_gpu.texture, NULL, &pixels, &pitch);
	memcpy(pixels, gb->gb_gpu.pixels, GB_SURF * 4);
	SDL_UnlockTexture(gb->gb_gpu.texture);

	SDL_RenderCopy(gb->gb_gpu.renderer, gb->gb_gpu.texture, NULL, NULL);
	SDL_RenderPresent(gb->gb_gpu.renderer);

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

void initGpu(struct s_gb *gb)
{
	gb->gb_gpu.scanline = 0;
	gb->gb_gpu.tick = 0;
	gb->gb_gpu.last_tick = 0;
}

char lcdIsEnable(unsigned char lcdc)
{
	return (lcdc & 0x01) == 0 ? 0 : 1;
}

void setLcdStatus(struct s_gb *gb)
{
	if (lcdIsEnable(read8bit(0xff40, gb) >> 7) != 0)
		return;
	gb->gb_gpu.scanline = 0;
	printf("reset scanline & s_gb->gb_cpu.totalTick\n");

	write8bit(0xff41, 253, gb);
}

void updateGpu(struct s_gb *gb)
{
	struct s_gpu *gpu;

	gpu = &gb->gb_gpu;
	gpu->tick += gb->gb_cpu.totalTick - gpu->last_tick;
	gpu->last_tick = gb->gb_cpu.totalTick;

	switch (gpu->gpuMode) {
	case HBLANK:
		if (gpu->tick < 204)
			break;

		if (gb->gb_io.lcd.LcdIsOn == 1 && gpu->scanline < GB_H)
			rendering(gb);
		gpu->scanline++;
		if (gpu->scanline >= GB_H) {
			gb->gb_interrupts.interFlag |= INT_VBLANK;
			gpu->gpuMode = VBLANK;
		}

		gpu->tick -= 204;
		break;
	case VBLANK:
		if (gpu->tick < 456)
			break;

		gpu->scanline++;
		if (gpu->scanline >= 153) {
			gpu->scanline = 0;
			gpu->gpuMode = OAM;
		}
		gpu->tick -= 456;
		if (gb->gb_io.lcd.LcdIsOn == 1 && gpu->scanline == GB_H + 1)
			display(gb);
		break;
	case OAM:
		if (gpu->tick < 80)
			break;

		gpu->scanline = 0;
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
