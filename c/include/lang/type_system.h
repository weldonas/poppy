#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_checker.h"

#define MAX_CONDITION_COUNT 16

enum type_rule_condition_type {
        CONDITION_LENGTH,
        CONDITION_PARENT_SYMBOL,
        CONDITION_SYMBOL_AT,
        CONDITION_TYPE_AT,
        CONDITION_TYPES_EQUAL_AT
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
                struct {
                        size_t index1;
                        size_t index2;
                };
        };
};

struct type_rule {
        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t conditions_len;
        bool has_output_type;
        union {
                const struct type *output_type;
                size_t output_index;
        };
};

struct type_system {
        const struct type_rule **rules;
        size_t rules_len;
};

const struct type_rule_condition *const new_length_condition(size_t length);
const struct type_rule_condition *const new_parent_symbol_condition(enum symbol parent_symbol);
const struct type_rule_condition *const new_symbol_at_condition(size_t index, enum symbol symbol);
const struct type_rule_condition *const new_type_at_condition(size_t index, bool (*is_valid)(const struct type *));
const struct type_rule_condition *const new_types_equal_at_condition(size_t index1, size_t index2);

const struct type_rule *const new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type);
const struct type_rule *const new_index_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index);
void free_type_rule(const struct type_rule *type_rule);

const struct type_system *const new_type_system(const struct type_rule **rules, size_t rules_len);
void free_type_system(const struct type_system *type_system);

const struct type *const find_type(const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map);

#endif
