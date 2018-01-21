#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H
#include <stdbool.h>

#include <histedit.h>

#include "cpu.h"
#include "utils.h"
#include "ae_config.h"

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

struct breakpoint {
	uint16_t pc;
	enum breakpoint_status status;
};

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
	struct s_gb *gb;
	struct s_register *registers;
	struct s_register previous_registers;
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
};

void console_debugger_print_registers(const struct s_register *registers);
int console_debugger_init(struct console_debugger *debugger,
		struct s_register *registers, struct s_gb *gb,
		struct ae_config *config);
int console_debugger_update(struct console_debugger *debugger);

bool str_matches(const char *s1, const char *s2);
bool str_matches_prefix(const char *s, const char *prefix);
char *str_diff_chr(const char *s1, const char *s2);

#endif /* CONSOLE_DEBUGGER_H */
