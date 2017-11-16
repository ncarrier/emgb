#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include "console_debugger.h"
#include "log.h"

struct command {
	char *line;
};

static sig_atomic_t signal_received;

static void console_debugger_init_signal_handler(int signum)
{
	printf("%s received, stopping\n", strsignal(signum));

	signal_received = true;
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
	return 0; // TODO stub
}

static int console_debugger_execute(struct console_debugger *debugger,
		const struct command *command)
{
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
