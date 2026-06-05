#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stddef.h>

#include "lang/type_system.h"
#include "codegen/register.h"

struct chunk;
struct word;

struct chunk *new_chunk();
void free_chunk(struct chunk *chunk);

struct word *new_word(struct variable variable, size_t word_index);
void free_word(struct word *word);
struct variable word_variable(struct word *word);

void add_variable(struct chunk *chunk, struct variable var);
bool has_variable(struct chunk *chunk, struct variable var);

char *push_chunk(struct chunk *chunk);
char *pop_chunk();

char *read_variable(struct chunk *chunk, enum reg into, struct word *word, enum reg chunk_address);
char *write_variable(struct chunk *chunk, struct word *word, enum reg from, enum reg chunk_address);

#endif
