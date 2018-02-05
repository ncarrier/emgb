#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H
#include <stdbool.h>

#include <histedit.h>

#include "utils.h"
#include "config.h"
#include "registers.h"
#include "key_action.h"

#define EMGB_CONSOLE_DEBUGGER_PROMPT "egd > "
#define EMGB_CONSOLE_DEBUGGER_PROMPT2 "... > "
#define EMGB_CONSOLE_DEBUGGER_HISTORY_FILE CONFIG_DIR "history"
#define EMGB_CONSOLE_DEBUGGER_PATH_MAX 500
#define EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS 20

#define NB_REGISTERS 14

struct command {
	const char *line;
	int argc;
	const char **argv;
	int continuation_status;
};

enum breakpoint_status {
	BREAKPOINT_STATUS_UNUSED,
	BREAKPOINT_STATUS_ENABLED,
	BREAKPOINT_STATUS_DISABLED,
};

#define MAX_LABEL_NAME_LEN 200
#define MAX_LABELS 1000
struct label {
	uint8_t bank;
	uint16_t address;
	char name[MAX_LABEL_NAME_LEN];
};

struct breakpoint {
	uint16_t pc;
	enum breakpoint_status status;
};

struct cpu;
struct memory;
struct console_debugger {
	char path[EMGB_CONSOLE_DEBUGGER_PATH_MAX];
	bool active;
	bool next;
	bool hud;
	struct editline *editline;
	struct history *history;
	struct HistEvent histevent;
	struct tokenizer *tokenizer;
	int length;
	struct command command;
	struct breakpoint breakpoints[EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS];
	struct memory *memory;
	struct registers *registers;
	struct registers previous_registers;
	struct {
		const char *name;
		union {
			uint8_t *v8;
			uint16_t *v16;
		} value;
		size_t size;
	} registers_map[NB_REGISTERS];
	struct {
		unsigned short columns;
		unsigned short rows;
	} terminal;
	struct label labels[MAX_LABELS];
	unsigned nb_labels;
	const struct key_action *key_action[KEY_ACTIONS_MAX];
};

int console_debugger_init(struct console_debugger *debugger,
		struct registers *registers, struct memory *memory,
		struct ae_config *config, const char *file_name);
void console_debugger_print_registers(const struct registers *registers);
int console_debugger_update(struct console_debugger *debugger);
int console_debugger_register_key_action(struct console_debugger *debugger,
		const struct key_action *key_action);
void console_debugger_cleanup(struct console_debugger *debugger);

#endif /* CONSOLE_DEBUGGER_H */
