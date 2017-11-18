#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>

#include <histedit.h>

#include "console_debugger.h"
#include "log.h"

static volatile sig_atomic_t signal_received;

typedef void (*debugger_command_fn)(struct console_debugger *debugger);

struct debugger_command {
	debugger_command_fn fn;
	const char *name;
	const char *help;
	int argc;
	int status;
};

static struct breakpoint *get_unused_breakpoint(
		struct console_debugger *debugger)
{
	unsigned i;

	for (i = 0; i < EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS; i++)
		if (debugger->breakpoints[i].status == BREAKPOINT_STATUS_UNUSED)
			return debugger->breakpoints + i;

	return NULL;
}

static void console_debugger_breakpoint(struct console_debugger *debugger)
{
	long adress;
	struct command *command;
	char *endptr;
	const char *adress_str;
	struct breakpoint *breakpoint;

	command = &debugger->command;
	adress_str = command->argv[1];

	adress = strtol(adress_str, &endptr, 0);
	if (*adress_str == '\0' || *endptr != '\0') {
		printf("Invalid pointer adress \"%s\"\n", adress_str);
		return;
	}
	if (adress < 0 || adress > UINT16_MAX) {
		printf("Breakpoint adress must be in range [0, %"PRIu16"]\n",
				UINT16_MAX);
		return;
	}
	breakpoint = get_unused_breakpoint(debugger);
	if (breakpoint == NULL) {
		puts("No more breakpoints available, delete one to proceed.");
		return;
	}

	printf("Breakpoint %d set at adress %#lx\n",
			breakpoint - debugger->breakpoints, adress);

	*breakpoint = (struct breakpoint) {
		.pc = adress,
		.status = BREAKPOINT_STATUS_ENABLED,
	};
}

static void console_debugger_continue(struct console_debugger *debugger)
{
	debugger->active = false;

	puts("continuing execution");
}

static struct debugger_command commands[];
static void console_debugger_help(struct console_debugger *debugger)
{
	struct debugger_command *dc;

	puts("Available commands:");
	for (dc = commands; dc->name != NULL; dc++)
		printf("\t%s: %d argument%s\n\t\t%s\n", dc->name, dc->argc,
				dc->argc > 1 ? "s" : "", dc->help);
}

static struct debugger_command commands[] = {
	{
		.fn = console_debugger_breakpoint,
		.name = "breakpoint",
		.help = "Places a breakpoint at the given pc value.",
		.argc = 2,
	},
	{
		.fn = console_debugger_continue,
		.name = "continue",
		.help = "Continues the execution of the gb rom.",
		.argc = 1,
	},
	{
		.fn = console_debugger_help,
		.name = "help",
		.help = "Shows a little help about available commands.",
		.argc = 1,
	},

	{ .name = NULL } /* NULL guard */
};

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

int console_debugger_init(struct console_debugger *debugger,
		struct s_register *registers)
{
	struct editline *el = debugger->editline;

	memset(debugger, 0, sizeof(*debugger));
	debugger->registers = registers;
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

	/* configure tokenizer */
	debugger->tokenizer = tok_init(NULL);
	if (debugger->tokenizer == NULL)
		ERR("tok_init");

	/*
	 * TODO enable debugger by default, this should be decided with a
	 * command-line switch
	 */
	debugger->active = true;

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

	return 0;
}

static int console_debugger_execute(struct console_debugger *debugger)
{
	struct command *command;
	struct debugger_command *dc;

	command = &debugger->command;
	assert(command->argc >= 0);

	for (dc = commands; dc->name != NULL; dc++) {
		if (strcmp(dc->name, command->argv[0]) != 0)
			continue;

		if (command->argc != dc->argc) {
			printf("got %d arguments, when \"%s\" requires %d\n",
					command->argc, dc->name, dc->argc);
			return 0;
		}
		dc->fn(debugger);

		return 0;
	}

	printf("\"%s\": command not found\n", command->argv[0]);

	return 0;
}

static int console_debugger_parse(struct console_debugger *debugger)
{
	struct command *command;

	command = &debugger->command;

	command->continuation_status = tok_str(debugger->tokenizer,
			command->line, &command->argc,  &command->argv);
	/* line isn't finished, nothing to do */
	if (command->continuation_status != 0)
		return 0;

	tok_reset(debugger->tokenizer);

	return console_debugger_execute(debugger);
}

static bool breakpoint_hit(const struct breakpoint *b, uint16_t pc)
{
	return b->pc == pc && b->status == BREAKPOINT_STATUS_ENABLED;
}

static void console_debugger_check_breakpoints(
		struct console_debugger *debugger)
{
	unsigned i;
	uint16_t pc;

	for (i = 0; i < EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS; i++) {
		pc = debugger->registers->pc;
		if (breakpoint_hit(debugger->breakpoints + i, pc)) {
			debugger->active = true;
			printf("Breakpoint %d hit (pc = %#"PRIx16")\n", i, pc);
		}
	}
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

	console_debugger_check_breakpoints(debugger);

	while (debugger->active) {
		ret = console_debugger_read(debugger);
		if (ret < 0)
			ERR("console_debugger_read: %s", strerror(-ret));
		ret = console_debugger_parse(debugger);
		if (ret < 0)
			ERR("console_debugger_parse: %s", strerror(-ret));
	}

	return 0;
}

