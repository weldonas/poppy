#include "lang/poppy_type_system.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_system.h"

#define RULE_COUNT 10

const struct type_system *poppy_type_system = NULL;
const struct type_rule *rules[RULE_COUNT];

bool is_non_null_int_type(const struct type *const type){
        return type && equals_type(type, int_type());
}

const struct type_system *const get_poppy_type_system(){
        if (poppy_type_system){
                return poppy_type_system;
        }

        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t i = 0;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CHAR);
        rules[i] = new_type_rule(conditions, 1, char_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CHARLIT);
        rules[i] = new_type_rule(conditions, 1, char_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_INT);
        rules[i] = new_type_rule(conditions, 1, int_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CONSTANT);
        rules[i] = new_type_rule(conditions, 1, int_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BOOL);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TRUE);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_FALSE);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VOID);
        rules[i] = new_type_rule(conditions, 1, void_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_MINUS);
        conditions[2] = new_type_at_condition(1, is_non_null_int_type);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        // conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        // conditions[1] = new_symbol_at_condition(0, SYMBOL_LPAREN);
        // rules[i] = new_index_type_rule(conditions, 2, 1);
        // ++i;

        poppy_type_system = new_type_system(rules, RULE_COUNT);
        return poppy_type_system;
}

void free_poppy_type_system(){
        if (poppy_type_system != NULL){
                free_type_system(poppy_type_system);
                poppy_type_system = NULL;
        }
}
