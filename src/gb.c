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

#define to_gb_from(p, ko) container_of((p), struct gb, ko)

static void do_save_restore(struct gb *gb,
		int (*action)(FILE *f, struct save_start *start,
		const struct save_end *end), const char *opentype)
{
	unsigned i;
	int ret;
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
	FILE cleanup(cleanup_file)*f = NULL;
	char cleanup(cleanup_string)*path = NULL;

	ret = asprintf(&path, "%s.sav", gb->file);
	if (ret == -1) {
		path = NULL;
		ERR("asprintf");
	}
	printf("Save path is %s\n", path);
	f = fopen(path, opentype);
	if (f == NULL) {
		DBG("Save/restore failed: fopen: %m");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(chunks); i++) {
		ret = action(f, chunks[i].start, chunks[i].end);
		if (ret < 0) {
			DBG("Save/restore failed on %s: %s", chunks[i].name,
					strerror(-ret));
			return;
		}
	}
};

static void gb_save(const struct key_op *key_op)
{
	struct gb *gb = to_gb_from(key_op, save);

	do_save_restore(gb, save_write_chunk, "wbe");

	printf("State saved\n");
};

static void gb_restore(const struct key_op *key_op)
{
	struct gb *gb = to_gb_from(key_op, restore);

	do_save_restore(gb, save_read_chunk, "rbe");

	printf("State restored\n");
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
	timer_init(&gb->timer, &gb->memory.spec_reg);

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

