#include "codegen/program.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/assem.h"
#include "codegen/control.h"
#include "codegen/function.h"
#include "codegen/ops.h"
#include "lang/parser.h"
#include "lang/symbol.h"

#define load_child_at(var, tree, n)                                        \
        do {                                                               \
                struct LIST_NODE(parse_tree) *node = tree->children->head; \
                for (int i = 0; i < n; ++i){                               \
                        node = node->next;                                 \
                }                                                          \
                var = node->data;                                          \
        } while (0);                                                       \

DEFINE_MAP(string, function);

const char *tail = "mov x0, #0\n"
                  "mov w8, #93\n"
                  "svc #0\n";

const char *start_label = "_start:";

void free_string_function_entry(const struct MAP_ENTRY(string, function) *entry) {
        free((void*) entry->key);
        free_function(entry->value);
        free((void*) entry);
}

void free_string(const struct string *str){
        // free((void*) str);
}

char *generate_head(struct parse_tree *tree){
        char *head = (char*) malloc(6 * sizeof(char));
        strcpy(head, ".text");
        char *global_start = (char*) malloc(15 * sizeof(char));
        strcpy(global_start, ".global _start");
        return concat(2, head, global_start);
}

char *generate_from_tree(struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        enum symbol symbol = tree->data.type;

        if (symbol == SYMBOL_STMTS){
                char *first_stmt = generate_from_tree(tree->children->head->data, functions, within);
                if (tree->children->len == 1){
                        return first_stmt;
                }

                return concat(2, first_stmt, generate_from_tree(tree->children->head->next->data, functions, within));
        } else if (symbol == SYMBOL_STMT){
                struct parse_tree *child = tree->children->head->data;
                enum symbol child_symbol = child->data.type;
                if (child_symbol == SYMBOL_SEMISTMT){
                        enum symbol grandchild_symbol = child->children->head->data->data.type;
                        if (grandchild_symbol == SYMBOL_ASM){
                                char *instr = child->children->head->next->next->data->data.value;
                                char *ret = (char*) malloc((strlen(instr) + 1) * sizeof(char));
                                strcpy(ret, instr);
                                return ret;
                        }
                }
                return generate_from_tree(tree->children->head->data, functions, within);
        } else if (symbol == SYMBOL_IFSTMT){
                struct parse_tree *cond; load_child_at(cond, tree, 2);
                char *cond_code = generate_from_tree(cond, functions, within);
                struct parse_tree *then; load_child_at(then, tree, 5);
                char *then_code = generate_from_tree(then, functions, within);
                struct parse_tree *optelse;  load_child_at(optelse, tree, 7);

                if ((optelse->children == NULL) || (optelse->children->len == 0)){
                        return if_stmt(cond_code, then_code, NULL);
                }
                struct parse_tree *else_stmts; load_child_at(else_stmts, optelse, 2);
                char *else_code = generate_from_tree(else_stmts, functions, within);
                return if_stmt(cond_code, then_code, else_code);
        } else if (symbol == SYMBOL_WHILESTMT){
                struct parse_tree *cond; load_child_at(cond, tree, 2);
                char *cond_code = generate_from_tree(cond, functions, within);
                struct parse_tree *stmts; load_child_at(stmts, tree, 5);
                char *stmts_code = generate_from_tree(stmts, functions, within);
                return while_loop(cond_code, stmts_code);
        } else if (symbol == SYMBOL_FORSTMT){
                struct parse_tree *init; load_child_at(init, tree, 2);
                char *init_code = generate_from_tree(init, functions, within);
                struct parse_tree *cond; load_child_at(cond, tree, 4);
                char *cond_code = generate_from_tree(cond, functions, within);
                struct parse_tree *post; load_child_at(post, tree, 6);
                char *post_code = generate_from_tree(post, functions, within);
                struct parse_tree *body; load_child_at(body, tree, 9);
                char *body_code = generate_from_tree(body, functions, within);
                return for_loop(init_code, cond_code, post_code, body_code);      
        } else if (symbol == SYMBOL_SEMISTMT){
                return generate_from_tree(tree->children->head->data, functions, within);
        } else if (symbol == SYMBOL_VARDEC){
                if (tree->children->len == 5){
                        struct parse_tree *id; load_child_at(id, tree, 2);
                        char *var = id->data.value;
                        struct parse_tree *expr; load_child_at(expr, tree, 4);
                        char *expr_code = generate_from_tree(expr, functions, within);
                        return concat(2, expr_code, write_function_variable(within, var, REG_ARITH_RESULT));
                }

                char *ret = (char*) malloc(sizeof(char));
                *ret = 0;
                return ret;
        } else if (symbol == SYMBOL_VARASST){
                struct parse_tree *id; load_child_at(id, tree, 0);
                char *var = id->data.value;
                struct parse_tree *expr; load_child_at(expr, tree, 2);
                char *expr_code = generate_from_tree(expr, functions, within);
                return concat(2, expr_code, write_function_variable(within, var, REG_ARITH_RESULT));                
        } else if (symbol == SYMBOL_RET){
                if (tree->children->len == 1){
                        return hop(within);
                }

                struct parse_tree *second; load_child_at(second, tree, 1);

                if (second->data.type == SYMBOL_TYPE){
                        char *ret = (char*) malloc(sizeof(char));
                        *ret = 0;
                        return ret;
                }
                
                char *expr_code = generate_from_tree(second, functions, within);
                return concat(2, expr_code, hop(within));
        } else if (symbol == SYMBOL_ANDCOND){
                if (tree->children->len == 1){
                        return generate_from_tree(tree->children->head->data, functions, within);
                }

                char *op1 = generate_from_tree(tree->children->head->data, functions, within);
                char *op2 = generate_from_tree(tree->children->head->next->next->data, functions, within);
                return cnjtn(op1, op2);
        } else if (symbol == SYMBOL_ORCOND){
                if (tree->children->len == 1){
                        return generate_from_tree(tree->children->head->data, functions, within);
                }

                char *op1 = generate_from_tree(tree->children->head->data, functions, within);
                char *op2 = generate_from_tree(tree->children->head->next->next->data, functions, within);
                return dsjtn(op1, op2);                
        } else if (symbol == SYMBOL_UNCOND){
                if (tree->children->len == 1){
                        if (tree->children->head->data->data.type == SYMBOL_TRUE){
                                return movi(REG_ARITH_RESULT, 1);
                        } else if (tree->children->head->data->data.type == SYMBOL_FALSE){
                                return movi(REG_ARITH_RESULT, 0);
                        }

                        return generate_from_tree(tree->children->head->data, functions, within);
                }

                if (tree->children->len == 2){
                        return ngtn(generate_from_tree(tree->children->head->next->data, functions, within));
                }

                enum symbol op_type = tree->children->head->next->data->data.type;
                if (op_type == SYMBOL_EXPR){
                        return generate_from_tree(tree->children->head->next->data, functions, within);
                }

                char *op1 = generate_from_tree(tree->children->head->data, functions, within);
                char *op2 = generate_from_tree(tree->children->head->next->next->data, functions, within);
                switch (op_type){
                        case SYMBOL_LT:
                                return lt(op1, op2);
                        case SYMBOL_GT:
                                return gt(op1, op2);
                        case SYMBOL_LE:
                                return le(op1, op2);
                        case SYMBOL_GE:
                                return ge(op1, op2);
                        case SYMBOL_EQ:
                                return eq(op1, op2);
                        case SYMBOL_NE:
                                return ne(op1, op2);
                        default:
                                assert(0);
                }
        } else if (symbol == SYMBOL_EXPR){
                return generate_from_tree(tree->children->head->data, functions, within);
        } else if ((symbol == SYMBOL_ADDEXPR) || (symbol == SYMBOL_MULTEXPR)){
                if (tree->children->len == 1){
                        return generate_from_tree(tree->children->head->data, functions, within);
                }

                char *op1 = generate_from_tree(tree->children->head->data, functions, within);
                char *op2 = generate_from_tree(tree->children->head->next->next->data, functions, within);
                enum symbol operand = tree->children->head->next->data->data.type;
                switch(operand){
                        case SYMBOL_PLUS:
                                return sum(op1, op2);
                        case SYMBOL_MINUS:
                                return subtract(op1, op2);
                        case SYMBOL_TIMES:
                                return multiply(op1, op2);
                        case SYMBOL_DIVIDE:
                                return divide(op1, op2);
                        case SYMBOL_MOD:
                                return modulo(op1, op2);
                        default:
                                assert(0);
                }

                return subtract(op1, op2);
        } else if (symbol == SYMBOL_UNEXPR){          
                enum symbol first = tree->children->head->data->data.type;
                if (first == SYMBOL_MINUS){
                        return subtract(
                                movi(REG_ARITH_RESULT, 0), 
                                generate_from_tree(tree->children->head->next->data, functions, within)
                        );
                }

                if (first == SYMBOL_INC){
                        char *var = tree->children->head->next->data->data.value;
                        return concat(2,
                                sum(
                                        read_function_variable(within, REG_ARITH_RESULT, var),
                                        movi(REG_ARITH_RESULT, 1)
                                ),
                                write_function_variable(within, var, REG_ARITH_RESULT)
                        );
                }

                if (first == SYMBOL_DEC){
                        char *var = tree->children->head->next->data->data.value;
                        return concat(2,
                                subtract(
                                        read_function_variable(within, REG_ARITH_RESULT, var),
                                        movi(REG_ARITH_RESULT, 1)
                                ),
                                write_function_variable(within, var, REG_ARITH_RESULT)
                        );
                }

                if (first == SYMBOL_LPAREN){
                        return generate_from_tree(tree->children->head->next->data, functions, within);
                }

                if (first == SYMBOL_CONSTANT){
                        long long imm = strtoll(tree->children->head->data->data.value, NULL, 10);
                        return movi(REG_ARITH_RESULT, imm);
                }

                if (first == SYMBOL_CHARLIT){
                        char *data = tree->children->head->data->data.value;
                        long long imm = data[0];
                        return movi(REG_ARITH_RESULT, imm);
                }

                if (first == SYMBOL_IDENTIFIER){
                        char *id = tree->children->head->data->data.value;

                        return read_function_variable(within, REG_ARITH_RESULT, id);
                }

                if (first == SYMBOL_CALL){
                        return generate_from_tree(tree->children->head->data, functions, within);
                }
        } else if (symbol == SYMBOL_CALL){
                char *id = tree->children->head->data->data.value;
                struct string s;
                s.data = id;
                const struct function *f; query_map(functions, (&s), f, string, function);
                char **args_code = (char**) malloc(num_params(f) * sizeof(char*));

                size_t i = 0;
                
                struct parse_tree *optargs = tree->children->head->next->next->data;
                if ((optargs->children != NULL) && (optargs->children->len != 0)){
                        struct parse_tree *args = optargs->children->head->data;
                        while (1) {
                                // args -> expr COMMA args
                                // args -> expr
                                struct parse_tree *expr = args->children->head->data;
                                args_code[i++] = generate_from_tree(expr, functions, within);

                                if (args->children->len == 3){
                                        load_child_at(args, args, 2);
                                } else {
                                        break;
                                }
                        };
                }

                if (num_params(f) == 0){
                        free(args_code);
                        return call_function(f, NULL);
                }

                char *ret = call_function(f, args_code);
                free(args_code);
                return ret;
        } else if (symbol == SYMBOL_BODY){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        assert(0);
        return NULL;
}

char *generate_code(const struct OUTER_TYPE_MAP *type_map, const struct parse_tree *tree){
        struct MAP(string, function) functions; init_map((&functions), equals_string, free_string_function_entry, string, function);
        // program -> defndecls END
        struct parse_tree *defndecls = tree->children->head->data;

        while (1) {
                // defndecls -> defndecl defndecls
                // defndecls -> defndecl
                struct parse_tree *defndecl = defndecls->children->head->data;
                struct parse_tree *defn = defndecl->children->head->data;

                struct LIST(string) params_list = get_parameters(defn, type_map);
                struct LIST(string) locals_list = get_local_variables(defn, type_map);

                for (struct LIST_NODE(string) *l_node = locals_list.head; l_node != NULL; l_node = l_node->next){
                        for (struct LIST_NODE(string) *successor = l_node->next; successor != NULL; successor = successor->next){
                                if (strcmp(l_node->data->data, successor->data->data) == 0){
                                        free_map((&functions), string, function);
                                        free_list((&params_list), free_string, string);
                                        free_list((&locals_list), free_string, string);
                                        return NULL;
                                }
                        }

                        for (struct LIST_NODE(string) *p_node = params_list.head; p_node != NULL; p_node = p_node->next){
                                if (strcmp(l_node->data->data, p_node->data->data) == 0){
                                        free_map((&functions), string, function);
                                        free_list((&params_list), free_string, string);
                                        free_list((&locals_list), free_string, string);
                                        return NULL;
                                }
                        }
                }

                char **params = (char**) malloc(params_list.len * sizeof(char*));
                size_t i = 0;
                for (struct LIST_NODE(string) *node = params_list.head; node != NULL; node = node->next){
                        params[i++] = node->data->data;
                }

                char **locals = (char**) malloc(locals_list.len * sizeof(char*));
                i = 0;
                for (struct LIST_NODE(string) *node = locals_list.head; node != NULL; node = node->next){
                        locals[i++] = node->data->data;
                }

                struct string *s = (struct string*) malloc(sizeof(struct string));
                struct parse_tree *signature = defn->children->head->data;
                s->data = signature->children->head->next->data->data.value;
                bool is_main = strcmp("main", s->data) == 0;

                struct function *fn = new_function(params, params_list.len, locals, locals_list.len, is_main);
                free_list((&params_list), free_string, string);
                free_list((&locals_list), free_string, string);
                free(locals);

                update_map((&functions), s, fn, string, function);

                if (defndecls->children->len == 2){
                        load_child_at(defndecls, defndecls, 1);
                } else {
                        break;
                }
        }

        defndecls = tree->children->head->data;
        while (1) {
                // defndecls -> defndecl defndecls
                // defndecls -> defndecl
                struct parse_tree *defndecl = defndecls->children->head->data;
                struct parse_tree *defn = defndecl->children->head->data;
                struct parse_tree *signature = defn->children->head->data;
                struct string s;
                s.data = signature->children->head->next->data->data.value;;
                const struct function *fn; query_map((&functions), (&s), fn, string, function);

                struct parse_tree *stmts; load_child_at(stmts, defn, 2);
                set_body((struct function*) fn, generate_from_tree(stmts, &functions, fn));

                if (defndecls->children->len == 2){
                        load_child_at(defndecls, defndecls, 1);
                } else {
                        break;
                }
        }

        // char *prog = (char*) malloc(44 * sizeof(char));
        // strcpy(prog, head);
        char *prog = generate_head(tree->children->head->data);
        
        for (struct string_function_map_entry_list_node *node = functions.list->head; node != NULL; node = node->next){
                if (strcmp(node->data->key->data, "main") == 0){
                        continue;
                }

                const struct function *f = node->data->value;
                prog = concat(2, prog, declare_function(f));
        }

        struct string s = {"main"};
        const struct function *main; query_map((&functions), (&s), main, string, function);

        if (main == NULL){
                free_map((&functions), string, function);
                free(prog);
                return NULL;
        }

        char *sl = (char*) malloc(8 * sizeof(char));
        char *t = (char*) malloc(31 * sizeof(char));
        strcpy(sl, start_label);
        strcpy(t, tail);
        prog = concat(4, prog, sl, declare_function(main), t);
        free_map((&functions), string, function);
        return prog;
}
