#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "data/map.h"
#include "lang/parser.h"

#define OUTER_TYPE_MAP parse_tree_string_type_map_map

struct string {
        char *data;
};
bool equals_string(const struct string *s1, const struct string *s2);

DEFINE_LIST(string)

DEFINE_MAP(string, type);
DEFINE_MAP(parse_tree, MAP(string, type));

struct OUTER_TYPE_MAP *find_types(const struct parse_tree *tree);
const struct type *find_symbol_type(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *outer_map);
struct LIST(string) get_local_variables(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols);
struct LIST(string) get_parameters(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols);
#endif
