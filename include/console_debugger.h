#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H
#include <stdbool.h>

#include <histedit.h>

#include "cpu.h"

#define BIT0(v) ((v) & 1)
#define BIT(i, v) BIT0((v) >> (i))

#define EMGB_CONSOLE_DEBUGGER_PROMPT "egd > "
#define EMGB_CONSOLE_DEBUGGER_PROMPT2 "... > "
#define EMGB_CONSOLE_DEBUGGER_HISTORY_FILE ".emgb_history"
#define EMGB_CONSOLE_DEBUGGER_PATH_MAX 500
#define EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS 20

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
	struct editline *editline;
	struct history *history;
	struct HistEvent histevent;
	struct tokenizer *tokenizer;
	int length;
	struct command command;
	struct breakpoint breakpoints[EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS];

	struct s_register *registers;
};

int console_debugger_init(struct console_debugger *debugger,
		struct s_register *registers);
int console_debugger_update(struct console_debugger *debugger);

#endif /* CONSOLE_DEBUGGER_H */
