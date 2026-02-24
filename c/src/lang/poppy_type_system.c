#include "lang/poppy_type_system.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_system.h"

#define RULE_COUNT 62

const struct type_system *poppy_type_system = NULL;
const struct type_rule *rules[RULE_COUNT];

bool is_non_null_int_type(const struct type *const type){
        return type && equals_type(type, int_type());
}

bool is_non_null_char_type(const struct type *const type){
        return type && equals_type(type, char_type());
}

bool is_non_null_int_or_char_type(const struct type *const type){
        return type && (equals_type(type, int_type()) || equals_type(type, char_type()));
}

bool is_non_null_bool_type(const struct type *const type){
        return type && equals_type(type, bool_type());
}

bool is_non_null_assignable_type(const struct type *const type){
        return type && is_assignable(type);
}

bool is_non_null_type(const struct type *const type){
        return type;
}

bool is_non_null_void_type(const struct type *const type){
        return type && equals_type(type, void_type());
}

char *find_signature_name(const struct parse_tree *defn){
        const struct parse_tree *signature = defn->children->head->data;
        const struct parse_tree *id = signature->children->head->next->data;
        return id->data.value;
}

const struct type_system *const get_poppy_type_system(){
        if (poppy_type_system){
                return poppy_type_system;
        }

        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t i = 0;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

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
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_INC);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_DEC);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_MINUS);
        conditions[2] = new_type_at_condition(1, is_non_null_int_type);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_LPAREN);
        rules[i] = new_index_type_rule(conditions, 2, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_EXPR);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ADDEXPR);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_MULTEXPR);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ADDEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_or_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_or_char_type);
        rules[i] = new_index_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_MULTEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_or_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_or_char_type);
        rules[i] = new_index_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ORCOND);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ANDCOND);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ORCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_index_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ANDCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_index_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_NOT);
        conditions[3] = new_type_at_condition(1, is_non_null_bool_type);
        rules[i] = new_index_type_rule(conditions, 4, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_LPAREN);
        // conditions[3] = new_type_at_condition(1, is_non_null_bool_type);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_symbol_at_condition(1, SYMBOL_EQ);
        conditions[4] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_type_rule(conditions, 5, bool_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_symbol_at_condition(1, SYMBOL_NE);
        conditions[4] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_type_rule(conditions, 5, bool_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, bool_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNCOND);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_char_type);
        rules[i] = new_type_rule(conditions, 4, bool_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_RET);
        conditions[1] = new_length_condition(1);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_RET);
        conditions[1] = new_length_condition(2);
        rules[i] = new_index_type_rule(conditions, 2, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_types_equal_at_condition(0, 2);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARDEC);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(1, is_non_null_assignable_type);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARDEC);
        conditions[1] = new_length_condition(5);
        conditions[2] = new_types_equal_at_condition(1, 4);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_ASM);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_VARDEC);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_VARASST);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_RET);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_EXPR);
        conditions[2] = new_type_at_condition(0, is_non_null_type);
        rules[i] = new_type_rule(conditions, 3, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BODY);
        conditions[1] = new_add_scope_side_effect();
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTELSE);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTELSE);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_add_scope_side_effect();
        rules[i] = new_index_type_rule(conditions, 3, 2);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_IFSTMT);
        conditions[1] = new_type_at_condition(2, is_non_null_bool_type);
        conditions[2] = new_type_at_condition(5, is_non_null_type);
        conditions[3] = new_type_at_condition(7, is_non_null_void_type);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_IFSTMT);
        conditions[1] = new_type_at_condition(2, is_non_null_bool_type);
        conditions[2] = new_type_at_condition(5, is_non_null_void_type);
        conditions[3] = new_type_at_condition(7, is_non_null_type);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_IFSTMT);
        conditions[1] = new_type_at_condition(2, is_non_null_bool_type);
        conditions[2] = new_types_equal_at_condition(5, 7);
        rules[i] = new_index_type_rule(conditions, 3, 5);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_WHILESTMT);
        conditions[1] = new_type_at_condition(2, is_non_null_bool_type);
        conditions[2] = new_type_at_condition(5, is_non_null_type);
        rules[i] = new_type_rule(conditions, 3, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_FORSTMT);
        conditions[1] = new_type_at_condition(2, is_non_null_void_type);
        conditions[2] = new_type_at_condition(4, is_non_null_bool_type);
        conditions[3] = new_type_at_condition(6, is_non_null_void_type);
        conditions[4] = new_type_at_condition(9, is_non_null_type);
        conditions[5] = new_add_scope_side_effect();
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_STMT);
        rules[i] = new_index_type_rule(conditions, 1, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_STMTS);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_STMTS);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_type_at_condition(0, is_non_null_type);
        rules[i] = new_index_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTPARAMS);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, NULL);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTPARAMS);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_PARAMS);
        conditions[1] = new_length_condition(1);
        rules[i] = new_param_type_rule(conditions, 2, 0, -1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_PARAMS);
        conditions[1] = new_length_condition(3);
        rules[i] = new_param_type_rule(conditions, 2, 0, 2);
        ++i; 

        conditions[0] = new_parent_symbol_condition(SYMBOL_PARAM);
        conditions[1] = new_add_symbol_name_index_side_effect(1, 0);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTARGS);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, NULL);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTARGS);
        conditions[1] = new_length_condition(1);
        rules[i] = new_index_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ARGS);
        conditions[1] = new_length_condition(1);
        rules[i] = new_param_type_rule(conditions, 2, 0, -1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ARGS);
        conditions[1] = new_length_condition(3);
        rules[i] = new_param_type_rule(conditions, 2, 0, 2);
        ++i; 

        conditions[0] = new_parent_symbol_condition(SYMBOL_SIGNATURE);
        rules[i] = new_function_type_rule(conditions, 1, 0, 3);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_DEFN);
        conditions[1] = new_add_symbol_name_function_side_effect(find_signature_name, 0);
        conditions[2] = new_add_scope_side_effect();
        rules[i] = new_index_type_rule(conditions, 3, 0);
        ++i;

        poppy_type_system = new_type_system(rules, RULE_COUNT);
        return poppy_type_system;
}

void free_poppy_type_system(){
        if (poppy_type_system != NULL){
                free_type_system(poppy_type_system);
                poppy_type_system = NULL;
        }
}
