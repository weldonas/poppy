#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "data/list.h"
#include "data/map.h"
#include "lang/lexer.h"
#include "lang/type.h"

struct string {
        const char *data;
};

struct symbol_table_value {
        const struct type *type;
        bool is_defined;
};

DEFINE_MAP(string, symbol_table_value);

void free_string(struct string *s);
bool equals_string(const struct string *s1, const struct string *s2);

DEFINE_LIST(parse_tree);

struct parse_tree {
        struct token data;
        struct LIST(parse_tree) * children;
        struct parse_tree *parent;
        const struct type *type;
        struct MAP(string, symbol_table_value) *symbol_table;
};

void free_parse_tree(const struct parse_tree *tree);
void print_parse_tree(const struct parse_tree *tree);
struct MAP(string, symbol_table_value) *new_symbol_table();

#endif
