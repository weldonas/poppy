#include "lang/type_checker.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data/map.h"
#include "lang/poppy_type_system.h"
#include "lang/symbol.h"
#include "lang/type.h"
#include "lang/type_system.h"

#define OUTER_TYPE_MAP_ENTRY parse_tree_string_type_map_map_entry

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);                                                       \

#define verify_type(tree, st) assert(tree->data.type == st);

bool equals_string(const struct string *s1, const struct string *s2) {
        return strcmp(s1->data, s2->data) == 0;
}

void free_string_entry(const struct MAP_ENTRY(string, type) *entry){
        free((void *) entry->key);
        free((void *) entry);
}

bool equals_parse_tree(const struct parse_tree *pt1, const struct parse_tree *pt2){
        return pt1 == pt2;
}

void free_typer_entry(const struct OUTER_TYPE_MAP_ENTRY *entry){
        free_map(entry->value, string, type);
        free((void *) entry->value);
        free((void *) entry);
}

struct MAP(string, type) * new_inner_map() {
        struct MAP(string, type) *ptr = (struct MAP(string, type)*) malloc(sizeof(struct MAP(string, type)));
        init_map(ptr, equals_string, free_string_entry, string, type);
        return ptr;
}

const struct type *find_parse_tree_type(struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map);

const struct type * find_symbol_type(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *outer_map){
        struct string string;
        string.data = tree->data.value;
        const struct parse_tree *cur = tree;
        while (cur != NULL){
                const struct MAP(string, type) *inner_map; query_map(outer_map, cur, inner_map, parse_tree, MAP(string, type));
                if (inner_map != NULL){
                        const struct type *type; query_map(inner_map, &string, type, string, type);

                        if (type != NULL){
                                return type;
                        }
                }
                cur = cur->parent;
        }
        return NULL;
}

char * find_defn_name(struct parse_tree *tree){
        struct parse_tree *identifier; load_child_at(identifier, tree, 1);
        return identifier->data.value;
}

const struct type * find_defn_type(struct parse_tree *tree, struct MAP(string, type) *scope_map){
        verify_type(tree, SYMBOL_DEFN);
        // defn -> type IDENTIFIER LPAREN optparams RPAREN LBRACE stmts RBRACE
        const struct type * ret = find_parse_tree_type(tree->children->head->data, NULL, NULL);

        struct parse_tree *optparams; load_child_at(optparams, tree, 3);
        if ((optparams->children == NULL) || (optparams->children->len == 0)){
                // optparams ->
                return function_type(ret, NULL);
        }

        // optparams -> params
        struct parse_tree *params = optparams->children->head->data;
        const struct type *params_type = find_parse_tree_type(params, NULL, scope_map);
        return function_type(ret, params_type);
}

const struct type * find_call_type(struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map){
        verify_type(tree, SYMBOL_CALL);
        // call -> IDENTIFIER LPAREN optargs RPAREN
        struct parse_tree *optargs; load_child_at(optargs, tree, 2);
        
        const struct type *args_type = find_parse_tree_type(optargs, outer_map, NULL);
        const struct type *ftype = find_symbol_type(tree->children->head->data, outer_map);

        if (ftype == NULL){
                return NULL;
        }

        if ((args_type == NULL) != (ftype->params_type == NULL)){
                return NULL;
        }

        if (args_type && !equals_type(args_type, ftype->params_type)){
                return NULL;
        }

        return return_type(ftype);
}

const struct type *find_parse_tree_type(struct parse_tree *tree, struct OUTER_TYPE_MAP *outer_map, struct MAP(string, type) *scope_map){
        const struct type_system *const system = get_poppy_type_system();
        const struct type *const type = find_type(system, tree, outer_map, scope_map);
        if (type || (tree->data.type == SYMBOL_OPTARGS) || (tree->data.type == SYMBOL_OPTPARAMS)){ // need this since we return NULL for empty type
                return type;
        }
        
        switch (tree->data.type) {
                case SYMBOL_DEFN:
                        assert(scope_map != NULL);
                        return find_defn_type(tree, scope_map);
                default:
                        assert(false);
                        return NULL;
        }
}

struct OUTER_TYPE_MAP * find_types(const struct parse_tree *tree){
        struct OUTER_TYPE_MAP *outer_map = (struct OUTER_TYPE_MAP*) malloc(sizeof (struct OUTER_TYPE_MAP));
        
        struct MAP(string, type) *inner_map = new_inner_map();
        init_map(outer_map, equals_parse_tree, free_typer_entry, parse_tree, MAP(string, type));
        update_map(outer_map, tree, inner_map, parse_tree, MAP(string, type));

        // program -> defns END
        struct parse_tree *defns = tree->children->head->data;

        while (1) {
                // defns -> defn defns
                // defns -> defn
                struct parse_tree *defn = defns->children->head->data;

                struct MAP(string, type) *defn_map = new_inner_map();
                update_map(outer_map, defn, defn_map, parse_tree, MAP(string, type));

                const struct type *defn_type = find_parse_tree_type(defn, NULL, defn_map);
                struct string *id = (struct string*) malloc(sizeof(struct string));
                id->data = find_defn_name(defn);

                update_map(inner_map, id, defn_type, string, type);

                if (defns->children->len == 2){
                        load_child_at(defns, defns, 1);
                } else {
                        break;
                }
        }
        
        defns = tree->children->head->data;
        while (1) {
                // defns -> defn defns
                // defns -> defn
                struct parse_tree *defn = defns->children->head->data;

                // defn -> type IDENTIFIER LPAREN optparams RPAREN LBRACE stmts RBRACE
                struct parse_tree *stmts; load_child_at(stmts, defn, 6);

                struct MAP(string, type) *stmts_map = new_inner_map(); 
                update_map(outer_map, stmts, stmts_map, parse_tree, MAP(string, type));
                // cast to non-const here because map assumes we can't modify values
                const struct type *stmts_type = find_parse_tree_type(stmts, outer_map, stmts_map);
                const struct type *ftype = find_symbol_type(defn->children->head->next->data, outer_map);

                if ((stmts_type == NULL) || (ftype == NULL) || !equals_type(stmts_type, return_type(ftype))){
                        free_map(outer_map, parse_tree, MAP(string, type));
                        free(outer_map);
                        return NULL;
                }

                if (defns->children->len == 2){
                        load_child_at(defns, defns, 1);
                } else {
                        break;
                }
        }

        return outer_map;
}

void get_variables_recursive(const struct parse_tree *tree, struct LIST(string) *list, const struct OUTER_TYPE_MAP *symbols){
        const struct MAP(string, type) *inner_map; query_map(symbols, tree, inner_map, parse_tree, MAP(string, type))

        if (inner_map != NULL){
                for (struct string_type_map_entry_list_node *map_node = inner_map->list->head; map_node != NULL; map_node = map_node->next){
                        append_list(list, (struct string*) map_node->data->key, string);
                }
        }

        if (tree->children != NULL){
                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        get_variables_recursive(node->data, list, symbols);
                }
        }
}

struct LIST(string) get_local_variables(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols){
        // defn -> type IDENTIFIER LPAREN optparams RPAREN LBRACE stmts RBRACE
        const struct parse_tree *stmts; load_child_at(stmts, tree, 6);
        struct LIST(string) list;
        init_list((&list))
        get_variables_recursive(stmts, &list, symbols);
        return list;
}

struct LIST(string) get_parameters(const struct parse_tree *tree, const struct OUTER_TYPE_MAP *symbols){
        // defn -> type IDENTIFIER LPAREN optparams RPAREN LBRACE stmts RBRACE
        const struct MAP(string, type) *inner_map; query_map(symbols, tree, inner_map, parse_tree, MAP(string, type))
        struct LIST(string) list;
        init_list((&list))
        for (struct string_type_map_entry_list_node *map_node = inner_map->list->head; map_node != NULL; map_node = map_node->next){
                append_list((&list), (struct string*) map_node->data->key, string);
        }
        return list;
}
