#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <histedit.h>

#include "console_debugger.h"
#include "log.h"

static volatile sig_atomic_t signal_received;

static void console_debugger_init_signal_handler(int signum)
{
	signal_received = signum;
}

static char *console_debugger_prompt(struct editline *el)
{
	struct console_debugger *debugger;

	el_get(el, EL_CLIENTDATA, &debugger);
	if (debugger->command.continuation_status == 0)
		return EMGB_CONSOLE_DEBUGGER_PROMPT;
	else
		return EMGB_CONSOLE_DEBUGGER_PROMPT2;
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
	struct editline *el = debugger->editline;

	memset(debugger, 0, sizeof(*debugger));
	signal(SIGINT, console_debugger_init_signal_handler);
	signal(SIGWINCH, console_debugger_init_signal_handler);
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
	history(debugger->history, &debugger->histevent, H_LOAD,
			debugger->path);
	el_set(el, EL_HIST, history, debugger->history);

	/* line edition setup */
	el_set(el, EL_EDITOR, "emacs");
	el_set(el, EL_PROMPT, console_debugger_prompt);

	/* built-in functions */
	el_set(el, EL_ADDFN, "continue", "Continue program execution",
			console_debugger_continue);

	/* configure tokenizer */
	debugger->tokenizer = tok_init(NULL);
	if (debugger->tokenizer == NULL)
		ERR("tok_init");

	return 0;
}

static int console_debugger_read(struct console_debugger *debugger)
{
	struct command *command;

	command = &debugger->command;
	command->line = el_gets(debugger->editline, &debugger->length);
	if (command->line == NULL) {
		history(debugger->history, &debugger->histevent, H_SAVE,
				debugger->path);
		ERR("EOF received, quitting now");
	}

	history(debugger->history, &debugger->histevent, H_ENTER,
			command->line);
	command->continuation_status = tok_str(debugger->tokenizer,
			command->line, &command->argc,  &command->argv);

	return 0;
}

static int console_debugger_execute(struct console_debugger *debugger)
{
	int i;
	struct command *command;

	command = &debugger->command;
	printf("last line entered: \"%.*s\"\n", strlen(command->line) - 1,
			command->line);
	/* line isn't finished, nothing to do */
	if (command->continuation_status != 0)
		return 0;
	for (i = 0; i < command->argc; i++)
		printf("arg[%d]: %s\n", i, command->argv[i]);

	tok_reset(debugger->tokenizer);

	return 0;
}

int console_debugger_update(struct console_debugger *debugger)
{
	int ret;

	if (signal_received) {
		switch (signal_received) {
			case SIGWINCH:
				el_resize(debugger->editline);
				break;

			case SIGINT:
				if (!debugger->active)
					puts("entering debugger");
				debugger->active = true;
				signal_received = 0;
				break;
		}
	}

	if (!debugger->active)
		return 0;

	ret = console_debugger_read(debugger);
	if (ret < 0)
		ERR("console_debugger_read: %s", strerror(-ret));
	ret = console_debugger_execute(debugger);
	if (ret < 0)
		ERR("console_debugger_execute: %s", strerror(-ret));

	return 0;
}

