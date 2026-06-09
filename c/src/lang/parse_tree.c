#include "lang/parse_tree.h"

#include <stdlib.h>
#include <string.h>

#include "data/list.h"

void free_string(struct string *s){}

bool equals_string(const struct string *s1, const struct string *s2) {
        return strcmp(s1->data, s2->data) == 0;
}

void free_string_entry(const struct MAP_ENTRY(string, symbol_table_value) *entry){
        free((void *) entry->key);
        free((void*) entry->value);
        free((void *) entry);
}

struct MAP(string, symbol_table_value) *new_symbol_table() {
        struct MAP(string, symbol_table_value) *ptr = (struct MAP(string, symbol_table_value)*) malloc(sizeof(struct MAP(string, symbol_table_value)));
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

void print_parse_tree_rec(const struct parse_tree *tree){
        if (is_terminal(tree->data.type)){
                printf("%s ", tree->data.value);
        }

        if (tree->children){
                for(struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        print_parse_tree_rec(node->data);
                }
        }

}

void print_parse_tree(const struct parse_tree *tree){
        print_parse_tree_rec(tree);
        printf("\n");
}

void free_variable(struct variable *v){
        free(v);
}

void get_variables_recursive(const struct parse_tree *tree, struct LIST(variable) *list){
        const struct MAP(string, symbol_table_value) *symbol_table = tree->symbol_table; 

        if (symbol_table != NULL){
                for (struct string_symbol_table_value_map_entry_list_node *map_node = symbol_table->list->head; map_node != NULL; map_node = map_node->next){
                        struct variable *v = (struct variable*) malloc(sizeof(struct variable));
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
                struct variable *v = (struct variable*) malloc(sizeof(struct variable));
                v->string = map_node->data->key->data;
                v->type = map_node->data->value->type;

                append_list((&list), v, variable);
        }
        return list;
}
