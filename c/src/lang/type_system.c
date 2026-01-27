#include "lang/type_system.h"
#include <stddef.h>

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);   

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
        ptr->type = CONDITION_SYMBOL_AT;
        ptr->index = index;
        ptr->is_valid = is_valid;
        return ptr; 
}

const struct type_rule *const new_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, const struct type *const output_type){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->has_output_type = true;
        ptr->output_type = output_type;
        return ptr;
}

const struct type_rule *const new_index_type_rule(const struct type_rule_condition *conditions[MAX_CONDITION_COUNT], size_t conditions_len, size_t output_index){
        struct type_rule *ptr = (struct type_rule*) malloc(sizeof(struct type_rule));
        for (size_t i = 0; i < conditions_len; ++i){
                ptr->conditions[i] = conditions[i];
        }
        ptr->conditions_len = conditions_len;
        ptr->has_output_type = false;
        ptr->output_index = output_index;
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

bool applies_condition(const struct type_rule_condition *const condition, const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        struct parse_tree *child;
        switch(condition->type){
                case CONDITION_LENGTH:
                        return tree->children->len == condition->length;
                case CONDITION_PARENT_SYMBOL:
                        return tree->data.type == condition->parent_symbol;
                case CONDITION_SYMBOL_AT:
                        load_child_at(child, tree, condition->index);
                        return child->data.type == condition->symbol;
                case CONDITION_TYPE_AT:
                        // TODO fill this in
                        load_child_at(child, tree, condition->index);
                        const struct type *const child_type = find_type(system, child, outer_map, scope_map);
                        return condition->is_valid(child_type);
        }

        return false;
}

const struct type *const apply(const struct type_rule *const type_rule, const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        for (size_t i = 0; i < type_rule->conditions_len; ++i){
                const struct type_rule_condition *condition = type_rule->conditions[i];
                if (!applies_condition(condition, system, tree, outer_map, scope_map)){
                        return NULL;
                }
        }

        if (type_rule->has_output_type){
                return type_rule->output_type;
        }

        struct parse_tree *child; load_child_at(child, tree, type_rule->output_index);
        // FIXME: this may result in evaluating the same tree twice
        return find_type(system, child, outer_map, scope_map);
}

const struct type *const find_type(const struct type_system *const system, struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        for (size_t i = 0; i < system->rules_len; ++i){
                const struct type *const output = apply(system->rules[i], system, tree, outer_map, scope_map);
                if (output){
                        return output;
                }
        }

        return NULL;
}
