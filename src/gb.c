#define _GNU_SOURCE
#include <stdio.h>

#include "gb.h"
#include "io.h"
#include "utils.h"
#include "log.h"
#if EMGB_CONSOLE_DEBUGGER
#include "console_debugger.h"
#endif /* EMGB_CONSOLE_DEBUGGER */
#include "log.h"
#include "save.h"
#include "bzip2.h"

#define to_gb_from(p, ko) container_of((p), struct gb, ko)

static void do_save_restore(struct gb *gb, save_action action,
		const char *opentype)
{
	unsigned i;
#define MAKE_CHUNK(g, c) {&g->c.save_start, &g->c.save_end, STRINGIFY(c)}
	struct save_chunk chunks[] = {
			MAKE_CHUNK(gb, timer),
			MAKE_CHUNK(gb, registers),
			MAKE_CHUNK(gb, gpu),
			MAKE_CHUNK(gb, interrupts),
			MAKE_CHUNK(gb, memory),
			MAKE_CHUNK(gb, joypad),
			MAKE_CHUNK(gb, cpu),
	};
#undef MAKE_CHUNK
	FILE cleanup(cleanup_file)*f = NULL;
	const char *name = action == save_write_chunk ? "Save" : "Restore";

	printf("%s %s.\n", name, gb->save_file);
	f = bzip2_fopen(gb->save_file, opentype);
	if (f == NULL) {
		DBG("%s failed: fopen: %m", name);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(chunks); i++)
		action(chunks + i, f);
};

static void gb_save(const struct key_action *key_action)
{
	do_save_restore(to_gb_from(key_action, save), save_write_chunk, "wbe");
};

static void gb_restore(const struct key_action *key_action)
{
	struct gb *gb;

	gb = to_gb_from(key_action, restore);
	do_save_restore(gb, save_read_chunk, "rbe");

	/*
	 * if save was performed when a button was pressed, the button won't be
	 * released when the restore occurs, hence the need for this reset
	 */
	joypad_reset(&gb->joypad);
};

static void register_keys(struct gb *gb)
{
	gb->save = (struct key_action) {
		.command = "save",
		.help = "Create a save of the current game state, to be "
				"restored later.",
		.sym = SDLK_F1,
		.action = gb_save,
	};
	gb->restore = (struct key_action) {
		.command = "restore",
		.help = "Restore a game state, previously saved.",
		.sym = SDLK_F2,
		.action = gb_restore,
	};
	joypad_register_key_action(&gb->joypad, &gb->save);
	joypad_register_key_action(&gb->joypad, &gb->restore);
}

static int init_file_paths(struct gb *gb, const char *file)
{
	int ret;

	if (str_matches_suffix(file, ".sav")) {
		gb->rom_file = strdup(file);
		if (gb->rom_file == NULL)
			ERR("strdup: %m");
		gb->rom_file[strlen(file) - 4] = '\0';
	} else if (str_matches_suffix(file, ".gb")) {
		gb->rom_file = strdup(file);
		if (gb->rom_file == NULL)
			ERR("strdup: %m");
	} else if (str_matches(file, "-")) {
		gb->rom_file = strdup("stdin");
		if (gb->rom_file == NULL)
			ERR("strdup: %m");
	} else {
		ERR("Unsupported extension, for file %s.", file);
	}
	ret = asprintf(&gb->save_file, "%s.sav", gb->rom_file);
	if (ret == -1)
		ERR("asprintf");

	return 0;
}

struct gb *gb_init(const char *file)
{
	struct gb *gb;
	bool restore;

	restore = str_matches_suffix(file, ".sav");
	gb = calloc(1, sizeof(*gb));
	if (gb == NULL)
		ERR("Cannot allocate gb");
	init_file_paths(gb, file);

	config_init(&gb->config);
	memory_init(&gb->memory, &gb->timer);
	if (!restore)
		rom_init(&gb->memory.rom_bank_0_rom,
				&gb->memory.switchable_rom_bank_rom,
				gb->memory.extra_rom_banks, gb->rom_file);
	rom_display_header(&gb->memory.rom_bank_0_rom.rom_header);
	registers_init(&gb->registers);
	cpu_init(&gb->cpu);
	interrupt_init(&gb->interrupts, &gb->memory, &gb->cpu,
			&gb->memory.spec_reg, &gb->registers);
	gpu_init(&gb->gpu, &gb->cpu, &gb->memory, &gb->config.config);
	timer_init(&gb->timer, &gb->memory.spec_reg);

	reset_joystick_config(&gb->joystick_config);
	joypad_init(&gb->joypad, &gb->config, &gb->memory.spec_reg,
			&gb->joystick_config);

	config_write(&gb->config);

	if (restore)
		do_save_restore(gb, save_read_chunk, "rbe");

	register_keys(gb);

	return gb;
}

void gb_cleanup(struct gb *gb)
{
	cleanup_joystick_config(&gb->joystick_config);
	gpu_cleanup(&gb->gpu);
	config_cleanup(&gb->config);
	cleanup_string(&gb->save_file);
	cleanup_string(&gb->rom_file);
	memset(gb, 0, sizeof(*gb));
	free(gb);
}

