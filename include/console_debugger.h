#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H
#include <stdbool.h>

#include "histedit.h"

#define EMGB_CONSOLE_DEBUGGER_PROMPT "egd > "
#define EMGB_CONSOLE_DEBUGGER_PROMPT2 "... > "
#define EMGB_CONSOLE_DEBUGGER_HISTORY_FILE ".emgb_history"
#define EMGB_CONSOLE_DEBUGGER_PATH_MAX 500

struct command {
	const char *line;
	int argc;
	const char **argv;
	int continuation_status;
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
};

int console_debugger_init(struct console_debugger *debugger);
int console_debugger_update(struct console_debugger *debugger);

#endif /* CONSOLE_DEBUGGER_H */
