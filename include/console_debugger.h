#ifndef CONSOLE_DEBUGGER_H
#define CONSOLE_DEBUGGER_H
#include <stdbool.h>

struct console_debugger {
	bool active;
};

int console_debugger_init(struct console_debugger *debugger);
int console_debugger_update(struct console_debugger *debugger);

#endif /* CONSOLE_DEBUGGER_H */
