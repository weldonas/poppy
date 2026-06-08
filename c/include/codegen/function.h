#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdbool.h>
#include <stddef.h>

#include "codegen/chunk.h"
#include "codegen/register.h"
#include "lang/type_system.h"
#include "data/list.h"

struct function;

struct function *new_function(struct LIST(variable) params, struct LIST(variable) vars, bool is_main);
void free_function(const struct function* function);

size_t num_params(const struct function *function);
char *read_function_variable(const struct function *function, enum reg reg, struct variable var);
char *write_function_variable(const struct function *function, struct variable var, enum reg reg);
char *function_variable_address(const struct function *function, struct variable var);
void set_body(struct function *function, char *body);
char *declare_function(const struct function *function);
char *call_function(const struct function *function, char **args);
char *hop(const struct function *function);
#endif
