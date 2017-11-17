#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "console_debugger.h"
#include "log.h"

struct command {
	char *line;
};

static volatile sig_atomic_t signal_received = 0;

static void console_debugger_init_signal_handler(int signum)
{
	if (signal_received)
		return;

	printf("%s received, entering debugger\n", strsignal(signum));

	signal_received = 1;
}

int console_debugger_init(struct console_debugger *debugger)
{
	memset(debugger, 0, sizeof(*debugger));
	signal(SIGINT, console_debugger_init_signal_handler);

	return 0;
}

static int console_debugger_read(struct console_debugger *debugger,
		struct command *command)
{
	command->line = readline(EMGB_CONSOLE_DEBUGGER_PROMPT);
	if (command->line == NULL)
		ERR("EOF received, quitting now");

	return 0;
}

static int console_debugger_execute(struct console_debugger *debugger,
		const struct command *command)
{
	printf("Execute command %s\n", command->line);

	return 0; // TODO stub
}

int console_debugger_update(struct console_debugger *debugger)
{
	int ret;
	struct command command;

	if (signal_received)
		debugger->active = true;

	if (debugger->active) {
		ret = console_debugger_read(debugger, &command);
		if (ret < 0)
			ERR("console_debugger_read: %s", strerror(-ret));
		ret = console_debugger_execute(debugger, &command);
		if (ret < 0)
			ERR("console_debugger_execute: %s", strerror(-ret));
	}

	return 0;
}
