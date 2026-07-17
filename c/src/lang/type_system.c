#include "lang/type_system.h"
#include "data/map.h"
#include "lang/parse_tree.h"
#include "lang/symbol.h"
#include "lang/type.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#define MAX_PRIORITY 7

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
        SIDE_EFFECT_ADD_SYMBOL_NAME_INDEX,
        SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION,
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

const struct type *find_type(const struct type_system *const system, struct parse_tree *tree, struct MAP(string, symbol_table_value) *scope_map);

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

bool is_valid_type_tree(const struct parse_tree *tree){
        if (tree->children){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        if (!is_valid_type_tree(node->data)){
                                return false;
                        }
                }
        }

        if ((tree->data.type == SYMBOL_IDENTIFIER) && name_reused(tree)){
                return false;
        }

        if ((tree->data.type == SYMBOL_IDENTIFIER) && (strcmp(tree->data.value, "main") == 0)){
                const struct parse_tree *main_signature = tree->parent;
                if (!equals_type(main_signature->type->ret_type, void_type())){
                        return false;
                }

                if (!equals_type(main_signature->type->params_type, unit_type())){
                        return false;
                }
        }

        return true;
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

        if (!is_valid_type_tree(tree)){
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

const struct type_rule_condition *new_add_symbol_name_function_side_effect(const struct parse_tree *(*find_name_tree)(const struct parse_tree *), size_t type_index, bool is_defined) {
        return new_type_rule_condition((struct type_rule_condition){.type = SIDE_EFFECT_ADD_SYMBOL_NAME_FUNCTION, .find_name_tree = find_name_tree, .type_index = type_index, .is_defined = is_defined});
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

const struct type_rule *new_deducer_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, type_deducer deducer){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
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
                                                satisfied = condition->is_valid(get_child_type(&data, condition->index));
                                        }
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
                                        get_child_type(&data, condition->name_index);
                                        struct parse_tree *child; load_child_at(child, tree, condition->name_index);

                                        if (child->type){
                                                return NULL;
                                        }

                                        struct string *str = (struct string*) malloc(sizeof(struct string));
                                        str->data = child->data.value;
                                        const struct symbol_table_value *v; query_map(scope_map, str, v, string, symbol_table_value);
                                        // NOTE: this adds to the enclosing scope, not any new scope created
                                        const struct type *new_type = get_child_type(&data, condition->type_index);
                                        new_type = make_assignable(new_type);
                                        const struct symbol_table_value *new_value = new_symbol_table_value(new_type, condition->is_defined);

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
                                        str->data = condition->find_name_tree(tree)->data.value;
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
        else if (type_rule->type == TYPE_RULE_DEDUCER){
                if (tree->children){
                        for (size_t i = 0; i < tree->children->len; ++i){
                                get_child_type(&data, i);
                        }
                }

                return type_rule->deducer(tree);
        }

        return NULL;
}

const struct type *find_type(const struct type_system *const system, struct parse_tree *tree, struct MAP(string, symbol_table_value) *scope_map){
        if (tree->type){
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
