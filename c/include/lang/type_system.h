#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "data/map.h"
#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"

#define OUTER_TYPE_MAP parse_tree_string_symbol_table_value_map_map

struct string {
        char *data;
};

void free_string(struct string *s);

bool equals_string(const struct string *s1, const struct string *s2);

struct symbol_table_value {
        const struct type *type;
        bool is_defined;
};

struct variable {
        const char *string;
        const struct type *type;
};

void free_variable(struct variable *v);

DEFINE_LIST(string)
DEFINE_LIST(variable)

DEFINE_MAP(string, symbol_table_value);
DEFINE_MAP(parse_tree, MAP(string, symbol_table_value));

struct type_rule;
struct type_rule_condition;
struct type_system;

struct OUTER_TYPE_MAP *find_types(const struct type_system *const system, const struct parse_tree *tree);
struct LIST(variable) get_local_variables(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols);
struct LIST(variable) get_parameters(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols);

#define MAX_CONDITION_COUNT 16
#define MAX_SIDE_EFFECT_COUNT 16

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
const struct type_rule *new_index_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index);
const struct type_rule *new_param_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t current_index, int next_index); // -ive value for next_index makes it NULL 
const struct type_rule *new_function_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t ret_index, size_t param_index);

void free_type_rule(const struct type_rule *type_rule);

const struct type_system *new_type_system(const struct type_rule **rules, size_t rules_len);
void free_type_system(const struct type_system *type_system);

#endif
