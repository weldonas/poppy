#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "data/list.h"

#define NOT_IN_MEMORY -1

enum category : uint8_t {
    CATEGORY_PRIMITIVE,
    CATEGORY_FUNCTION,
    CATEGORY_PARAMS,
    CATEGORY_ARRAY,
    CATEGORY_UNIT,
    CATEGORY_RECORD,
    CATEGORY_POINTER
};

struct variable {
        const char *string;
        const struct type *type;
};

void free_variable(struct variable *v);

DEFINE_LIST(type);
DEFINE_LIST(variable)

struct type {
    uint32_t byte_count;
    enum category category;
    bool is_assignable;
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
            const struct type *element_type;
            uint32_t length;
        }; // array

        struct {
            struct LIST(variable) fields;
            char *name;
        }; // record

        struct {
            const struct type *referenced_type;
        }; // pointer
    };
};

const struct type* const int_type();
const struct type* const bool_type();
const struct type* const void_type();
const struct type* const char_type();

const struct type* const unit_type();

const struct type* const function_type(const struct type *ret, const struct type *params);
const struct type* const return_type(const struct type *type);

struct type* const param_type();
void add_param(struct type *params, const struct type *type_to_add);

const struct type* const array_type(const struct type *element_type, char *length);

struct type* const record_type();
bool add_field(struct type *record, struct variable *v);
void name_record_type(struct type *record, char *name);
const struct type *query_record_type(const char *name);
const struct type *field_type(const struct type *record, const char *name);
size_t record_type_offset(const struct type *record, const char *name);

const struct type* const pointer_type(const struct type *type);

const struct type *make_assignable(const struct type *type);

bool equals_type(const struct type *t1, const struct type *t2);
bool is_numeric(const struct type *type);
bool is_returnable(const struct type *type);
bool is_composite(const struct type *type);
bool can_safe_cast(const struct type *src, const struct type *dst);
void free_types();
#endif
