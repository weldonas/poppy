#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"

#define MAX_CONDITION_COUNT 16

enum type_rule_condition_type {
        CONDITION_LENGTH,
        CONDITION_PARENT_SYMBOL,
        CONDITION_SYMBOL_AT,
        CONDITION_TYPE_AT
};

struct type_rule_condition {
        enum type_rule_condition_type type;
        union {
                size_t length;
                enum symbol parent_symbol;
                struct {
                        size_t index;
                        union {
                                enum symbol symbol;
                                bool (*is_valid)(const struct type *);
                        };
                };
        };
};

struct type_rule {
        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t conditions_len;
        const struct type *output_type;
};

struct type_system {
        const struct type_rule **rules;
        size_t rules_len;
};

const struct type_rule_condition *const new_length_condition(size_t length);
const struct type_rule_condition *const new_parent_symbol_condition(enum symbol parent_symbol);
const struct type_rule_condition *const new_symbol_at_condition(size_t index, enum symbol symbol);
const struct type_rule_condition *const new_type_at_condition(size_t index, bool (*is_valid)(const struct type *));

const struct type_rule *const new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type);
void free_type_rule(const struct type_rule *type_rule);

const struct type_system *const new_type_system(const struct type_rule **rules, size_t rules_len);
void free_type_system(const struct type_system *type_system);

const struct type *const find_type(const struct type_system *const system, struct parse_tree *tree);

#endif
