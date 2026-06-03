#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stdlib.h>

enum category {
    CATEGORY_PRIMITIVE,
    CATEGORY_FUNCTION,
    CATEGORY_PARAMS
};

struct type {
    enum category category;
    size_t word_count;
    union{
        struct {
            char repr;
        }; // primitive

        struct {
            const struct type *params_type; // params (or NULL if no arguments)
            const struct type *ret_type; // primitive
        }; // function

        struct {
            const struct type *current_type; // primitive
            const struct type *previous; // params or NULL
        }; // params
    };
};

const struct type* const int_type();
const struct type* const bool_type();
const struct type* const void_type();
const struct type* const char_type();
const struct type* const function_type(const struct type *ret, const struct type *params);
const struct type* const param_type(const struct type *current, const struct type *previous);
const struct type* const return_type(const struct type *type);
bool equals_type(const struct type *t1, const struct type *t2);
bool is_numeric(const struct type *type);
bool is_assignable(const struct type *type);
bool is_returnable(const struct type *type);
void free_types();
#endif
