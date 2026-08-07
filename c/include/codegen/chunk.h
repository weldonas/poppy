#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stddef.h>

#include "lang/type_system.h"
#include "codegen/register.h"

struct chunk;

struct chunk *new_chunk();
void free_chunk(struct chunk *chunk);

void add_variable(struct chunk *chunk, struct variable var);
bool has_variable(struct chunk *chunk, struct variable var);

// this does not modify any register other than dest (including chunk_address)
char *variable_address(struct chunk *chunk, struct variable var, enum reg chunk_address, enum reg dest);

char *push_chunk(struct chunk *chunk);
char *pop_chunk();         

#endif
