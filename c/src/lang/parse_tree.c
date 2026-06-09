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
        }
        free(tree->children);
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
