#include "lang/parse_tree.h"

#include <stdlib.h>

#include "data/list.h"

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
