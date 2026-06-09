#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "data/list.h"
#include "lang/lexer.h"

DEFINE_LIST(parse_tree);

struct parse_tree {
        struct token data;
        struct LIST(parse_tree) * children;
        struct parse_tree *parent;
};

void free_parse_tree(const struct parse_tree *tree);
void print_parse_tree(const struct parse_tree *tree);

#endif
