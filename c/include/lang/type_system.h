#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "data/map.h"
#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"

struct type_rule;
struct type_rule_condition;
struct type_system;

void find_types(const struct type_system *const system, struct parse_tree *tree);

#define MAX_CONDITION_COUNT 16

typedef const struct type *(*type_deducer)(const struct parse_tree *tree);

const struct type_rule_condition *new_length_condition(size_t length);
const struct type_rule_condition *new_parent_symbol_condition(enum symbol parent_symbol);
const struct type_rule_condition *new_symbol_at_condition(size_t index, enum symbol symbol);
const struct type_rule_condition *new_type_at_condition(size_t index, bool (*is_valid)(const struct type *));
const struct type_rule_condition *new_types_equal_at_condition(size_t index1, size_t index2);
const struct type_rule_condition *new_return_type_at_condition(size_t return_index, size_t function_index);

const struct type_rule_condition *new_add_symbol_name_index_side_effect(size_t name_index, size_t type_index, bool is_defined);
const struct type_rule_condition *new_add_symbol_name_function_side_effect(char *(*find_name)(const struct parse_tree *), size_t type_index, bool is_defined);
const struct type_rule_condition *new_add_scope_side_effect();

const struct type_rule *new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type);
const struct type_rule *new_child_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index);
const struct type_rule *new_deducer_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, type_deducer deducer);

void free_type_rule(const struct type_rule *type_rule);

const struct type_system *new_type_system(const struct type_rule **rules, size_t rules_len);
void free_type_system(const struct type_system *type_system);

#endif
