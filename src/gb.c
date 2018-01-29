#define _GNU_SOURCE
#include <stdio.h>

#include "gb.h"
#include "io.h"
#include "utils.h"
#include "log.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#include "log.h"
#endif /* EMGB_CONSOLE_DEBUGGER */

#define to_gb_from(p, ko) container_of((p), struct gb, ko)

static void gb_save(const struct key_op *key_op)
{
	int ret;
	struct gb *gb;
	FILE cleanup(cleanup_file)*f = NULL;
	char cleanup(cleanup_string)*path = NULL;

	gb = to_gb_from(key_op, save);
	ret = asprintf(&path, "%s.sav", gb->file);
	if (ret == -1) {
		path = NULL;
		ERR("asprintf");
	}
	f = fopen(path, "wbe");
	if (f == NULL) {
		DBG("Save failed: fopen: %m");
		return;
	}

	ret = timer_save(&gb->timer, f);
	if (ret == -1) {
		DBG("Save failed writing timer");
		return;
	}
	ret = registers_save(&gb->registers, f);
	if (ret == -1) {
		DBG("Save failed writing registers");
		return;
	}
	ret = gpu_save(&gb->gpu, f);
	if (ret == -1) {
		DBG("Save failed writing gpu");
		return;
	}
	ret = interrupt_save(&gb->interrupts, f);
	if (ret == -1) {
		DBG("Save failed writing interrupt");
		return;
	}
	ret = memory_save(&gb->memory, f);
	if (ret == -1) {
		DBG("Save failed writing memory");
		return;
	}
	ret = joypad_save(&gb->joypad, f);
	if (ret == -1) {
		DBG("Save failed writing joypad");
		return;
	}

	printf("State saved to %s\n", path);
};

static void gb_restore(const struct key_op *key_op)
{
	printf("restore requested\n");
};

static void register_keys(struct gb *gb)
{
	gb->save = (struct key_op) {
		.sym = SDLK_F1,
		.action = gb_save,
	};
	gb->restore = (struct key_op) {
		.sym = SDLK_F2,
		.action = gb_restore,
	};
	joypad_register_key_op(&gb->joypad, &gb->save);
	joypad_register_key_op(&gb->joypad, &gb->restore);
}

struct gb *gb_init(const char *file)
{
	struct gb *gb = NULL;
	long rom_size;

	gb = calloc(1, sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	gb->file = strdup(file);
	if (gb->file == NULL)
		ERR("strdup: %m");
	rom_size = get_file_size_from_path(file);
	if (rom_size < 0)
		ERR("get_file_size_from_path: %s", strerror(-rom_size));

	config_init(&gb->config);
	memory_init(&gb->memory, &gb->timer, rom_size);
	rom_init(&gb->memory.rom_bank_0_rom,
			&gb->memory.switchable_rom_bank_rom,
			gb->memory.extra_rom_banks, file);
	rom_display_header(&gb->memory.rom_bank_0_rom.rom_header);
	registers_init(&gb->registers);
	cpu_init(&gb->cpu);
	interrupt_init(&gb->interrupts, &gb->memory, &gb->cpu,
			&gb->memory.spec_reg, &gb->registers);
	gpu_init(&gb->gpu, &gb->cpu, &gb->memory, &gb->config.config);
	timer_init(&gb->timer, &gb->memory, &gb->cpu);

	reset_joystick_config(&gb->joystick_config);
	joypad_init(&gb->joypad, &gb->config, &gb->memory.spec_reg,
			&gb->joystick_config);

	config_write(&gb->config);

	register_keys(gb);

	return gb;
}

void gb_cleanup(struct gb *gb)
{
	cleanup_joystick_config(&gb->joystick_config);
	gpu_cleanup(&gb->gpu);
	config_cleanup(&gb->config);
	cleanup_string(&gb->file);
	memset(gb, 0, sizeof(*gb));
	free(gb);
}

