#include "lang/type_system.h"
#include "data/map.h"
#include "lang/symbol.h"
#include "lang/type.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#define MAX_PRIORITY 7

struct type_system {
        const struct type_rule **rules;
        size_t rules_len;
};

enum type_rule_condition_type {
        CONDITION_LENGTH,
        CONDITION_PARENT_SYMBOL,
        CONDITION_SYMBOL_AT,
        CONDITION_TYPE_AT,
        CONDITION_TYPES_EQUAL_AT,
        CONDITION_RETURN_TYPE_AT,
        SIDE_EFFECT_ADD_SYMBOL_NAME_INDEX,
        SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION,
        SIDE_EFFECT_ADD_SCOPE
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
                struct {
                        union {
                                size_t name_index;
                                char *(*find_name)(const struct parse_tree *);
                        };
                        size_t type_index;
                        bool is_defined;
                };
                struct {
                        size_t return_index;
                        size_t function_index;
                };
        };
};

enum type_rule_type {
        TYPE_RULE_PRIMITIVE,
        TYPE_RULE_CHILD,
        TYPE_RULE_PARAM,
        TYPE_RULE_FUNCTION,
        TYPE_RULE_ARRAY,
        TYPE_RULE_ELEMENT
};

struct type_rule {
        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t conditions_len;
        enum type_rule_type type;
        union {
                const struct type *output_type;
                size_t output_index;
                struct {
                        size_t current_index;
                        int next_index;
                };
                struct {
                        size_t ret_index;
                        size_t param_index;
                };
                struct {
                        size_t element_index;
                        size_t length_index;
                };
                size_t array_index;
        };
};

const struct type *find_type(const struct type_system *const system, struct parse_tree *tree, struct MAP(string, symbol_table_value) *scope_map);

bool equals_parse_tree(const struct parse_tree *pt1, const struct parse_tree *pt2){
        return pt1 == pt2;
}

const struct type * find_symbol_type(const struct parse_tree *tree){
        struct string string;
        string.data = tree->data.value;

        const struct parse_tree *cur = tree;
        
        while (cur != NULL){
                const struct MAP(string, symbol_table_value) *symbol_table = cur->symbol_table; 
                if (symbol_table != NULL){
                        const struct symbol_table_value *value; query_map(symbol_table, &string, value, string, symbol_table_value);

                        if (value != NULL){
                                return value->type;
                        }
                }
                cur = cur->parent;
        }
        return NULL;
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

struct variable find_symbol_variable(const struct parse_tree *tree){
        const struct type *type = find_symbol_type(tree);
        struct variable var = {tree->data.value, type};
        return var;
}

const struct type * find_call_type(const struct type_system *const system, const struct parse_tree *tree){
        // call -> IDENTIFIER LPAREN optargs RPAREN
        struct parse_tree *optargs; load_child_at(optargs, tree, 2);
        
        const struct type *args_type = find_type(system, optargs, NULL);
        const struct type *ftype = find_symbol_type(tree->children->head->data);

        if (ftype == NULL){
                return NULL;
        }

        if ((args_type == NULL) != (ftype->params_type == NULL)){
                return NULL;
        }

        if (args_type && !equals_type(args_type, ftype->params_type)){
                return NULL;
        }

        return return_type(ftype);
}

bool contains_collision(const struct parse_tree *tree){
        if (tree->children){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        if (contains_collision(node->data)){
                                return true;
                        }
                }
        }

        if ((tree->data.type == SYMBOL_IDENTIFIER) && name_reused(tree)){
                return true;
        }

        return false;
}

void find_types(const struct type_system *const system, struct parse_tree *tree){
        struct MAP(string, symbol_table_value) *symbol_table = new_symbol_table();
        tree->symbol_table = symbol_table;

        const struct type *program_type = find_type(system, tree, symbol_table);

        if (program_type == NULL){
                return;
        }

        for (struct string_symbol_table_value_map_entry_list_node *map_node = symbol_table->list->head; map_node != NULL; map_node = map_node->next){
                if (!map_node->data->value->is_defined){
                        tree->type = NULL;
                        return;
                }
        }

        if (contains_collision(tree)){
                tree->type = NULL;
        }
}

#define MAX_CHILDREN 16

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
                case SIDE_EFFECT_ADD_SYMBOL_NAME_INDEX:
                case SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION:
                        return 3;
                case CONDITION_RETURN_TYPE_AT:
                        return 4;
        }
        assert(0);
}

const struct type_rule_condition *new_type_rule_condition(struct type_rule_condition cond){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
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

const struct type_rule_condition *new_add_symbol_name_index_side_effect(size_t name_index, size_t type_index, bool is_defined) {
        return new_type_rule_condition((struct type_rule_condition){.type = SIDE_EFFECT_ADD_SYMBOL_NAME_INDEX, .name_index = name_index, .type_index = type_index, .is_defined = is_defined});
}

const struct type_rule_condition *new_add_symbol_name_function_side_effect(char *(*find_name)(const struct parse_tree *), size_t type_index, bool is_defined) {
        return new_type_rule_condition((struct type_rule_condition){.type = SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION, .find_name = find_name, .type_index = type_index, .is_defined = is_defined});
}

const struct type_rule_condition *new_add_scope_side_effect() {
        return new_type_rule_condition((struct type_rule_condition){.type = SIDE_EFFECT_ADD_SCOPE});
}

const struct symbol_table_value *new_symbol_table_value(const struct type *type, bool is_defined){
        struct symbol_table_value *value = (struct symbol_table_value*) malloc(sizeof(struct symbol_table_value));
        value->type = type;
        value->is_defined = is_defined;
        return value;
}

const struct type_rule *new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_PRIMITIVE;
        ptr->output_type = output_type;
        return ptr;
}

const struct type_rule *new_child_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_CHILD;
        ptr->output_index = output_index;
        return ptr;
}

const struct type_rule *new_param_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t current_index, int next_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_PARAM;
        ptr->current_index = current_index;
        ptr->next_index = next_index;
        return ptr;
}

const struct type_rule *new_function_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t ret_index, size_t param_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_FUNCTION;
        ptr->ret_index = ret_index;
        ptr->param_index = param_index;
        return ptr;
}

const struct type_rule *new_array_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t element_index, size_t length_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_ARRAY;
        ptr->element_index = element_index;
        ptr->length_index = length_index;
        return ptr;
}

const struct type_rule *new_element_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t array_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_ELEMENT;
        ptr->array_index = array_index;
        return ptr;
}

void free_type_rule(const struct type_rule *type_rule){
        for (size_t i = 0; i < type_rule->conditions_len; ++i){
                free((void*) type_rule->conditions[i]);
        }

        free((void*) type_rule);
}

const struct type_system *new_type_system(const struct type_rule **rules, size_t rules_len){
        struct type_system *ptr = (struct type_system*) malloc(sizeof(struct type_system));
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

struct application_data {
        const struct type_system *system;
        const struct parse_tree *tree;
        struct MAP(string, symbol_table_value) *scope_map;
};

const struct type *get_child_type(struct application_data *data, size_t index){
        struct parse_tree *child; load_child_at(child, data->tree, index);
        return find_type(data->system, child, data->scope_map);
}

const struct type *const apply(const struct type_rule *const type_rule, const struct type_system *system, struct parse_tree *tree, struct MAP(string, symbol_table_value) *scope_map){
        struct application_data data = {0};
        data.system = system;
        data.tree = tree;
        data.scope_map = scope_map;
        
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
                                        load_child_at(child, tree, condition->index);
                                        satisfied = child->data.type == condition->symbol;
                                        break;
                                case CONDITION_TYPE_AT:
                                        load_child_at(child, tree, condition->index);
                                        satisfied = condition->is_valid(get_child_type(&data, condition->index));
                                        break;
                                case CONDITION_TYPES_EQUAL_AT:
                                        const struct type *child_type1 = get_child_type(&data, condition->index1);
                                        const struct type *child_type2 = get_child_type(&data, condition->index2);
                                        satisfied = child_type1 && child_type2 && equals_type(child_type1, child_type2);
                                        break;
                                case CONDITION_RETURN_TYPE_AT:
                                        const struct type *fn_type = get_child_type(&data, condition->function_index);
                                        const struct type *ret_type = get_child_type(&data, condition->return_index);
                                        satisfied = fn_type && ret_type && equals_type(return_type(fn_type), ret_type);
                                        break;
                                case SIDE_EFFECT_ADD_SYMBOL_NAME_INDEX:
                                        struct parse_tree *child; load_child_at(child, tree, condition->name_index);
                                        struct string *str = (struct string*) malloc(sizeof(struct string));
                                        str->data = child->data.value;
                                        const struct symbol_table_value *v; query_map(scope_map, str, v, string, symbol_table_value);
                                        // NOTE: this adds to the enclosing scope, not any new scope created
                                        const struct symbol_table_value *new_value = new_symbol_table_value(get_child_type(&data, condition->type_index), condition->is_defined);

                                        if ((v != NULL) && ((v->is_defined) || !equals_type(v->type, new_value->type))){
                                                free(str);
                                                free((void*) new_value);
                                                return NULL;
                                        }
                                        update_map(scope_map, str, new_value, string, symbol_table_value);
                                        satisfied = true;
                                        break;
                                case SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION: {
                                        struct string *str = (struct string*) malloc(sizeof(struct string));
                                        str->data = condition->find_name(tree);
                                        const struct symbol_table_value *v; query_map(scope_map, str, v, string, symbol_table_value);
                                        // NOTE: this adds to the enclosing scope, not any new scope created
                                        const struct symbol_table_value *new_value = new_symbol_table_value(get_child_type(&data, condition->type_index), condition->is_defined);

                                        if ((v != NULL) && ((v->is_defined) || !equals_type(v->type, new_value->type))){
                                                free(str);
                                                free((void*) new_value);
                                                return NULL;
                                        }
                                        update_map(scope_map, str, new_value, string, symbol_table_value);
                                }
                                        satisfied = true;
                                        break;
                                case SIDE_EFFECT_ADD_SCOPE:
                                        struct MAP(string, symbol_table_value) *new_map = new_symbol_table();
                                        tree->symbol_table = new_map;
                                        data.scope_map = new_map;
                                        satisfied = true;
                                        break;
                        }

                        if (!satisfied) {
                                return NULL;
                        }
                }
        }

        if (type_rule->type == TYPE_RULE_PRIMITIVE) {
                return type_rule->output_type;
        }
        else if (type_rule->type == TYPE_RULE_CHILD){
                return get_child_type(&data, type_rule->output_index);
        }
        else if (type_rule->type == TYPE_RULE_PARAM){
                size_t current = type_rule->current_index;
                int next = type_rule->next_index;
                const struct type *next_type;

                get_child_type(&data, current);
                if (next < 0){
                        next_type = NULL;
                }
                else {
                        next_type = get_child_type(&data, next);
                }

                if (!get_child_type(&data, current) || (!next_type && (next >= 0))){
                        return NULL;
                }

                return param_type(get_child_type(&data, current), next_type);            
        }
        else if (type_rule->type == TYPE_RULE_FUNCTION){
                if (!get_child_type(&data, type_rule->ret_index)){
                        return NULL;
                }
                return function_type(get_child_type(&data, type_rule->ret_index), get_child_type(&data, type_rule->param_index));          
        }
        else if (type_rule->type == TYPE_RULE_ARRAY){
                if (!get_child_type(&data, type_rule->element_index)){
                        return NULL;
                }

                struct parse_tree *length_tree; load_child_at(length_tree, tree, type_rule->length_index);
                return array_type(get_child_type(&data, type_rule->element_index), length_tree->data.value);
        }
        else if (type_rule->type == TYPE_RULE_ELEMENT){
                if (!get_child_type(&data, type_rule->array_index)){
                        return NULL;
                }
                return get_child_type(&data, type_rule->array_index)->element_type;
        }

        return NULL;
}

const struct type *find_type(const struct type_system *const system, struct parse_tree *tree, struct MAP(string, symbol_table_value) *scope_map){
        if (tree->type){
                return tree->type;
        }
        
        if (tree->data.type == SYMBOL_IDENTIFIER){
                tree->type = find_symbol_type(tree);
                return tree->type;
        }
        else if (tree->data.type == SYMBOL_CALL){
                tree->type = find_call_type(system, tree);
                return tree->type;
        }

        for (size_t i = 0; i < system->rules_len; ++i){
                const struct type *const output = apply(system->rules[i], system, tree, scope_map);
                if (output){
                        tree->type = output;
                        return tree->type;
                }
        }

        return NULL;
}
