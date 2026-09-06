#include "lang/type_system.h"
#include "data/map.h"
#include "data/result.h"
#include "lang/parse_tree.h"
#include "lang/symbol.h"
#include "lang/type.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#define MAX_PRIORITY 7

DEFINE_RESULT(type);

struct type_system {
        const struct type_rule **rules;
        uint8_t rules_len;
};

enum type_rule_condition_type : uint8_t {
        CONDITION_LENGTH,
        CONDITION_PARENT_SYMBOL,
        CONDITION_SYMBOL_AT,
        CONDITION_TYPE_AT,
        CONDITION_TYPES_EQUAL_AT,
        CONDITION_RETURN_TYPE_AT,
        SIDE_EFFECT_ADD_SCOPE
};

struct type_rule_condition {
        enum type_rule_condition_type type;
        union {
                uint8_t length;
                enum symbol parent_symbol;
                struct {
                        uint8_t index;
                        union {
                                enum symbol symbol;
                                bool (*is_valid)(const struct type *);
                        };
                };
                struct {
                        uint8_t index1;
                        uint8_t index2;
                };
                struct {
                        union {
                                uint8_t name_index;
                                const struct parse_tree *(*find_name_tree)(const struct parse_tree *);
                        };
                        uint8_t type_index;
                        bool is_defined;
                };
                struct {
                        uint8_t return_index;
                        uint8_t function_index;
                };
        };
};

enum type_rule_type : uint8_t {
        TYPE_RULE_PRIMITIVE,
        TYPE_RULE_CHILD,
        TYPE_RULE_DEDUCER
};

struct type_rule {
        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        uint8_t conditions_len;
        enum type_rule_type type;
        union {
                const struct type *output_type; // primitive
                uint8_t output_index; // child
                struct {
                        type_deducer deducer;
                }; // deducer
        };
};

struct RESULT(type) find_type(const struct type_system *const system, struct parse_tree *tree);

bool equals_parse_tree(const struct parse_tree *pt1, const struct parse_tree *pt2){
        return pt1 == pt2;
}

bool name_reused(const struct parse_tree *tree){
        struct string string;
        string.data = tree->data.value;

        const struct parse_tree *cur = tree;
        bool found = false;

        while (cur != NULL){
                const struct MAP(string, symbol_table_value) *symbol_table = cur->symbol_table; 
                if (symbol_table != NULL){
                        const struct symbol_table_value *value; query_map(symbol_table, &string, value, string, symbol_table_value);

                        if (value != NULL){
                                if (found){
                                        return true;
                                }
                                else {
                                        found = true;
                                }
                        }
                }
                cur = cur->parent;
        }
        return false;
}

struct RESULT(unit) is_valid_type_tree(const struct parse_tree *tree){
        if (tree->children){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        struct RESULT(unit) result = is_valid_type_tree(node->data);
                        if (!result.is_ok){
                                return result;
                        }
                }
        }

        if ((tree->data.type == SYMBOL_IDENTIFIER) && name_reused(tree)){
                char *lit = "Name reuse for symbol ";
                char *name = tree->data.value;
                char *err = malloc((strlen(lit) + strlen(name) + 1) * sizeof(char));
                strcpy(err, lit);
                strcat(err, name);
                struct RESULT(unit) result;
                make_error(result, err);
                return result;
        }

        if ((tree->data.type == SYMBOL_IDENTIFIER) && (strcmp(tree->data.value, "main") == 0)){
                const struct parse_tree *main_signature = tree->parent;
                if (!equals_type(main_signature->type->ret_type, void_type())){
                        struct RESULT(unit) result;
                        make_error_lit(result, "Incorrect return type for main function");
                        return result;
                }

                if (!equals_type(main_signature->type->params_type, unit_type())){
                        struct RESULT(unit) result;
                        make_error_lit(result, "Incorrect parameter type for main function");
                        return result;
                }
        }

        struct RESULT(unit) result;
        make_ok(result, 0);
        return result;
}

struct RESULT(unit) find_types(const struct type_system *const system, struct parse_tree *tree){
        struct MAP(string, symbol_table_value) *symbol_table = new_symbol_table();
        tree->symbol_table = symbol_table;

        struct RESULT(type) program_type = find_type(system, tree);

        if (!program_type.is_ok){
                struct RESULT(unit) result;
                make_error(result, program_type.error);
                return result;                
        }

        for (struct string_symbol_table_value_map_entry_list_node *map_node = symbol_table->list->head; map_node != NULL; map_node = map_node->next){
                if (!map_node->data->value->is_defined){
                        tree->type = NULL;
                        char *lit = "No definition for symbol ";
                        const char *name = map_node->data->key->data;
                        char *err = malloc((strlen(lit) + strlen(name) + 1) * sizeof(char));
                        strcpy(err, lit);
                        strcat(err, name);
                        struct RESULT(unit) result;
                        make_error(result, err);
                        return result;
                }
        }

        struct RESULT(unit) valid_result = is_valid_type_tree(tree);
        if (!valid_result.is_ok){
                tree->type = NULL;
                return valid_result;
        }

        return valid_result;
}

size_t get_priority(enum type_rule_condition_type type){
        switch(type){
                case CONDITION_LENGTH:
                case CONDITION_PARENT_SYMBOL:
                case CONDITION_SYMBOL_AT:
                        return 0;
                case SIDE_EFFECT_ADD_SCOPE:
                        return 1;
                case CONDITION_TYPE_AT:
                case CONDITION_TYPES_EQUAL_AT:
                        return 2;
                case CONDITION_RETURN_TYPE_AT:
                        return 3;
        }
        assert(0);
}

const struct type_rule_condition *new_type_rule_condition(struct type_rule_condition cond){
        struct type_rule_condition *ptr = malloc(sizeof(struct type_rule_condition));
        *ptr = cond;
        return ptr;
}

const struct type_rule_condition *new_length_condition(size_t length){
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_LENGTH, .length = length});
}

const struct type_rule_condition *new_parent_symbol_condition(enum symbol parent_symbol) {
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_PARENT_SYMBOL, .parent_symbol = parent_symbol});
}

const struct type_rule_condition *new_symbol_at_condition(size_t index, enum symbol symbol) {
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_SYMBOL_AT, .index = index, .symbol = symbol});
}

const struct type_rule_condition *new_type_at_condition(size_t index, bool (*is_valid)(const struct type *)) {
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_TYPE_AT, .index = index, .is_valid = is_valid});
}

const struct type_rule_condition *new_types_equal_at_condition(size_t index1, size_t index2) {
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_TYPES_EQUAL_AT, .index1 = index1, .index2 = index2});
}

const struct type_rule_condition *new_return_type_at_condition(size_t return_index, size_t function_index) {
        return new_type_rule_condition((struct type_rule_condition){.type = CONDITION_RETURN_TYPE_AT, .return_index = return_index, .function_index = function_index});
}

const struct type_rule_condition *new_add_scope_side_effect() {
        return new_type_rule_condition((struct type_rule_condition){.type = SIDE_EFFECT_ADD_SCOPE});
}

const struct symbol_table_value *new_symbol_table_value(const struct type *type, bool is_defined){
        struct symbol_table_value *value = malloc(sizeof(struct symbol_table_value));
        value->type = type;
        value->is_defined = is_defined;
        return value;
}

const struct type_rule *new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type){
        struct type_rule *ptr = malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_PRIMITIVE;
        ptr->output_type = output_type;
        return ptr;
}

const struct type_rule *new_child_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index){
        struct type_rule *ptr = malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_CHILD;
        ptr->output_index = output_index;
        return ptr;
}

const struct type_rule *new_deducer_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, type_deducer deducer){
        struct type_rule *ptr = malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_DEDUCER;
        ptr->deducer = deducer;
        return ptr;
}

void free_type_rule(const struct type_rule *type_rule){
        for (size_t i = 0; i < type_rule->conditions_len; ++i){
                free((void*) type_rule->conditions[i]);
        }

        free((void*) type_rule);
}

const struct type_system *new_type_system(const struct type_rule **rules, size_t rules_len){
        struct type_system *ptr = malloc(sizeof(struct type_system));
        ptr->rules = rules;
        ptr->rules_len = rules_len;
        return ptr;
}

void free_type_system(const struct type_system *type_system){
        for (size_t i = 0; i < type_system->rules_len; ++i){
                free_type_rule(type_system->rules[i]);
        }
        free((void*) type_system);
}

struct RESULT(type) get_child_type(const struct type_system *system, const struct parse_tree *tree, size_t index){
        struct parse_tree *child; load_child_at(child, tree, index);
        return find_type(system, child);
}

struct RESULT(type) apply(const struct type_rule *const type_rule, const struct type_system *system, struct parse_tree *tree){
        for (size_t priority = 0; priority <= MAX_PRIORITY; ++priority){
                for (size_t i = 0; i < type_rule->conditions_len; ++i) {
                        const struct type_rule_condition *condition = type_rule->conditions[i];

                        if (get_priority(condition->type) != priority){
                                continue;
                        }

                        struct parse_tree *child = NULL;
                        bool satisfied = false;

                        switch (condition->type) {
                                case CONDITION_LENGTH:
                                        if (tree->children){
                                                satisfied = tree->children->len == condition->length;
                                        }
                                        else {
                                                satisfied = condition->length == 0;
                                        }
                                        break;
                                case CONDITION_PARENT_SYMBOL:
                                        satisfied = tree->data.type == condition->parent_symbol;
                                        break;
                                case CONDITION_SYMBOL_AT:
                                        if (tree->children->len <= condition->index){
                                                satisfied = false;
                                        }
                                        else {
                                                load_child_at(child, tree, condition->index);
                                                satisfied = child->data.type == condition->symbol;
                                        }
                                        break;
                                case CONDITION_TYPE_AT:
                                        if (tree->children->len <= condition->index){
                                                satisfied = false;
                                        }
                                        else {
                                                load_child_at(child, tree, condition->index);
                                                struct RESULT(type) child_type_result = get_child_type(system, tree, condition->index);
                                                if (!child_type_result.is_ok){
                                                        return child_type_result;
                                                }
                                                satisfied = condition->is_valid(child_type_result.value);
                                        }
                                        break;
                                case CONDITION_TYPES_EQUAL_AT:
                                        struct RESULT(type) child_type_result1 = get_child_type(system, tree, condition->index1);
                                        struct RESULT(type) child_type_result2 = get_child_type(system, tree, condition->index2);
                                        if (!child_type_result1.is_ok){
                                                return child_type_result1;
                                        }
                                        if (!child_type_result2.is_ok){
                                                return child_type_result2;
                                        }

                                        satisfied = child_type_result1.value && child_type_result2.value && equals_type(child_type_result1.value, child_type_result2.value);
                                        break;
                                case CONDITION_RETURN_TYPE_AT:
                                        struct RESULT(type) fn_type_result = get_child_type(system, tree, condition->function_index);
                                        struct RESULT(type) ret_type_result = get_child_type(system, tree, condition->return_index);
                                        if (!fn_type_result.is_ok){
                                                return fn_type_result;
                                        }
                                        if (!ret_type_result.is_ok){
                                                return ret_type_result;
                                        }

                                        satisfied = fn_type_result.value && ret_type_result.value && equals_type(return_type(fn_type_result.value), ret_type_result.value);
                                        break;
                                case SIDE_EFFECT_ADD_SCOPE:
                                        struct MAP(string, symbol_table_value) *new_map = new_symbol_table();
                                        tree->symbol_table = new_map;
                                        satisfied = true;
                                        break;
                        }

                        if (!satisfied) {
                                struct RESULT(type) result;
                                make_ok(result, NULL);
                                return result;
                        }
                }
        }

        if (type_rule->type == TYPE_RULE_PRIMITIVE) {
                struct RESULT(type) result;
                make_ok(result, type_rule->output_type);
                return result;
        }
        else if (type_rule->type == TYPE_RULE_CHILD){
                return get_child_type(system, tree, type_rule->output_index);
        }
        else if (type_rule->type == TYPE_RULE_DEDUCER){
                if (tree->children){
                        for (size_t i = 0; i < tree->children->len; ++i){
                                struct RESULT(type) result = get_child_type(system, tree, i);
                                if (!result.is_ok){
                                        return result;
                                }
                        }
                }

                struct RESULT(type) result;
                make_ok(result, type_rule->deducer(tree));
                return result;
        }

        assert(0);
        struct RESULT(type) result;
        make_error_lit(result, "Cannot handle type rule");
        return result;
}

struct RESULT(type) find_type(const struct type_system *const system, struct parse_tree *tree){
        if (tree->type){
                struct RESULT(type) result;
                make_ok(result, tree->type);
                return result;
        }
        
        for (size_t i = 0; i < system->rules_len; ++i){
                struct RESULT(type) current_result = apply(system->rules[i], system, tree);
                if (current_result.is_ok && current_result.value != NULL){
                        tree->type = current_result.value;
                        return current_result;
                }
                if (!current_result.is_ok){
                        return current_result;
                }
        }

        struct RESULT(type) result;
        if (tree->children){
                print_parse_tree(tree);
                char *lit = "Could not find type for subtree ";
                char *tree_str = parse_tree_string(tree);
                char *err = malloc((strlen(lit) + strlen(tree_str) + 1) * sizeof(char));
                strcpy(err, lit);
                strcat(err, tree_str);
                free(tree_str);
                make_error(result, err);
        }
        else {
                make_ok(result, NULL);
        }
        return result;
}
