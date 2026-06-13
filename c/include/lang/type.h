#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stdlib.h>

#include "data/list.h"

enum category {
    CATEGORY_PRIMITIVE,
    CATEGORY_FUNCTION,
    CATEGORY_PARAMS,
    CATEGORY_ARRAY,
    CATEGORY_UNIT,
    CATEGORY_RECORD
};

struct variable {
        const char *string;
        const struct type *type;
};

void free_variable(struct variable *v);

DEFINE_LIST(type);
DEFINE_LIST(variable)

struct type {
    enum category category;
    size_t word_count;
    union{
        struct {
            char repr;
        }; // primitive

        struct {
            const struct type *params_type; // params (or unit type if no arguments)
            const struct type *ret_type; // primitive
        }; // function

        struct {
            struct LIST(type) subtypes;
            // const struct type *current_type; // primitive
            // const struct type *previous; // params or NULL
        }; // params

        struct {
            const struct type *element_type; // primitive or array
            size_t length;
        }; // array

        struct LIST(variable) fields; // record
    };
};

const struct type* const int_type();
const struct type* const bool_type();
const struct type* const void_type();
const struct type* const char_type();
const struct type* const unit_type();
const struct type* const function_type(const struct type *ret, const struct type *params);

struct type* const param_type();
void add_param(struct type *params, const struct type *type_to_add);

const struct type* const array_type(const struct type *element_type, char *length);
const struct type* const record_type(const struct LIST(variable) fields);
const struct type* const return_type(const struct type *type);
bool equals_type(const struct type *t1, const struct type *t2);
bool is_numeric(const struct type *type);
bool is_assignable(const struct type *type);
bool is_returnable(const struct type *type);
void free_types();
#endif
