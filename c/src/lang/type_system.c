#include "lang/type_system.h"
#include "data/map.h"
#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#define OUTER_TYPE_MAP_ENTRY parse_tree_string_symbol_table_value_map_map_entry
#define MAX_PRIORITY 7

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);                                                       \

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
        TYPE_RULE_INDEX,
        TYPE_RULE_PARAM,
        TYPE_RULE_FUNCTION
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
        };
};

const struct type *find_type(const struct type_system *const system, const struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, symbol_table_value) *scope_map);

bool equals_string(const struct string *s1, const struct string *s2) {
        return strcmp(s1->data, s2->data) == 0;
}

void free_string_entry(const struct MAP_ENTRY(string, symbol_table_value) *entry){
        free((void *) entry->key);
        free((void*) entry->value);
        free((void *) entry);
}

bool equals_parse_tree(const struct parse_tree *pt1, const struct parse_tree *pt2){
        return pt1 == pt2;
}

void free_typer_entry(const struct OUTER_TYPE_MAP_ENTRY *entry){
        free_map(entry->value, string, symbol_table_value);
        free((void *) entry->value);
        free((void *) entry);
}

struct MAP(string, symbol_table_value) * new_inner_map() {
        struct MAP(string, symbol_table_value) *ptr = (struct MAP(string, symbol_table_value)*) malloc(sizeof(struct MAP(string, symbol_table_value)));
        init_map(ptr, equals_string, free_string_entry, string, symbol_table_value);
        return ptr;
}

const struct type * find_symbol_type(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *outer_map){
        struct string string;
        string.data = tree->data.value;
        const struct parse_tree *cur = tree;
        while (cur != NULL){
                const struct MAP(string, symbol_table_value) *inner_map; query_map(outer_map, cur, inner_map, parse_tree, MAP(string, symbol_table_value));
                if (inner_map != NULL){
                        const struct symbol_table_value *value; query_map(inner_map, &string, value, string, symbol_table_value);

                        if (value != NULL){
                                return value->type;
                        }
                }
                cur = cur->parent;
        }
        return NULL;
}

const struct type * find_call_type(const struct type_system *const system, const struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map){
        // call -> IDENTIFIER LPAREN optargs RPAREN
        struct parse_tree *optargs; load_child_at(optargs, tree, 2);
        
        const struct type *args_type = find_type(system, optargs, outer_map, NULL);
        const struct type *ftype = find_symbol_type(tree->children->head->data, outer_map);

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

struct OUTER_TYPE_MAP * find_types(const struct type_system *const system, const struct parse_tree *tree){
        struct OUTER_TYPE_MAP *outer_map = (struct OUTER_TYPE_MAP*) malloc(sizeof (struct OUTER_TYPE_MAP));
        
        struct MAP(string, symbol_table_value) *inner_map = new_inner_map();
        init_map(outer_map, equals_parse_tree, free_typer_entry, parse_tree, MAP(string, symbol_table_value));
        update_map(outer_map, tree, inner_map, parse_tree, MAP(string, symbol_table_value));

        const struct type *program_type = find_type(system, tree, outer_map, inner_map);

        if (program_type == NULL){
                free_map(outer_map, parse_tree, MAP(string, symbol_table_value));
                free(outer_map);
                return NULL;
        }

        for (struct string_symbol_table_value_map_entry_list_node *map_node = inner_map->list->head; map_node != NULL; map_node = map_node->next){
                if (!map_node->data->value->is_defined){
                        free_map(outer_map, parse_tree, MAP(string, symbol_table_value));
                        free(outer_map);
                        return NULL;
                }
        }

        return outer_map;
}

void get_variables_recursive(const struct parse_tree *tree, struct LIST(variable) *list, const struct OUTER_TYPE_MAP *symbols){
        const struct MAP(string, symbol_table_value) *inner_map; query_map(symbols, tree, inner_map, parse_tree, MAP(string, symbol_table_value))

        if (inner_map != NULL){
                for (struct string_symbol_table_value_map_entry_list_node *map_node = inner_map->list->head; map_node != NULL; map_node = map_node->next){
                        struct variable *v = (struct variable*) malloc(sizeof(struct variable));
                        v->string = map_node->data->key->data;
                        v->type = map_node->data->value->type;
                        append_list(list, v, variable);
                }
        }

        if (tree->children != NULL){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        get_variables_recursive(node->data, list, symbols);
                }
        }
}

struct LIST(variable) get_local_variables(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols){
        // defn -> signature LBRACE stmts RBRACE
        const struct parse_tree *stmts; load_child_at(stmts, tree, 2);
        struct LIST(variable) list;
        init_list((&list))
        get_variables_recursive(stmts, &list, symbols);
        return list;
}

struct LIST(variable) get_parameters(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols){
        // defn -> signature LBRACE stmts RBRACE
        // signature -> type IDENTIFIER LPAREN optparams RPAREN
        const struct MAP(string, symbol_table_value) *inner_map; query_map(symbols, tree, inner_map, parse_tree, MAP(string, symbol_table_value))
        struct LIST(variable) list;
        init_list((&list))
        for (struct string_symbol_table_value_map_entry_list_node *map_node = inner_map->list->head; map_node != NULL; map_node = map_node->next){
                struct variable *v = (struct variable*) malloc(sizeof(struct variable));
                v->string = map_node->data->key->data;
                v->type = map_node->data->value->type;

                append_list((&list), v, variable);
        }
        return list;
}

#define MAX_CHILDREN 16

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);   

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

const struct type_rule *new_index_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_INDEX;
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
        const struct type *child_types[MAX_CHILDREN];
        bool child_type_computed[MAX_CHILDREN];
        const struct type_system *system;
        const struct parse_tree *tree;
        struct OUTER_TYPE_MAP *outer_map;
        struct MAP(string, symbol_table_value) *scope_map;
};

const struct type *get_child_type(struct application_data *data, size_t index){
        const struct parse_tree *child; load_child_at(child, data->tree, index);
        if (!data->child_type_computed[index]){
                data->child_types[index] = find_type(data->system, child, data->outer_map, data->scope_map);
                data->child_type_computed[index] = true;
        }

        return data->child_types[index];
}

const struct type *const apply(const struct type_rule *const type_rule, const struct type_system *system, const struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, symbol_table_value) *scope_map){
        struct application_data data = {0};
        data.system = system;
        data.tree = tree;
        data.outer_map = outer_map;
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
                                        struct MAP(string, symbol_table_value) *new_map = new_inner_map();
                                        update_map(outer_map, tree, new_map, parse_tree, MAP(string, symbol_table_value));
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
        else if (type_rule->type == TYPE_RULE_INDEX){
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

        return NULL;
}

const struct type *find_type(const struct type_system *const system, const struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, symbol_table_value) *scope_map){
        if (tree->data.type == SYMBOL_IDENTIFIER){
                return find_symbol_type(tree, outer_map);
        }
        else if (tree->data.type == SYMBOL_CALL){
                return find_call_type(system, tree, outer_map);
        }

        for (size_t i = 0; i < system->rules_len; ++i){
                const struct type *const output = apply(system->rules[i], system, tree, outer_map, scope_map);
                if (output){
                        return output;
                }
        }

        return NULL;
}

void free_string(struct string *s){}

void free_variable(struct variable *v){
        free(v);
}
