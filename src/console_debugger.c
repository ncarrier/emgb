#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <histedit.h>

#include "console_debugger.h"
#include "log.h"

struct command {
	const char *line;
};

static volatile sig_atomic_t signal_received = false;

static void console_debugger_init_signal_handler(int signum)
{
	if (signal_received)
		return;

	printf("%s received, entering debugger\n", strsignal(signum));

	signal_received = true;
}

static char *console_debugger_prompt(struct editline *el)
{
	return EMGB_CONSOLE_DEBUGGER_PROMPT;
}

static unsigned char console_debugger_continue(struct editline *el, int ch)
{
	struct console_debugger *debugger;

	el_get(el, EL_CLIENTDATA, &debugger);
	debugger->active = false;

	return CC_REFRESH;
}

int console_debugger_init(struct console_debugger *debugger)
{
	int ret;
	struct editline *el = debugger->editline;

	memset(debugger, 0, sizeof(*debugger));
	signal(SIGINT, console_debugger_init_signal_handler);
	// TODO how to perform a cleanup ?
	debugger->editline = el = el_init("emgb", stdin, stdout, stderr);
	if (el == NULL)
		ERR("el_init");
	el_set(el, EL_CLIENTDATA, debugger);

	/* configure history */
	debugger->history = history_init();
	if (debugger-> history == NULL)
		ERR("history_init");
	snprintf(debugger->path, EMGB_CONSOLE_DEBUGGER_PATH_MAX,
			"%s/"EMGB_CONSOLE_DEBUGGER_HISTORY_FILE,
			getenv("HOME"));
	printf("loading history from %s\n", debugger->path);
	history(debugger->history, &debugger->histevent, H_SETSIZE, 100);
	ret = history(debugger->history, &debugger->histevent, H_LOAD,
			debugger->path);
	el_set(el, EL_HIST, history, debugger->history);

	/* line edition setup */
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, console_debugger_prompt);

	/* built-in functions */
	el_set(el, EL_ADDFN, "continue", "Continue program execution",
			console_debugger_continue);

	return 0;
}

static int console_debugger_read(struct console_debugger *debugger,
		struct command *command)
{
	command->line = el_gets(debugger->editline, &debugger->length);
	if (command->line == NULL) {
		history(debugger->history, &debugger->histevent, H_SAVE,
				debugger->path);
		ERR("EOF received, quitting now");
	}

	history(debugger->history, &debugger->histevent, H_ENTER,
			command->line);

	return 0;
}

static int console_debugger_execute(struct console_debugger *debugger,
		const struct command *command)
{
	printf("Execute command \"%.*s\"\n", strlen(command->line) - 1,
			command->line);

	return 0; // TODO stub
}

int console_debugger_update(struct console_debugger *debugger)
{
	int ret;
	struct command command;

	if (signal_received) {
		debugger->active = true;
		signal_received = false;
	}

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
