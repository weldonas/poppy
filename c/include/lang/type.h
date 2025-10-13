#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stdlib.h>

#define MAX_PARAM_COUNT 29
#define MAX_TYPE_STRING_LEN MAX_PARAM_COUNT + 3
// + 1 for space, + 1 for return type, + 1 for null-termination

enum category {
    CATEGORY_PRIMITIVE,
    CATEGORY_FUNCTION
};

struct type {
    enum category category;
    union{
        struct {char repr;}; // primitive

        struct {
            struct type **params;
            size_t params_len;
            const struct type *ret_type;
        }; // function
    };
};

const struct type* const int_type();
const struct type* const bool_type();
const struct type* const void_type();
const struct type* const char_type();
const struct type* const function_type(const struct type *ret, const struct type *params[MAX_PARAM_COUNT], size_t params_len);
const struct type* const return_type(const struct type *type);
bool equals_type(const struct type *t1, const struct type *t2);
bool equals_arg_types(const struct type *args[MAX_PARAM_COUNT], size_t args_len, const struct type *type);
bool is_numeric(const struct type *type);
bool is_assignable(const struct type *type);
bool is_returnable(const struct type *type);
void free_types();
#endif
