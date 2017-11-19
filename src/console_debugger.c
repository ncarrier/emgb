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

static void console_debugger_disable_enable(struct console_debugger *debugger,
		bool enable)
{
	long id;
	struct command *command;
	char *endptr;
	const char *id_str;
	struct breakpoint *breakpoint;

	command = &debugger->command;

	id_str = command->argv[1];
	id = strtol(id_str, &endptr, 0);
	if (*id_str == '\0' || *endptr != '\0') {
		printf("Invalid breakpoint id \"%s\"\n", id_str);
		return;
	}
	if (id < 0 || id >= EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS) {
		printf("Breakpoint id must be in range [0, %"PRIu16"]\n",
				EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS);
		return;
	}
	breakpoint = debugger->breakpoints + id;
	breakpoint->status = enable ? BREAKPOINT_STATUS_ENABLED :
		BREAKPOINT_STATUS_DISABLED;

	printf("%sabled breakpoint %ld at address %#"PRIx16")\n",
			enable ? "En" : "Dis", id, breakpoint->pc);
}

static void console_debugger_enable(struct console_debugger *debugger)
{
	console_debugger_disable_enable(debugger, true);
}

static void console_debugger_delete(struct console_debugger *debugger)
{
	long id;
	struct command *command;
	char *endptr;
	const char *id_str;
	const char *item;
	struct breakpoint *breakpoint;

	command = &debugger->command;
	item = command->argv[1];
	id_str = command->argv[2];
	if (!str_matches(item, "breakpoint")) {
		puts("only delete breakpoint is supported");
		return;
	}

	id = strtol(id_str, &endptr, 0);
	if (*id_str == '\0' || *endptr != '\0') {
		printf("Invalid breakpoint id \"%s\"\n", id_str);
		return;
	}
	if (id < 0 || id >= EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS) {
		printf("Breakpoint id must be in range [0, %"PRIu16"]\n",
				EMGB_CONSOLE_DEBUGGER_MAX_BREAKPOINTS);
		return;
	}
	breakpoint = debugger->breakpoints + id;
	breakpoint->status = BREAKPOINT_STATUS_UNUSED;

	printf("Deleted breakpoint %ld (was %#"PRIx16")\n", id,
			breakpoint->pc);
}

static void console_debugger_disable(struct console_debugger *debugger)
{
	console_debugger_disable_enable(debugger, false);
}

static struct debugger_command commands[];
static void console_debugger_help(struct console_debugger *debugger)
{
	struct debugger_command *dc;

	puts("Available commands:");
	for (dc = commands; dc->name != NULL; dc++)
		printf("\t%s: %d argument%s\n\t\t%s\n", dc->name, dc->argc,
				dc->argc > 1 ? "s" : "", dc->help);
	puts("\nCommand name can be entered partially, if non ambiguous.");
}

static void console_debugger_next(struct console_debugger *debugger)
{
	debugger->next = true;
}

static void console_debugger_print(struct console_debugger *debugger)
{
	const char *expression;
	struct s_register *registers;
	uint16_t f;

	registers = debugger->registers;
	expression = debugger->command.argv[1];
	if (str_matches(expression, "af")) {
		printf("af = %#.04"PRIx16"\n", registers->af);
	} else if (str_matches(expression, "a")) {
		printf("a = %#.02"PRIx8"\n", registers->a);
	} else if (str_matches(expression, "f")) {
		f = registers->f;
		printf("f = %#.02"PRIx8" (z = %d, n = %d, h = %d, c = %d)\n",
				f, BIT(7, f), BIT(6, f), BIT(5, f), BIT(4, f));
	} else if (str_matches(expression, "bc")) {
		printf("bc = %#.04"PRIx16"\n", registers->bc);
	} else if (str_matches(expression, "b")) {
		printf("b = %#.02"PRIx8"\n", registers->b);
	} else if (str_matches(expression, "c")) {
		printf("c = %#.02"PRIx8"\n", registers->c);
	} else if (str_matches(expression, "de")) {
		printf("de = %#.04"PRIx16"\n", registers->de);
	} else if (str_matches(expression, "d")) {
		printf("d = %#.02"PRIx8"\n", registers->d);
	} else if (str_matches(expression, "e")) {
		printf("e = %#.02"PRIx8"\n", registers->e);
	} else if (str_matches(expression, "hl")) {
		printf("hl = %#.04"PRIx16"\n", registers->hl);
	} else if (str_matches(expression, "h")) {
		printf("h = %#.02"PRIx8"\n", registers->h);
	} else if (str_matches(expression, "l")) {
		printf("l = %#.02"PRIx8"\n", registers->l);
	} else if (str_matches(expression, "pc")) {
		printf("pc = %#.04"PRIx16"\n", registers->pc);
	} else if (str_matches(expression, "sp")) {
		printf("sp = %#.04"PRIx16"\n", registers->sp);
	} else if (str_matches(expression, "registers")) {
		printf("af = 0x%.02"PRIx16" %.02"PRIx16"\t", registers->a,
				registers->f);
		printf("bc = 0x%.02"PRIx16" %.02"PRIx16"\t", registers->b,
				registers->c);
		printf("pc = 0x%.04"PRIx16"\t", registers->pc);
		f = registers->f;
		printf("z (zero)       = %d  ", BIT(7, f));
		printf("n (substract) = %d\n", BIT(6, f));
		printf("hl = 0x%.02"PRIx16" %.02"PRIx16"\t", registers->h,
				registers->l);
		printf("de = 0x%.02"PRIx16" %.02"PRIx16"\t", registers->d,
				registers->e);
		printf("sp = 0x%.04"PRIx16"\t", registers->sp);
		printf("h (half carry) = %d  ", BIT(5, f));
		printf("c (carry)     = %d\n", BIT(4, f));
	} else {
		printf("Unable to print \"%s\".\n", expression);
	}
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
		.fn = console_debugger_enable,
		.name = "enable",
		.help = "Enables a breakpoint.\n"
			"\t\t\tusage: enable breakpoint_id.",
		.argc = 2,
	},
	{
		.fn = console_debugger_delete,
		.name = "delete",
		.help = "Deletes an item, item type must be: breakpoint.\n"
			"\t\t\tusage: delete item item_id.",
		.argc = 3,
	},
	{
		.fn = console_debugger_disable,
		.name = "disable",
		.help = "Disables a breakpoint.\n"
			"\t\t\tusage: disable breakpoint_id.",
		.argc = 2,
	},
	{
		.fn = console_debugger_help,
		.name = "help",
		.help = "Shows a little help about available commands.",
		.argc = 1,
	},
	{
		.fn = console_debugger_next,
		.name = "next",
		.help = "Execute until next instruction.",
		.argc = 1,
	},
	{
		.fn = console_debugger_print,
		.name = "print",
		.help = "Prints internal values, memory, registers...\n"
			"\t\tValues printable so far: registers, a, f, af, sp"
			"...\n"
			"\t\t\texample: print registers.",
		.argc = 2,
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
	history(debugger->history, &debugger->histevent, H_SETUNIQUE, true);
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

	if (command->line[0] != '\n')
		history(debugger->history, &debugger->histevent, H_ENTER,
				command->line);

	return 0;
}

static bool is_ambiguous(const char *name, const struct debugger_command *dc,
		const struct console_debugger *debugger)
{
	const struct debugger_command *cur;
	unsigned count;
	const char *diff_char;

	for (cur = dc + 1, count = 0; cur->name != NULL; cur++) {
		diff_char = str_diff_chr(name, cur->name);
		if (*diff_char == '\0') {
			count++;
			if (count == 1)
				printf("Command \"%s\" is ambiguous, "
						"candidates: %s", name,
						dc->name);
			printf(", %s", cur->name);
		}
	}

	if (count > 0) {
		puts("");
		return true;
	}

	return false;
}

static int console_debugger_execute(struct console_debugger *debugger)
{
	struct command *command;
	struct debugger_command *dc;
	char *diff_char;
	const char *name;

	command = &debugger->command;
	assert(command->argc >= 0);

	for (dc = commands; dc->name != NULL; dc++) {
		name = command->argv[0];
		diff_char = str_diff_chr(name, dc->name);
		if (*diff_char != '\0')
			continue;
		if (is_ambiguous(name, dc, debugger))
			return 0;

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
	if (*command->line == '\n') {
		/* reexecute last command */
		if (command->argc != 0)
			return console_debugger_execute(debugger);
		else
			return 0; /* no previous command */
	}

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
			printf("Breakpoint %d hit (pc = %#.04"PRIx16")\n", i, pc);
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
					puts("\rentering debugger, type help");
				debugger->active = true;
				signal_received = 0;
				break;
		}
	}

	console_debugger_check_breakpoints(debugger);

	if (debugger->next) {
		debugger->next = false;
		printf("pc = %#.04"PRIx16"\n", debugger->registers->pc);
	}
	while (debugger->active && !debugger->next) {
		ret = console_debugger_read(debugger);
		if (ret < 0)
			ERR("console_debugger_read: %s", strerror(-ret));
		ret = console_debugger_parse(debugger);
		if (ret < 0)
			ERR("console_debugger_parse: %s", strerror(-ret));
	}

	return 0;
}

bool str_matches(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0;
}

bool str_matches_prefix(const char *s, const char *prefix)
{
	return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* returns an adress inside string s1 */
char *str_diff_chr(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	return (char *)s1;
}

