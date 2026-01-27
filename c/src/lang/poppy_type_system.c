#include "lang/poppy_type_system.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_system.h"

#define RULE_COUNT 8

const struct type_system *poppy_type_system = NULL;
const struct type_rule *rules[RULE_COUNT];

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

        poppy_type_system = new_type_system(rules, RULE_COUNT);
        return poppy_type_system;
}

void free_poppy_type_system();
