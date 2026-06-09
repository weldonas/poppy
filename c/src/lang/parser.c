#include "lang/parser.h"

#include <stdbool.h>
#include <stdlib.h>

#include "lang/symbol.h"

struct parse_tree * new_tree(struct token data){
        struct parse_tree *ptr = (struct parse_tree*) malloc(sizeof(struct parse_tree));
        ptr->data = data;
        ptr->children = NULL;
        ptr->parent = NULL;
        ptr->type = NULL;
        return ptr;
}

struct parse_tree * copy_tree(struct parse_tree *tree){
        struct parse_tree *ptr = (struct parse_tree*) malloc(sizeof(struct parse_tree));
        ptr->data = tree->data;
        ptr->parent = tree->parent;
        if (tree->children == NULL){
                ptr->children = NULL;
        } else {
                ptr->children = (struct LIST(parse_tree)*) malloc(sizeof(struct LIST(parse_tree)));
                init_list(ptr->children);

                for (struct LIST_NODE(parse_tree) *node = tree->children->head; node != NULL; node = node->next){
                        struct parse_tree *child = copy_tree(node->data);
                        append_list(ptr->children, child, parse_tree);
                        child->parent = ptr;
                }
        }

        return ptr;
}

void add_child(struct parse_tree *parent, struct parse_tree *child){
        if (parent->children == NULL){
                parent->children = (struct LIST(parse_tree)*) malloc(sizeof(struct LIST(parse_tree)));
                init_list(parent->children);                
        }

        append_list(parent->children, child, parse_tree);
        child->parent = parent;
}

// Earley item
struct item {
        struct rule *rule;
        size_t dot; // position of the dot (occurs before the symbol at this index of rule's RHS)
        size_t start;
        struct parse_tree *tree;
};

struct item * new_item(struct rule *rule, size_t dot, size_t start, struct parse_tree *tree){
        struct item *ptr = (struct item*) malloc(sizeof(struct item));
        ptr->rule = rule;
        ptr->dot = dot;
        ptr->start = start;
        ptr->tree = tree;
        return ptr;
}

void free_item(struct item *item){
        if (item->tree != NULL){
                free_parse_tree(item->tree);
        }
        free(item);
}

bool equals(struct item i1, struct item i2){
        return (i1.rule == i2.rule) && (i1.dot == i2.dot) && (i1.start == i2.start);
}

bool is_done(const struct item item){
        return item.dot == item.rule->rhs_len;
}

DEFINE_LIST(item);

bool append_if_not_present(struct LIST(item) *state_set, struct item *new_item) {
        for (struct LIST_NODE(item) *node = state_set->head; node != NULL; node = node->next){
                if (equals(*node->data, *new_item)){
                        free_item(new_item);
                        return false;
                }
        }

        append_list(state_set, new_item, item)
        return true;
}

const struct parse_tree * const parse(const struct grammar *grammar, const struct LIST(token) *tokens) {
        struct LIST(item) *state_sets = (struct LIST(item)*) malloc((tokens->len + 1) * sizeof(struct LIST(item)));

        for (size_t i = 0; i <= tokens->len; ++i){
                init_list((&state_sets[i]));
        }

        for (size_t i = 0; i < grammar->rules_len; ++i){
                if (grammar->rules[i].lhs == grammar->start){
                        struct token t = {grammar->start, NULL};
                        struct parse_tree *tree = new_tree(t);
                        append_list((&state_sets[0]), new_item(&grammar->rules[i], 0, 0, tree), item);
                }
        }

        struct LIST_NODE(token) *token_node = tokens->head;
        for (size_t i = 0; i <= tokens->len; ++i){
                for (struct LIST_NODE(item) *node = state_sets[i].head; node != NULL; node = node->next){
                        struct item *item = node->data;

                        // prediction
                        if (!is_done(*item) && (!is_terminal(item->rule->rhs[item->dot]))){
                                for (size_t j = 0; j < grammar->rules_len; ++j){
                                        if (grammar->rules[j].lhs == item->rule->rhs[item->dot]){
                                                struct token t = {item->rule->rhs[item->dot], NULL};
                                                struct parse_tree *tree = new_tree(t);
                                                struct item *new = new_item(&grammar->rules[j], 0, i, tree);
                                                append_if_not_present(&state_sets[i], new);
                                        }
                                }

                                if (grammar->nullable[item->rule->rhs[item->dot]]){
                                        struct token t = {item->rule->rhs[item->dot], NULL};
                                        struct parse_tree *tree = copy_tree(item->tree);
                                        add_child(tree, new_tree(t));
                                        struct item *new = new_item(item->rule, item->dot + 1, item->start, tree);
                                        append_if_not_present(&state_sets[i], new);
                                }
                        }

                        // scanning
                        if (!is_done(*item) && is_terminal(item->rule->rhs[item->dot])){
                                if((token_node != NULL) && (token_node->data->type == item->rule->rhs[item->dot])) {
                                        struct parse_tree *tree = item->tree; 
                                        item->tree = NULL;
                                        add_child(tree, new_tree(*token_node->data));
                                        struct item *new = new_item(item->rule, item->dot + 1, item->start, tree);
                                        append_if_not_present(&state_sets[i + 1], new);
                                }
                        }

                        // completion
                        if (is_done(*item)) {
                                for (struct LIST_NODE(item) *start_item_node = state_sets[item->start].head; start_item_node != NULL; start_item_node = start_item_node->next){
                                    struct item *start_item = start_item_node->data;
                                    
                                    if (!is_done(*start_item) && (start_item->rule->rhs[start_item->dot] == item->rule->lhs) && (start_item->rule->rhs_len != 0)){
                                        struct parse_tree *tree = copy_tree(start_item->tree);
                                        add_child(tree, copy_tree(item->tree));
                                        struct item *new = new_item(start_item->rule, start_item->dot + 1, start_item->start, tree);
                                        append_if_not_present(&state_sets[i], new);
                                    } 
                                }
                        }
                }

                if (token_node != NULL){
                        token_node = token_node->next;
                }
        }

        struct parse_tree *ret = NULL;

        for (struct LIST_NODE(item) *node = state_sets[tokens->len].head; node != NULL; node = node->next){
                struct item *i = node->data;
                if (is_done(*i) && (i->start == 0) && (i->rule->lhs == grammar->start)){
                        ret = copy_tree(i->tree);
                        break;
                }
        }

        for (size_t i = 0; i <= tokens->len; ++i){
                free_list((&state_sets[i]), free_item, item);
        }

        free(state_sets);

        return ret;
}
