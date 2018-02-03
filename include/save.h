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

typedef void (*save_action)(struct save_chunk *chunk, FILE *f);

void save_write_chunk(struct save_chunk *chunk, FILE *f);
void save_read_chunk(struct save_chunk *chunk, FILE *f);

#endif /* INCLUDE_SAVE_H_ */
