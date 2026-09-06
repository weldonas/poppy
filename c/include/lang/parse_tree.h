#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "data/list.h"
#include "data/map.h"
#include "lang/lexer.h"
#include "lang/type.h"

#define MAX_PARSE_CHILDREN 16

struct string {
        const char *data;
};

struct symbol_table_value {
        const struct type *type;
        bool is_defined;
        bool has_compile_time_value;
        int64_t compile_time_value;
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
char *parse_tree_string(const struct parse_tree *tree);

struct MAP(string, symbol_table_value) *new_symbol_table();

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);   

struct LIST(variable) get_local_variables(const struct parse_tree *tree);
struct LIST(variable) get_parameters(const struct parse_tree *tree);
struct variable find_symbol_variable(const struct parse_tree *tree);
bool evaluate_immediate(const struct parse_tree *tree, int64_t *result);

#endif
