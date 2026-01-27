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
        ptr->output_type = output_type;
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

bool applies_condition(const struct type_rule_condition *const condition, struct parse_tree *tree){
        switch(condition->type){
                case CONDITION_LENGTH:
                        return tree->children->len == condition->length;
                case CONDITION_PARENT_SYMBOL:
                        return tree->data.type == condition->parent_symbol;
                case CONDITION_SYMBOL_AT:
                        struct parse_tree *child; load_child_at(child, tree, condition->index);
                        return child->data.type == condition->symbol;
                case CONDITION_TYPE_AT:
                        // TODO fill this in
                        return false;
        }

        return false;
}

const struct type *const apply(const struct type_rule *const type_rule, struct parse_tree *tree){
        for (size_t i = 0; i < type_rule->conditions_len; ++i){
                const struct type_rule_condition *condition = type_rule->conditions[i];
                if (!applies_condition(condition, tree)){
                        return NULL;
                }
        }
        return type_rule->output_type;
}

const struct type *const find_type(const struct type_system *const system, struct parse_tree *tree){
        for (size_t i = 0; i < system->rules_len; ++i){
                const struct type *const output = apply(system->rules[i], tree);
                if (output){
                        return output;
                }
        }

        return NULL;
}
