#include "lang/poppy_type_system.h"
#include "data/map.h"
#include "lang/parse_tree.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_system.h"
#include <assert.h>

#define RULE_COUNT 89

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

bool is_non_null_in_memory_type(const struct type *const type){
        return type && (type->byte_count != NOT_IN_MEMORY);
}

bool is_non_null_type(const struct type *const type){
        return type;
}

bool is_non_null_void_type(const struct type *const type){
        return type && equals_type(type, void_type());
}

bool is_non_null_array_type(const struct type *const type){
        return type && type->category == CATEGORY_ARRAY;
}

bool is_non_null_returnable_type(const struct type *const type){
        return type && is_returnable(type);
}

bool is_non_null_assignable_type(const struct type *const type){
        return type && type->is_assignable;
}

bool is_non_null_pointer_type(const struct type *const type){
        return type && (type->category == CATEGORY_POINTER);
}

bool is_non_null_one_word_type(const struct type *const type){
        return type && (type->byte_count <= 8);
}

const struct parse_tree *find_defn_signature_name(const struct parse_tree *defn){
        const struct parse_tree *signature = defn->children->head->data;
        const struct parse_tree *id = signature->children->head->next->data;
        return id;
}

const struct parse_tree *find_decl_signature_name(const struct parse_tree *decl){
        const struct parse_tree *signature = decl->children->head->next->data;
        const struct parse_tree *id = signature->children->head->next->data;
        return id;
}

const struct type *deduce_from_last_child(const struct parse_tree *tree){
        const struct type *current;
        
        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                current = node->data->type;
                if (!current){
                        return NULL;
                }
        }

        return current;
}

const struct type *deduce_from_last_child_skip(const struct parse_tree *tree){
        const struct type *current;
        
        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next ? node->next->next : NULL){
                current = node->data->type;
                if (!current){
                        return NULL;
                }
        }

        return current;
}

bool is_hop(const struct parse_tree *tree){
        if (tree->data.type == SYMBOL_RET){
                return true;
        }

        if ((tree->children) && (tree->children->len >= 1)){
                return is_hop(tree->children->head->data);
        }

        return false;
}

const struct type *deduce_stmts(const struct parse_tree *tree){
        const struct type *last_type = tree->children->tail->data->type;
        if (!last_type){
                return NULL;
        }

        const struct type *current;
        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                current = node->data->type;
                if (!current){
                        return NULL;
                }

                // return NULL if this type doesn't equal the last type and doesn't equal void
                if (!equals_type(current, last_type) && (!equals_type(current, void_type()) || is_hop(node->data))){
                        return NULL;
                }
        }

        return current;
}

const struct type *deduce_params(const struct parse_tree *tree){
        struct type *params = param_type();

        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next ? node->next->next : NULL){
                if (!node->data->type){
                        return NULL;
                }
                add_param(params, node->data->type);
        }

        return params;
}

const struct type *deduce_record(const struct parse_tree *tree){
        const struct parse_tree *name_tree; load_child_at(name_tree, tree, 1);
        const struct parse_tree *field_tree; load_child_at(field_tree, tree, 3);      
        if (!field_tree->type){
                return NULL;
        }

        struct LIST(variable) fields;
        init_list((&fields));

        for (struct LIST_NODE(parse_tree) *node = field_tree->children->head; node != NULL; node = node->next ? node->next->next : NULL){
                const struct type *field_type = node->data->type;
                if (!field_type){
                        free_list((&fields), free_variable, variable);
                        return NULL;
                }
                
                char *field_name = node->data->children->head->next->data->data.value;

                struct variable *v = malloc(sizeof(struct variable));
                v->type = field_type;
                v->string = field_name;

                append_list((&fields), v, variable);
        }

        return record_type(name_tree->data.value, fields);
}

const struct type *deduce_enum(const struct parse_tree *tree){
        const struct parse_tree *parent = tree->parent->parent->parent;
        assert(parent->data.type == SYMBOL_PROGRAM);
        const struct parse_tree *name_tree; load_child_at(name_tree, tree, 1);
        const struct parse_tree *item_tree; load_child_at(item_tree, tree, 3);      
        if (!item_tree->type){
                return NULL;
        }

        // const struct type *type = make_assignable(enum_type(name_tree->data.value));
        struct LIST(string) *items = malloc(sizeof(struct LIST(string)));
        init_list(items);

        for (struct LIST_NODE(parse_tree) *node = item_tree->children->head; node != NULL; node = node->next ? node->next->next : NULL){
                char *item_name = node->data->children->head->data->data.value;
                struct string *s = malloc(sizeof(struct string));
                s->data = item_name;
                append_list(items, s, string);
        }

        const struct type *type = make_assignable(enum_type(name_tree->data.value, items));
        for (struct LIST_NODE(string) *node = items->head; node != NULL; node = node->next){
                const char *item_name = node->data->data;
                struct string *s = malloc(sizeof(struct string));
                s->data = item_name;

                struct symbol_table_value *v = malloc(sizeof(struct symbol_table_value));
                v->type = type;
                v->is_defined = true;
                update_map((parent->symbol_table), s, v, string, symbol_table_value); 
        }

        return void_type();
}

const struct type *deduce_enumitems(const struct parse_tree *tree){
        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next ? node->next->next : NULL){
                const struct type *item_type = node->data->type;
                
                if (!item_type){
                        return NULL;
                }
        }

        return void_type();
}

const struct type *deduce_enumitem(const struct parse_tree *tree){
        const struct type *child_type = tree->children->head->data->type;

        // if the type of the identifier is not null, it has been declared already
        if (child_type != NULL){
                return NULL;
        }

        return void_type();
}

const struct type *deduce_record_type(const struct parse_tree *tree){
        const struct parse_tree *record = tree->children->head->data;
        char *record_name = record->data.value;
        return query_named_type(record_name);
}

const struct type *deduce_addressable_dot(const struct parse_tree *tree){
        const struct parse_tree *record_tree; load_child_at(record_tree, tree, 0);
        const struct parse_tree *field_tree; load_child_at(field_tree, tree, 2);

        if (!record_tree->type || (record_tree->type->category != CATEGORY_RECORD)){
                return NULL;
        }
 
        return make_assignable(field_type(record_tree->type, field_tree->data.value));
}

const struct type *deduce_addressable_dotdot(const struct parse_tree *tree){
        const struct parse_tree *pointer_tree; load_child_at(pointer_tree, tree, 0);
        const struct parse_tree *field_tree; load_child_at(field_tree, tree, 2);

        if (!pointer_tree->type || (pointer_tree->type->category != CATEGORY_POINTER)){
                return NULL;
        }

        const struct type *record_type = pointer_tree->type->referenced_type;

        if (record_type->category != CATEGORY_RECORD){
                return NULL;
        }
 
        return make_assignable(field_type(record_type, field_tree->data.value));
}

const struct type *deduce_addressable_index(const struct parse_tree *tree){
        const struct parse_tree *array_tree; load_child_at(array_tree, tree, 0);

        if (!array_tree->type){
                return NULL;
        }

        return array_tree->type->element_type;
}

const struct type *deduce_array(const struct parse_tree *tree){
        const struct parse_tree *element_tree; load_child_at(element_tree, tree, 0);
        const struct parse_tree *length_tree; load_child_at(length_tree, tree, 2);

        if (!element_tree->type){
                return NULL;
        }

        return array_type(element_tree->type, length_tree->data.value);
}

const struct type *deduce_function(const struct parse_tree *tree){
        const struct parse_tree *ret_tree; load_child_at(ret_tree, tree, 0);
        const struct parse_tree *param_tree; load_child_at(param_tree, tree, 3);

        if (!ret_tree->type || !param_tree->type){
                return NULL;
        }

        return function_type(ret_tree->type, param_tree->type);

}

const struct type *deduce_call(const struct parse_tree *tree){
        const struct parse_tree *optargs; load_child_at(optargs, tree, 2);
        const struct parse_tree *fn; load_child_at(fn, tree, 0);

        if (fn->type->category != CATEGORY_FUNCTION){
                return NULL;
        }

        if (!equals_type(optargs->type, fn->type->params_type)){
                return NULL;
        }

        return return_type(fn->type);
}

const struct type *deduce_symbol_type(const struct parse_tree *tree){
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

const struct type *deduce_cast(const struct parse_tree *tree){
        if (tree->children->head->data->data.type == SYMBOL_UNSAFE){
                const struct parse_tree *dst_tree; load_child_at(dst_tree, tree, 1);
                return dst_tree->type;
        }

        const struct parse_tree *dst_tree; load_child_at(dst_tree, tree, 1);
        const struct parse_tree *src_tree; load_child_at(src_tree, tree, 3);

        if (can_safe_cast(src_tree->type, dst_tree->type)){
                return dst_tree->type;
        }

        return NULL;
}

const struct type *deduce_reference(const struct parse_tree *tree){
        const struct parse_tree *type_tree; load_child_at(type_tree, tree, 1);
        return pointer_type(type_tree->type);
}

const struct type *deduce_double_reference(const struct parse_tree *tree){
        const struct parse_tree *type_tree; load_child_at(type_tree, tree, 1);
        return pointer_type(pointer_type(type_tree->type));
}

const struct type *deduce_dereference(const struct parse_tree *tree){
        const struct parse_tree *type_tree; load_child_at(type_tree, tree, 1);
        return type_tree->type->referenced_type;
}

const struct type *deduce_vardec(const struct parse_tree *tree){
        // const struct parse_tree *type_tree; load_child_at(type_tree, tree, 1);
        return void_type();
}

const struct type_system *const get_poppy_type_system(){
        if (poppy_type_system){
                return poppy_type_system;
        }

        const struct type_rule_condition *conditions[MAX_CONDITION_COUNT];
        size_t i = 0;

        // Program
        conditions[0] = new_parent_symbol_condition(SYMBOL_PROGRAM);
        rules[i] = new_child_type_rule(conditions, 1, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_DEFNDECLS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_from_last_child);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_DEFNDECL);
        conditions[1] = new_type_at_condition(0, is_non_null_type);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        // Functions
        conditions[0] = new_parent_symbol_condition(SYMBOL_DECL);
        conditions[1] = new_add_symbol_name_function_side_effect(find_decl_signature_name, 1, false);
        conditions[2] = new_add_scope_side_effect(); // this ensures params dont get added to the global scope
        rules[i] = new_type_rule(conditions, 3, unit_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_FNDEFN);
        conditions[1] = new_add_symbol_name_function_side_effect(find_defn_signature_name, 0, true);
        conditions[2] = new_add_scope_side_effect();
        conditions[3] = new_return_type_at_condition(2, 0);
        rules[i] = new_child_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SIGNATURE);
        conditions[1] = new_type_at_condition(0, is_non_null_returnable_type);
        conditions[2] = new_type_at_condition(3, is_non_null_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_function);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CALL);
        conditions[1] = new_type_at_condition(0, is_non_null_type);
        conditions[2] = new_type_at_condition(2, is_non_null_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_call);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTPARAMS);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, unit_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_PARAMS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_params);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_PARAM);
        conditions[1] = new_add_symbol_name_index_side_effect(1, 0, true);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTARGS);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, unit_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ARGS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_params);
        ++i;

        // Variables
        // we don't add a condition for LHS being assignable since this should always be the case
        // and the variable is not added until after this type rule completes execution
        conditions[0] = new_parent_symbol_condition(SYMBOL_VARDEC);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(1, is_non_null_in_memory_type);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1, true);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARDEC);
        conditions[1] = new_length_condition(5);
        conditions[2] = new_types_equal_at_condition(1, 4);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1, true);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_GLOBALVARDEC);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(1, is_non_null_in_memory_type);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1, true);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_GLOBALVARDEC);
        conditions[1] = new_length_condition(5);
        conditions[2] = new_types_equal_at_condition(1, 4);
        conditions[3] = new_add_symbol_name_index_side_effect(2, 1, true);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_types_equal_at_condition(0, 2);
        conditions[3] = new_type_at_condition(0, is_non_null_assignable_type);
        rules[i] = new_type_rule(conditions, 4, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_IDENTIFIER);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_symbol_type);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_VARDEC);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_VARASST);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        // Non-primitive data types
        conditions[0] = new_parent_symbol_condition(SYMBOL_MEMBEREXPR);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_type_at_condition(0, is_non_null_array_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_deducer_type_rule(conditions, 4, deduce_addressable_index);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_AMP);
        conditions[2] = new_type_at_condition(1, is_non_null_assignable_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_reference);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_STAR);
        conditions[2] = new_type_at_condition(1, is_non_null_pointer_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_dereference);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_MEMBEREXPR);
        conditions[1] = new_symbol_at_condition(1, SYMBOL_DOT);
        rules[i] = new_deducer_type_rule(conditions, 2, deduce_addressable_dot);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_MEMBEREXPR);
        conditions[1] = new_symbol_at_condition(1, SYMBOL_DOTDOT);
        rules[i] = new_deducer_type_rule(conditions, 2, deduce_addressable_dotdot);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CAST);
        conditions[1] = new_length_condition(5);
        conditions[2] = new_type_at_condition(1, is_non_null_type);
        conditions[3] = new_type_at_condition(3, is_non_null_type);
        rules[i] = new_deducer_type_rule(conditions, 4, deduce_cast);
        ++i;

        // Statements
        conditions[0] = new_parent_symbol_condition(SYMBOL_STMTS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_stmts);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BODY);
        conditions[1] = new_add_scope_side_effect();
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_STMT);
        rules[i] = new_child_type_rule(conditions, 1, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_EXPR);
        conditions[2] = new_type_at_condition(0, is_non_null_type);
        rules[i] = new_type_rule(conditions, 3, void_type());
        ++i;

        // return statements
        conditions[0] = new_parent_symbol_condition(SYMBOL_SEMISTMT);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_RET);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_RET);
        conditions[1] = new_length_condition(1);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_RET);
        conditions[1] = new_length_condition(2);
        rules[i] = new_child_type_rule(conditions, 2, 1);
        ++i;

        // if statements
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
        rules[i] = new_child_type_rule(conditions, 3, 5);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTELSE);
        conditions[1] = new_length_condition(0);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_OPTELSE);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_add_scope_side_effect();
        rules[i] = new_child_type_rule(conditions, 3, 2);
        ++i;

        // loops
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

        // inline assembly
        conditions[0] = new_parent_symbol_condition(SYMBOL_BASEEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_ASM);
        rules[i] = new_type_rule(conditions, 2, void_type());
        ++i;

        // Predicates
        conditions[0] = new_parent_symbol_condition(SYMBOL_OREXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_child_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ANDEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_bool_type);
        conditions[3] = new_type_at_condition(2, is_non_null_bool_type);
        rules[i] = new_child_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_NOT);
        conditions[3] = new_type_at_condition(1, is_non_null_bool_type);
        rules[i] = new_child_type_rule(conditions, 4, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BASEEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_LPAREN);
        // conditions[3] = new_type_at_condition(1, is_non_null_bool_type);
        rules[i] = new_child_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_EQEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_one_word_type);
        conditions[3] = new_types_equal_at_condition(0, 2);
        rules[i] = new_type_rule(conditions, 4, bool_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_COMPEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_or_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 4, bool_type());
        ++i;

        // Arithmetic
        conditions[0] = new_parent_symbol_condition(SYMBOL_EXPR);
        conditions[1] = new_length_condition(1);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ADDEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_or_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_or_char_type);
        rules[i] = new_child_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_MULTEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_or_char_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_or_char_type);
        rules[i] = new_child_type_rule(conditions, 4, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_INC);
        conditions[3] = new_type_at_condition(1, is_non_null_int_or_char_type);
        conditions[4] = new_type_at_condition(1, is_non_null_assignable_type);
        rules[i] = new_child_type_rule(conditions, 5, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_DEC);
        conditions[3] = new_type_at_condition(1, is_non_null_int_or_char_type);
        conditions[4] = new_type_at_condition(1, is_non_null_assignable_type);
        rules[i] = new_child_type_rule(conditions, 5, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_MINUS);
        conditions[2] = new_type_at_condition(1, is_non_null_int_type);
        rules[i] = new_child_type_rule(conditions, 3, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BANDEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, int_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BOREXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, int_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BXOREXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, int_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BSHIFTEXPR);
        conditions[1] = new_length_condition(3);
        conditions[2] = new_type_at_condition(0, is_non_null_int_type);
        conditions[3] = new_type_at_condition(2, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, int_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_UNARYEXPR);
        conditions[1] = new_length_condition(2);
        conditions[2] = new_symbol_at_condition(0, SYMBOL_BNOT);
        conditions[3] = new_type_at_condition(1, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 4, int_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_PLUS);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_MINUS);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_STAR);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_DIVIDE);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_MOD);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_or_char_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_AMP);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_BXOR);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_BOR);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_BLEFT);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VARASST);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_symbol_at_condition(1, SYMBOL_BRIGHT);
        conditions[3] = new_types_equal_at_condition(0, 3);
        conditions[4] = new_type_at_condition(0, is_non_null_assignable_type);
        conditions[5] = new_type_at_condition(0, is_non_null_int_type);
        rules[i] = new_type_rule(conditions, 6, void_type());
        ++i;

        // Literals
        conditions[0] = new_parent_symbol_condition(SYMBOL_CONSTANT);
        rules[i] = new_type_rule(conditions, 1, int_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_CHARLIT);
        rules[i] = new_type_rule(conditions, 1, char_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_STRINGLIT);
        rules[i] = new_type_rule(conditions, 1, string_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TRUE);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_FALSE);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        // Type parsing
        conditions[0] = new_parent_symbol_condition(SYMBOL_CHAR);
        rules[i] = new_type_rule(conditions, 1, char_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_INT);
        rules[i] = new_type_rule(conditions, 1, int_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_BOOL);
        rules[i] = new_type_rule(conditions, 1, bool_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_VOID);
        rules[i] = new_type_rule(conditions, 1, void_type()); 
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_length_condition(1);
        rules[i] = new_child_type_rule(conditions, 2, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_LPAREN);
        rules[i] = new_child_type_rule(conditions, 2, 1);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_AMP);
        conditions[2] = new_type_at_condition(1, is_non_null_in_memory_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_reference);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_AND);
        conditions[2] = new_type_at_condition(1, is_non_null_in_memory_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_double_reference);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_length_condition(4);
        conditions[2] = new_type_at_condition(0, is_non_null_in_memory_type);
        rules[i] = new_deducer_type_rule(conditions, 3, deduce_array);
        ++i;
        
        conditions[0] = new_parent_symbol_condition(SYMBOL_TYPE);
        conditions[1] = new_symbol_at_condition(0, SYMBOL_IDENTIFIER);
        rules[i] = new_deducer_type_rule(conditions, 2, deduce_record_type);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_RECDEFN);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_record);
        ++i;       

        conditions[0] = new_parent_symbol_condition(SYMBOL_FIELDS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_from_last_child_skip);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_FIELD);
        rules[i] = new_child_type_rule(conditions, 1, 0);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ENUMDEFN);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_enum);
        ++i;       

        conditions[0] = new_parent_symbol_condition(SYMBOL_ENUMITEMS);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_enumitems);
        ++i;

        conditions[0] = new_parent_symbol_condition(SYMBOL_ENUMITEM);
        rules[i] = new_deducer_type_rule(conditions, 1, deduce_enumitem);
        ++i;

        assert(i == RULE_COUNT);

        poppy_type_system = new_type_system(rules, RULE_COUNT);
        return poppy_type_system;
}

void free_poppy_type_system(){
        if (poppy_type_system != NULL){
                free_type_system(poppy_type_system);
                poppy_type_system = NULL;
        }
}
