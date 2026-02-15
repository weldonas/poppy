#include "lang/type_system.h"
#include "data/map.h"
#include "lang/parser.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_checker.h"

#include <assert.h>
#include <stddef.h>

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
                case SIDE_EFFECT_ADD_SYMBOL:
                        return 3;
        }
        assert(0);
}

const struct type_rule_condition *const new_length_condition(size_t length){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = CONDITION_LENGTH;
        ptr->length = length;
        return ptr;
}

const struct type_rule_condition *const new_parent_symbol_condition(enum symbol parent_symbol){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = CONDITION_PARENT_SYMBOL;
        ptr->parent_symbol = parent_symbol;
        return ptr;
}

const struct type_rule_condition *const new_symbol_at_condition(size_t index, enum symbol symbol){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = CONDITION_SYMBOL_AT;
        ptr->index = index;
        ptr->symbol = symbol;
        return ptr;
}

const struct type_rule_condition *const new_type_at_condition(size_t index, bool (*is_valid)(const struct type *)){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = CONDITION_TYPE_AT;
        ptr->index = index;
        ptr->is_valid = is_valid;
        return ptr; 
}
const struct type_rule_condition *const new_types_equal_at_condition(size_t index1, size_t index2){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = CONDITION_TYPES_EQUAL_AT;
        ptr->index1 = index1;
        ptr->index2 = index2;
        return ptr;  
}

const struct type_rule_condition *const new_add_symbol_side_effect(size_t name_index, size_t type_index){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = SIDE_EFFECT_ADD_SYMBOL;
        ptr->name_index = name_index;
        ptr->type_index = type_index;
        return ptr;
}

const struct type_rule_condition *const new_add_scope_side_effect(){
        struct type_rule_condition *ptr = (struct type_rule_condition*) malloc(sizeof(struct type_rule_condition));
        ptr->type = SIDE_EFFECT_ADD_SCOPE;
        return ptr;
}

const struct type_rule *const new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_PRIMITIVE;
        ptr->output_type = output_type;
        return ptr;
}

const struct type_rule *const new_index_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->type = TYPE_RULE_INDEX;
        ptr->output_index = output_index;
        return ptr;
}

const struct type_rule *const new_param_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t current_index, int next_index){
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

void free_type_rule(const struct type_rule *type_rule){
        for (size_t i = 0; i < type_rule->conditions_len; ++i){
                free((void*) type_rule->conditions[i]);
        }

        free((void*) type_rule);
}

const struct type_system *const new_type_system(const struct type_rule **rules, size_t rules_len){
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

const struct type *const apply(const struct type_rule *const type_rule, const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        const struct type *child_types[MAX_CHILDREN] = {0};
        bool child_type_computed[MAX_CHILDREN] = {0};

        struct MAP(string, type) *map_to_use = scope_map;

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
                                        if (!child_type_computed[condition->index]) {
                                                child_types[condition->index] = find_type(system, child, outer_map, map_to_use);
                                                child_type_computed[condition->index] = true;
                                        }
                                        satisfied = condition->is_valid(child_types[condition->index]);
                                        break;
                                case CONDITION_TYPES_EQUAL_AT:
                                        struct parse_tree *child1; load_child_at(child1, tree, condition->index1);
                                        struct parse_tree *child2; load_child_at(child2, tree, condition->index2);
                                        if (!child_type_computed[condition->index1]) {
                                                child_types[condition->index1] = find_type(system, child1, outer_map, map_to_use);
                                                child_type_computed[condition->index1] = true;
                                        }
                                        if (!child_type_computed[condition->index2]) {
                                                child_types[condition->index2] = find_type(system, child2, outer_map, map_to_use);
                                                child_type_computed[condition->index2] = true;
                                        }
                                        satisfied = child_types[condition->index1] && child_types[condition->index2] && equals_type(child_types[condition->index1], child_types[condition->index2]);
                                        break;
                                case SIDE_EFFECT_ADD_SYMBOL:
                                        struct parse_tree *child; load_child_at(child, tree, condition->name_index);
                                        struct string *str = (struct string*) malloc(sizeof(struct string));
                                        str->data = child->data.value;
                                        const struct type *t; query_map(map_to_use, str, t, string, type);
                                        if (t != NULL){
                                                free(str);
                                                return NULL;
                                        }
                                        if (!child_type_computed[condition->type_index]) {
                                                load_child_at(child, tree, condition->type_index);
                                                child_types[condition->type_index] = find_type(system, child, outer_map, map_to_use);
                                                child_type_computed[condition->type_index] = true;
                                        }
                                        update_map(map_to_use, str, child_types[condition->type_index], string, type);
                                        satisfied = true;
                                        break;
                                case SIDE_EFFECT_ADD_SCOPE:
                                        struct MAP(string, type) *new_map = new_inner_map();
                                        update_map(outer_map, tree, new_map, parse_tree, MAP(string, type));
                                        map_to_use = new_map;
                                        satisfied = true;
                                        break;
                        }

                        if (!satisfied) {
                                if (map_to_use != scope_map){
                                        free_map(map_to_use, string, type);
                                        free(map_to_use);
                                }
                                return NULL;
                        }
                }
        }

        if (type_rule->type == TYPE_RULE_PRIMITIVE) {
                // if (!type_rule->output_type){
                //         if (map_to_use != scope_map){
                //                 free_map(map_to_use, string, type);
                //                 free(map_to_use);
                //         }
                // }

                return type_rule->output_type;
        }
        else if (type_rule->type == TYPE_RULE_INDEX){
                size_t out = type_rule->output_index;

                if (!child_type_computed[out]) {
                        struct parse_tree *child;
                        load_child_at(child, tree, out);
                        child_types[out] = find_type(system, child, outer_map, map_to_use);
                }

                if (!child_types[out]){
                        if (map_to_use != scope_map){
                                free_map(map_to_use, string, type);
                                free(map_to_use);
                        }
                }

                return child_types[out];
        }
        else if (type_rule->type == TYPE_RULE_PARAM){
                size_t current = type_rule->current_index;
                int next = type_rule->next_index;

                if (!child_type_computed[current]) {
                        struct parse_tree *child;
                        load_child_at(child, tree, current);
                        child_types[current] = find_type(system, child, outer_map, map_to_use);
                }

                if (next < 0){
                        child_types[next] = NULL;
                }
                else if (!child_type_computed[next]) {
                        struct parse_tree *child;
                        load_child_at(child, tree, next);
                        child_types[next] = find_type(system, child, outer_map, map_to_use);
                }

                if (!child_types[current] || (!child_types[next] && (next >= 0))){
                        if (map_to_use != scope_map){
                                free_map(map_to_use, string, type);
                                free(map_to_use);
                        }
                }

                return param_type(child_types[current], child_types[next]);            
        }

        return NULL;
}

const struct type *const find_type(const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        if (tree->data.type == SYMBOL_IDENTIFIER){
                return find_symbol_type(tree, outer_map);
        }
        else if (tree->data.type == SYMBOL_CALL){
                return find_call_type(tree, outer_map);
        }

        for (size_t i = 0; i < system->rules_len; ++i){
                const struct type *const output = apply(system->rules[i], system, tree, outer_map, scope_map);
                if (output){
                        return output;
                }
        }

        return NULL;
}
