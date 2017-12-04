#if EMGB_CONSOLE_DEBUGGER
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
#include "memory.h"

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

/* returns -1 if "name" doesn't name a register, the value fetched on success */
static int get_register_value(const struct console_debugger *debugger,
		const char *name)
{
	unsigned i;

	for (i = 0; i < NB_REGISTERS; i++) {
		if (str_matches(name, debugger->registers_map[i].name)) {
			if (debugger->registers_map[i].size == 1)
				return *debugger->registers_map[i].value.v8;
			else
				return *debugger->registers_map[i].value.v16;
		}
	}

	return -1;
}

/* computes the value of an expression of type "*reg_name[+offset|-offset]" */
static bool compute_expression(const struct console_debugger *debugger,
		const char *expression, uint16_t *output)
{
	int nb_match;
	char start_str[10];
	int offset;
	int start;
	int address;
	int length;

	if (*expression == '\0')
		return false;

	nb_match = sscanf(expression, "%2s%i%n", start_str, &offset, &length);
	if (nb_match < 1) {
		printf("invalid value expression %s\n", expression);
		return false;
	}
	start = get_register_value(debugger, start_str);
	if (start == -1) {
		printf("\"%s\" doesn't name a register\n", start_str);
		return false;
	}
	if (nb_match == 1) {
		if (strlen(expression) != strlen(start_str)) {
			printf("Spurious chars \"%s\" at the end of expression "
					"\"%s\"\n",
					expression + strlen(start_str),
					expression);
			return false;
		}
		address = start;
	} else {
		if ((unsigned)length != strlen(expression)) {
			printf("Spurious chars \"%s\" at the end of expression "
					"\"%s\"\n", expression + length,
					expression);
			return false;
		}
		address = start + offset;
	}
	if (address < 0 || address > UINT16_MAX) {
		printf("address %s points outside the memory\n", expression);
		return false;
	}

	*output = address;

	return true;
}

static void console_debugger_assembler(struct console_debugger *debugger)
{
	const char *start_str;
	const char *stop_str;
	const char *tmp;
	uint16_t start;
	uint16_t stop;
	uint16_t i;
	uint16_t pc;
	uint8_t opcode;
	const struct s_cpu_z80 *instruction;
	unsigned cur;

	start_str = debugger->command.argv[1];
	stop_str = debugger->command.argv[2];

	if (!compute_expression(debugger, start_str, &start)) {
		printf("Invalid start expression \"%s\"\n", start_str);
		return;
	}
	if (!compute_expression(debugger, stop_str, &stop)) {
		printf("Invalid stop expression \"%s\"\n", stop_str);
		return;
	}
	/* swap if order is wrong */
	if (start > stop) {
		tmp = stop_str;
		i = stop;
		stop_str = start_str;
		stop = start;
		start_str = tmp;
		start = i;
	}
	printf("%s = 0x%04"PRIx16"\n", start_str, start);
	for (pc = start; pc <= stop; ) {
		opcode = read8bit(pc, debugger->gb);
		instruction = instructions + opcode;
		printf("[0x%04"PRIx16"] (0x%02"PRIx8") %s", pc, opcode,
				instruction->value);
		printf("\033[%dG", 53);
		for (cur = 1; cur <= instruction->size; cur++)
			printf(" %02"PRIx8, read8bit(pc + cur, debugger->gb));
		puts("");
		pc += instruction->size + 1;
	}
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

	printf("Breakpoint %u set at adress %#lx\n",
			(unsigned)(breakpoint - debugger->breakpoints), adress);

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

static void console_debugger_memory(struct console_debugger *debugger)
{
	const char *start_str;
	const char *stop_str;
	const char *tmp;
	uint16_t start;
	uint16_t stop;
	uint16_t i;

	start_str = debugger->command.argv[1];
	stop_str = debugger->command.argv[2];

	if (!compute_expression(debugger, start_str, &start)) {
		printf("Invalid start expression \"%s\"\n", start_str);
		return;
	}
	if (!compute_expression(debugger, stop_str, &stop)) {
		printf("Invalid stop expression \"%s\"\n", stop_str);
		return;
	}
	/* swap if order is wrong */
	if (start > stop) {
		tmp = stop_str;
		i = stop;
		stop_str = start_str;
		stop = start;
		start_str = tmp;
		start = i;
	}
	if (start % 2) /* only allow aligned accesses */
		start--;
	if (stop % 2)
		stop++;
	for (i = start; i <= stop; i += 2) {
		if (i == start)
			printf(" %s = 0x%04"PRIx16, start_str, start);
		else if (((i - start) % 0x10) == 0 && i != stop)
			printf(" 0x%04"PRIx16, i);
		if (i == stop)
			printf(" %s = 0x%04"PRIx16, stop_str, stop);
		printf("\033[%dG0x%04"PRIx16"\n", 40,
				read16bit(i, debugger->gb));
	}
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
	uint16_t address;

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
		if (*expression != '*' ||
				!compute_expression(debugger, expression + 1,
					&address))
			printf("Unable to print \"%s\".\n", expression);
		else
			printf("%s[%#.04x] = %#.02"PRIx16"\n", expression,
					address,
					read8bit(address, debugger->gb));
	}
}

static struct debugger_command commands[] = {
	{
		.fn = console_debugger_assembler,
		.name = "assembler",
		.help = "Shows the assembler code from one address to another.",
		.argc = 3,
	},
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
		.fn = console_debugger_enable,
		.name = "enable",
		.help = "Enables a breakpoint.\n"
			"\t\t\tusage: enable breakpoint_id.",
		.argc = 2,
	},
	{
		.fn = console_debugger_help,
		.name = "help",
		.help = "Shows a little help about available commands.",
		.argc = 1,
	},
	{
		.fn = console_debugger_memory,
		.name = "memory",
		.help = "Prints a memory region.\n"
			"\t\t\tusage: memory start stop\n"
			"\t\t\texample: memory *sp+0x50 0x200",
		.argc = 3,
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
			"\t\t\tusage: print {af,a,f,bc,b,c,de,d,e,hl,h,l,sp,"
			"pc}\n"
			"\t\t\t       print registers\n"
			"\t\t\t       print *REG_NAME[{+offset,-offset}]\n"
		        "\t\t\texamples: print pc\n"
		        "\t\t\t          print *sp-0x02",
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

static void init_registers_map(struct console_debugger *debugger)
{
	unsigned index;
	struct s_register *registers;

	registers = debugger->registers;
	index = 0;
	debugger->registers_map[index].name = "af";
	debugger->registers_map[index].value.v16 = &registers->af;
	debugger->registers_map[index++].size = sizeof(registers->af);

	debugger->registers_map[index].name = "a";
	debugger->registers_map[index].value.v8 = &registers->a;
	debugger->registers_map[index++].size = sizeof(registers->a);

	debugger->registers_map[index].name = "f";
	debugger->registers_map[index].value.v8 = &registers->f;
	debugger->registers_map[index++].size = sizeof(registers->f);

	debugger->registers_map[index].name = "bc";
	debugger->registers_map[index].value.v16 = &registers->bc;
	debugger->registers_map[index++].size = sizeof(registers->bc);

	debugger->registers_map[index].name = "b";
	debugger->registers_map[index].value.v8 = &registers->b;
	debugger->registers_map[index++].size = sizeof(registers->b);

	debugger->registers_map[index].name = "c";
	debugger->registers_map[index].value.v8 = &registers->c;
	debugger->registers_map[index++].size = sizeof(registers->c);

	debugger->registers_map[index].name = "de";
	debugger->registers_map[index].value.v16 = &registers->de;
	debugger->registers_map[index++].size = sizeof(registers->de);

	debugger->registers_map[index].name = "d";
	debugger->registers_map[index].value.v8 = &registers->d;
	debugger->registers_map[index++].size = sizeof(registers->d);

	debugger->registers_map[index].name = "e";
	debugger->registers_map[index].value.v8 = &registers->e;
	debugger->registers_map[index++].size = sizeof(registers->e);

	debugger->registers_map[index].name = "hl";
	debugger->registers_map[index].value.v16 = &registers->hl;
	debugger->registers_map[index++].size = sizeof(registers->hl);

	debugger->registers_map[index].name = "h";
	debugger->registers_map[index].value.v8 = &registers->h;
	debugger->registers_map[index++].size = sizeof(registers->h);

	debugger->registers_map[index].name = "l";
	debugger->registers_map[index].value.v8 = &registers->l;
	debugger->registers_map[index++].size = sizeof(registers->l);

	debugger->registers_map[index].name = "sp";
	debugger->registers_map[index].value.v16 = &registers->sp;
	debugger->registers_map[index++].size = sizeof(registers->sp);

	debugger->registers_map[index].name = "pc";
	debugger->registers_map[index].value.v16 = &registers->pc;
	debugger->registers_map[index++].size = sizeof(registers->pc);
}

int console_debugger_init(struct console_debugger *debugger,
		struct s_register *registers, struct s_gb *gb)
{
	struct editline *el = debugger->editline;

	memset(debugger, 0, sizeof(*debugger));
	debugger->registers = registers;
	debugger->gb = gb;
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

	init_registers_map(debugger);
	/*
	 * TODO enable debugger by default, this should be decided with a
	 * command-line switch
	 */
//	debugger->active = true;

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

static void console_debugger_check_op_undefined(
		struct console_debugger *debugger)
{
	uint8_t opcode;

	opcode = read8bit(debugger->registers->pc, debugger->gb);
	if (is_opcode_undefined(opcode)) {
		printf("opcode 0x%02"PRIx8" is undefined\n", opcode);
		debugger->active = true;
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
	console_debugger_check_op_undefined(debugger);

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

#endif /* EMGB_CONSOLE_DEBUGGER */

