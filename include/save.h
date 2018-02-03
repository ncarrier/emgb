#ifndef INCLUDE_SAVE_H_
#define INCLUDE_SAVE_H_
#include <stdio.h>

struct save_start {
};
struct save_end {
};

struct save_chunk {
	struct save_start *start;
	const struct save_end *end;
	const char *name;
};

int save_write_chunk(FILE *f, struct save_start *start,
		const struct save_end *end);
int save_read_chunk(FILE *f, struct save_start *start,
		const struct save_end *end);

#endif /* INCLUDE_SAVE_H_ */
