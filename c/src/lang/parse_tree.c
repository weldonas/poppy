#include "lang/parse_tree.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "data/list.h"

void free_string(struct string *s){
        free(s);
}

bool equals_string(const struct string *s1, const struct string *s2) {
        return strcmp(s1->data, s2->data) == 0;
}

void free_string_entry(const struct MAP_ENTRY(string, symbol_table_value) *entry){
        free((void *) entry->key);
        free((void*) entry->value);
        free((void *) entry);
}

struct MAP(string, symbol_table_value) *new_symbol_table() {
        struct MAP(string, symbol_table_value) *ptr = malloc(sizeof(struct MAP(string, symbol_table_value)));
        init_map(ptr, equals_string, free_string_entry, string, symbol_table_value);
        return ptr;
}

void free_parse_tree(const struct parse_tree *tree){
        if (tree->children != NULL) {
                free_list(tree->children, free_parse_tree, parse_tree);
                free(tree->children);
        }
        
        if (tree->symbol_table != NULL){
                free_map(tree->symbol_table, string, symbol_table_value);
                free(tree->symbol_table);
        }

        free((void *) tree);
}

void print_parse_tree_rec(const struct parse_tree *tree, size_t depth) {
    for (size_t i = 0; i < depth; ++i) {
        printf("  ");
    }

    printf("%s", symbol_name(tree->data.type));

    if (is_terminal(tree->data.type) && tree->data.value) {
        printf(" %s", tree->data.value);
    }

    printf("\n");

    if (tree->children) {
        for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next) {
            print_parse_tree_rec(node->data, depth + 1);
        }
    }
}

void print_parse_tree(const struct parse_tree *tree){
        print_parse_tree_rec(tree, 0);
        printf("\n");
}

void buf_append(char **buf, size_t *len, size_t *cap, const char *str) {
        size_t slen = strlen(str);

        if (*len + slen + 1 > *cap) {
                while (*len + slen + 1 > *cap) {
                        *cap *= 2;
                }
                *buf = realloc(*buf, *cap);
        }

        memcpy(*buf + *len, str, slen);
        *len += slen;
        (*buf)[*len] = 0;
}


void tree_concat(const struct parse_tree *tree, char **buf, size_t *len, size_t *cap) {
        if (tree->children) {
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next) {
                        tree_concat(node->data, buf, len, cap);
                }
        } 
        else if (is_terminal(tree->data.type)) {
                assert(tree->data.value);
                buf_append(buf, len, cap, tree->data.value);
        }
}

char *parse_tree_string(const struct parse_tree *tree) {
        size_t cap = 16;
        size_t len = 0;

        char *buf = malloc(cap);
        buf[0] = 0;
        
        tree_concat(tree, &buf, &len, &cap);
        return buf;
}

struct variable find_symbol_variable(const struct parse_tree *tree){
        struct variable var = {tree->data.value, tree->type};
        return var;
}

void get_variables_recursive(const struct parse_tree *tree, struct LIST(variable) *list){
        const struct MAP(string, symbol_table_value) *symbol_table = tree->symbol_table; 

        if (symbol_table != NULL){
                for (struct string_symbol_table_value_map_entry_list_node *map_node = symbol_table->list->head; map_node != NULL; map_node = map_node->next){
                        struct variable *v = malloc(sizeof(struct variable));
                        v->string = map_node->data->key->data;
                        v->type = map_node->data->value->type;
                        append_list(list, v, variable);
                }
        }

        if (tree->children != NULL){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        get_variables_recursive(node->data, list);
                }
        }
}

struct LIST(variable) get_local_variables(const struct parse_tree *tree){
        // defn -> signature LBRACE stmts RBRACE
        const struct parse_tree *stmts; load_child_at(stmts, tree, 2);
        struct LIST(variable) list;
        init_list((&list))
        get_variables_recursive(stmts, &list);
        return list;
}

struct LIST(variable) get_parameters(const struct parse_tree *tree){
        // defn -> signature LBRACE stmts RBRACE
        // signature -> type IDENTIFIER LPAREN optparams RPAREN
        const struct MAP(string, symbol_table_value) *symbol_table = tree->symbol_table;
        struct LIST(variable) list;
        init_list((&list))
        for (struct string_symbol_table_value_map_entry_list_node *map_node = symbol_table->list->head; map_node != NULL; map_node = map_node->next){
                struct variable *v = malloc(sizeof(struct variable));
                v->string = map_node->data->key->data;
                v->type = map_node->data->value->type;

                append_list((&list), v, variable);
        }
        return list;
}

const struct symbol_table_value *query_symbol_table(const struct parse_tree *tree){
        struct string string;
        string.data = tree->data.value;
        const struct parse_tree *cur = tree;
        
        while (cur != NULL){
                const struct MAP(string, symbol_table_value) *symbol_table = cur->symbol_table; 
                if (symbol_table != NULL){
                        const struct symbol_table_value *value; query_map(symbol_table, &string, value, string, symbol_table_value);

                        if (value != NULL){
                                return value;
                        }
                }
                cur = cur->parent;
        }
        return NULL;
}

bool evaluate_immediate(const struct parse_tree *tree, int64_t *result){
        // impossible if type is not int, char, or bool
        if (!equals_type(tree->type, int_type()) && !equals_type(tree->type, char_type()) && !equals_type(tree->type, bool_type())){
                return false;
        }

        if (tree->data.type == SYMBOL_IDENTIFIER){
                const struct symbol_table_value *value = query_symbol_table(tree);
                if (value->has_compile_time_value){
                        *result = value->compile_time_value;
                        return true;
                }
                return false;
        }

        if (tree->data.type == SYMBOL_CALL){
                return false;
        }

        if (tree->data.type == SYMBOL_CONSTANT){
                *result = strtoll(tree->data.value, NULL, 10);
                return true;
        }

        if (tree->data.type == SYMBOL_CHARLIT){
                *result = tree->data.value[0];
                return true;
        }

        if (tree->data.type == SYMBOL_TRUE){
                *result = true;
                return true;
        }

        if (tree->data.type == SYMBOL_FALSE){
                *result = false;
                return true;
        }

        if (tree->children->len == 1){
                return evaluate_immediate(tree->children->head->data, result);
        }

        if (tree->children->len == 2){
                return false;
        }

        switch (tree->children->head->data->data.type){
                case SYMBOL_LPAREN:
                        return evaluate_immediate(tree->children->head->next->data, result);
                case SYMBOL_UNSAFE:
                case SYMBOL_SAFE:
                        const struct parse_tree *expr_tree; load_child_at(expr_tree, tree, 3);
                        return evaluate_immediate(expr_tree, result);
                case SYMBOL_ASM:
                        return false;
                default:
                        break;
        }

        if (tree->children->head->data->data.type == SYMBOL_LPAREN){
                return evaluate_immediate(tree->children->head->next->data, result);
        }


        const struct parse_tree *op1_tree; load_child_at(op1_tree, tree, 0);
        const struct parse_tree *operand_tree; load_child_at(operand_tree, tree, 1);
        const struct parse_tree *op2_tree; load_child_at(op2_tree, tree, 2);
        int64_t op1;
        int64_t op2;

        if (!evaluate_immediate(op1_tree, &op1) || !evaluate_immediate(op2_tree, &op2)){
                return false;
        }

        switch(operand_tree->data.type){
                case SYMBOL_OR:
                        *result = op1 || op2; 
                        return true;
                case SYMBOL_AND:
                        *result = op1 && op2;
                        return true;
                case SYMBOL_EQ:
                        *result = op1 == op2;
                        return true;
                case SYMBOL_NE:
                        *result = op1 != op2;
                        return true;
                case SYMBOL_LT:
                        *result = op1 < op2;
                        return true;
                case SYMBOL_LE:
                        *result = op1 <= op2;
                        return true;
                case SYMBOL_GT:
                        *result = op1 > op2;
                        return true;
                case SYMBOL_GE:
                        *result = op1 >= op2;
                        return true;
                case SYMBOL_BOR:
                        *result = op1 | op2;
                        return true;
                case SYMBOL_BXOR:
                        *result = op1 ^ op2;
                        return true;
                case SYMBOL_AMP:
                        *result = op1 & op2;
                        return true;
                case SYMBOL_BLEFT:
                        *result = op1 << op2;
                        return true;
                case SYMBOL_BRIGHT:
                        *result = op1 >> op2;
                        return true;
                case SYMBOL_PLUS:
                        *result = op1 + op2;
                        return true;
                case SYMBOL_MINUS:
                        *result = op1 - op2;
                        return true;
                case SYMBOL_STAR:
                        *result = op1 * op2;
                        return true;
                case SYMBOL_DIVIDE:
                        *result = op1 / op2;
                        return true;
                case SYMBOL_MOD:
                        *result = op1 % op2;
                        return true;
                default:
                        return false;
        }
}
