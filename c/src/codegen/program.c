#include "codegen/program.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/assem.h"
#include "codegen/control.h"
#include "codegen/function.h"
#include "codegen/ops.h"
#include "codegen/register.h"
#include "lang/parse_tree.h"
#include "lang/symbol.h"
#include "lang/type.h"

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

char *generate_head(const struct parse_tree *tree){
        return concat(8,
                literal(".bss"),
                literal("heapmeta:"),
                literal(".skip 8192"),
                literal("heap:"),
                literal(".skip 65536"),
                literal(".text"),
                literal(".include \"utils.s\""),
                literal(".global _start")
        );
}

char *generate_from_tree(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within);
char *generate_value(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within);
char *generate_address(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within);

char *generate_first_child(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        return generate_from_tree(tree->children->head->data, functions, within);
}

char *generate_stmts(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        char *cur = NULL;

        for (struct LIST_NODE(parse_tree) *child = tree->children->head; child != NULL; child = child->next){
                char *child_code = generate_from_tree(child->data, functions, within);
                if (cur){
                        cur = concat(2, cur, child_code);
                }
                else {
                        cur = child_code;
                }
        }

        return cur;
}

char *generate_stmt(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        return generate_first_child(tree, functions, within);
}

char *generate_ifstmt(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        const struct parse_tree *cond; load_child_at(cond, tree, 2);
        char *cond_code = generate_value(cond, functions, within);
        const struct parse_tree *then; load_child_at(then, tree, 5);
        char *then_code = generate_from_tree(then, functions, within);
        const struct parse_tree *optelse;  load_child_at(optelse, tree, 7);

        if ((optelse->children == NULL) || (optelse->children->len == 0)){
                return if_stmt(cond_code, then_code, NULL);
        }
        const struct parse_tree *else_stmts; load_child_at(else_stmts, optelse, 2);
        char *else_code = generate_from_tree(else_stmts, functions, within);
        return if_stmt(cond_code, then_code, else_code);
}

char *generate_whilestmt(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        const struct parse_tree *cond; load_child_at(cond, tree, 2);
        char *cond_code = generate_value(cond, functions, within);
        const struct parse_tree *stmts; load_child_at(stmts, tree, 5);
        char *stmts_code = generate_from_tree(stmts, functions, within);
        return while_loop(cond_code, stmts_code);
}

char *generate_forstmt(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        const struct parse_tree *init; load_child_at(init, tree, 2);
        char *init_code = generate_from_tree(init, functions, within);
        const struct parse_tree *cond; load_child_at(cond, tree, 4);
        char *cond_code = generate_value(cond, functions, within);
        const struct parse_tree *post; load_child_at(post, tree, 6);
        char *post_code = generate_from_tree(post, functions, within);
        const struct parse_tree *body; load_child_at(body, tree, 9);
        char *body_code = generate_from_tree(body, functions, within);
        return for_loop(init_code, cond_code, post_code, body_code); 
}

char *generate_vardec(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 5){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 2);
                char *find_memory_addr = generate_address(addressable, functions, within);
                const struct parse_tree *expr; load_child_at(expr, tree, 4);

                if (is_composite(expr->type)){
                        char *address_code = generate_address(expr, functions, within);
                        return concat(5,
                                find_memory_addr,
                                push(REG_RESULT),
                                address_code,
                                pop(REG_SCRATCH),
                                memory_copy(REG_RESULT, REG_SCRATCH, expr->type->word_count)
                        );
                }
                else {
                        char *value_code = generate_value(expr, functions, within);
                        return concat(5,
                                find_memory_addr,
                                push(REG_RESULT),
                                value_code,
                                pop(REG_SCRATCH),
                                str(REG_RESULT, REG_SCRATCH)
                        );
                }
        }

        char *ret = (char*) malloc(sizeof(char));
        *ret = 0;
        return ret;
}


char *generate_varasst(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        const struct parse_tree *addressable; load_child_at(addressable, tree, 0);
        char *find_memory_addr = generate_address(addressable, functions, within);
        const struct parse_tree *expr; load_child_at(expr, tree, 2);

        if (is_composite(expr->type)){
                char *address_code = generate_address(expr, functions, within);
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        address_code,
                        pop(REG_SCRATCH),
                        memory_copy(REG_RESULT, REG_SCRATCH, expr->type->word_count)
                );
        }
        else {
                char *value_code = generate_value(expr, functions, within);
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        value_code,
                        pop(REG_SCRATCH),
                        str(REG_RESULT, REG_SCRATCH)
                );
        }
}

char *generate_ret(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 1){
                return hop(within);
        }

        const struct parse_tree *second; load_child_at(second, tree, 1);

        if (second->data.type == SYMBOL_TYPE){
                char *ret = (char*) malloc(sizeof(char));
                *ret = 0;
                return ret;
        }
        
        if (is_composite(second->type)){
                char *address = generate_address(second, functions, within);
                return concat(2, address, hop(within));
        }

        char *value = generate_value(second, functions, within);
        return concat(2, value, hop(within));
}
char *generate_andcond(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 1){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        char *op1 = generate_value(tree->children->head->data, functions, within);
        char *op2 = generate_value(tree->children->head->next->next->data, functions, within);
        return cnjtn(op1, op2);
}

char *generate_orcond(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 1){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        char *op1 = generate_value(tree->children->head->data, functions, within);
        char *op2 = generate_value(tree->children->head->next->next->data, functions, within);
        return dsjtn(op1, op2);    
}

char *generate_uncond(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 1){
                if (tree->children->head->data->data.type == SYMBOL_TRUE){
                        return movi(REG_RESULT, 1);
                } else if (tree->children->head->data->data.type == SYMBOL_FALSE){
                        return movi(REG_RESULT, 0);
                }

                return generate_from_tree(tree->children->head->data, functions, within);
        }

        if (tree->children->len == 2){
                return ngtn(generate_value(tree->children->head->next->data, functions, within));
        }

        enum symbol op_type = tree->children->head->next->data->data.type;
        if (op_type == SYMBOL_EXPR){
                return generate_from_tree(tree->children->head->next->data, functions, within);
        }

        char *op1 = generate_value(tree->children->head->data, functions, within);
        char *op2 = generate_value(tree->children->head->next->next->data, functions, within);
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
                        return NULL;
        }
}

char *generate_addmultbitexpr(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->children->len == 1){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        if (tree->children->head->data->data.type == SYMBOL_BNOT){
                char *op = generate_value(tree->children->head->next->data, functions, within);
                return bnot(op);
        }

        char *op1 = generate_value(tree->children->head->data, functions, within);
        char *op2 = generate_value(tree->children->head->next->next->data, functions, within);
        enum symbol operand = tree->children->head->next->data->data.type;
        switch(operand){
                case SYMBOL_PLUS:
                        return sum(op1, op2);
                case SYMBOL_MINUS:
                        return subtract(op1, op2);
                case SYMBOL_STAR:
                        return multiply(op1, op2);
                case SYMBOL_DIVIDE:
                        return divide(op1, op2);
                case SYMBOL_MOD:
                        return modulo(op1, op2);
                case SYMBOL_AMP:
                        return band(op1, op2);
                case SYMBOL_BOR:
                        return bor(op1, op2);
                case SYMBOL_BXOR:
                        return bxor(op1, op2);
                case SYMBOL_BLEFT:
                        return bleft(op1, op2);
                case SYMBOL_BRIGHT:
                        return bright(op1, op2);
                default:
                        assert(0);
                        return NULL;
        }
}

char *generate_unexpr(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        enum symbol first = tree->children->head->data->data.type;
        if (first == SYMBOL_MINUS){
                return subtract(
                        movi(REG_RESULT, 0), 
                        generate_value(tree->children->head->next->data, functions, within)
                );
        }

        if (first == SYMBOL_INC){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 1);
                char *find_memory_addr = generate_address(addressable, functions, within);

                return concat(5,
                        find_memory_addr,
                        ldr(REG_SCRATCH2, REG_RESULT),
                        movi(REG_SCRATCH, 1),
                        add(REG_SCRATCH2, REG_SCRATCH2, REG_SCRATCH),
                        str(REG_SCRATCH2, REG_RESULT)
                );
        }

        if (first == SYMBOL_DEC){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 1);
                char *find_memory_addr = generate_address(addressable, functions, within);

                return concat(5,
                        find_memory_addr,
                        ldr(REG_SCRATCH2, REG_RESULT),
                        movi(REG_SCRATCH, 1),
                        sub(REG_SCRATCH2, REG_SCRATCH2, REG_SCRATCH),
                        str(REG_SCRATCH2, REG_RESULT)
                );
        }

        if (first == SYMBOL_LPAREN){
                return generate_from_tree(tree->children->head->next->data, functions, within);
        }

        if (first == SYMBOL_CONSTANT){
                long long imm = strtoll(tree->children->head->data->data.value, NULL, 10);
                return movi(REG_RESULT, imm);
        }

        if (first == SYMBOL_CHARLIT){
                char *data = tree->children->head->data->data.value;
                long long imm = data[0];
                return movi(REG_RESULT, imm);
        }

        if (first == SYMBOL_CALL){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        if (first == SYMBOL_CAST){
                return generate_from_tree(tree->children->head->data, functions, within);
        }

        if ((tree->children->len == 1) && (tree->children->head->data->data.type == SYMBOL_IDENTIFIER)){
                char *address = generate_address(tree, functions, within);
                return concat(2, address, ldr(REG_RESULT, REG_RESULT));
        }

        if (tree->children->len == 1){
                return generate_first_child(tree, functions, within);
        }

        if (tree->children->head->data->data.type == SYMBOL_AMP){
                return generate_address(tree->children->head->next->data, functions, within);
        }

        if (tree->children->head->data->data.type == SYMBOL_STAR){
                char *find_memory_address = generate_value(tree->children->head->next->data, functions, within);
                return concat(2,
                        find_memory_address,
                        ldr(REG_RESULT, REG_RESULT)
                );
        }

        if (tree->children->head->data->data.type == SYMBOL_ASM){
                const struct parse_tree *instr_tree; load_child_at(instr_tree, tree, 2);
                char *ret = (char*) malloc((strlen(instr_tree->data.value) + 1) * sizeof(char));
                strcpy(ret, instr_tree->data.value);
                return ret;
        }

        if (tree->children->head->next->data->data.type == SYMBOL_DOT){
                char *address = generate_address(tree, functions, within);
                return concat(2, address, ldr(REG_RESULT, REG_RESULT));
        }

        if (tree->children->len == 4){
                char *address = generate_address(tree, functions, within);
                return concat(2, address, ldr(REG_RESULT, REG_RESULT));
        }

        print_parse_tree(tree);
        assert(0);
        return NULL;
}

char *generate_call(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        char *id = tree->children->head->data->data.value;
        struct string s;
        s.data = id;
        const struct function *f; query_map(functions, (&s), f, string, function);
        char **args_code = (char**) malloc(num_params(f) * sizeof(char*));

        size_t i = 0;
        
        const struct parse_tree *optargs = tree->children->head->next->next->data;
        if ((optargs->children != NULL) && (optargs->children->len != 0)){
                const struct parse_tree *args = optargs->children->head->data;

                for (struct LIST_NODE(parse_tree) *child = args->children->head; child != NULL; child = child->next ? child->next->next : NULL){
                        args_code[i++] = is_composite(child->data->type)
                                ? generate_address(child->data, functions, within)
                                : generate_value(child->data, functions, within);
                }
        }

        if (num_params(f) == 0){
                free(args_code);
                return call_function(f, NULL);
        }

        char *ret = call_function(f, args_code);
        free(args_code);
        return ret;
}

char *generate_cast(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        const struct parse_tree *src_tree; 
        if (tree->children->len == 4){
                load_child_at(src_tree, tree, 2);
        }
        else {
                load_child_at(src_tree, tree, 3);
        }

        return generate_from_tree(src_tree, functions, within);
}

char *generate_address(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        if (tree->data.type == SYMBOL_IDENTIFIER){
                struct variable var = find_symbol_variable(tree);
                return function_variable_address(within, var, REG_RESULT);
        }
        
        if (tree->children->len == 1){
                return generate_address(tree->children->head->data, functions, within);
        }

        if (tree->data.type == SYMBOL_CALL){
                return generate_call(tree, functions, within);
        }

        if (tree->data.type == SYMBOL_CAST){
                return generate_address(tree->children->head->next->next->data, functions, within);
        }

        if (tree->children->head->data->data.type == SYMBOL_LPAREN){
                return generate_address(tree->children->head->next->data, functions, within);
        }

        if (tree->children->head->data->data.type == SYMBOL_STAR){
                char *referenced = generate_address(tree->children->head->next->data, functions, within);
                return concat(2, referenced, ldr(REG_RESULT, REG_RESULT));
        }

        if (tree->children->head->next->data->data.type == SYMBOL_DOT){
                const struct parse_tree *record_tree; load_child_at(record_tree, tree, 0);
                const struct parse_tree *field_tree; load_child_at(field_tree, tree, 2);
                size_t field_offset = record_type_offset(record_tree->type, field_tree->data.value);

                char *record = generate_address(record_tree, functions, within);

                return concat(3,
                        record,
                        movi(REG_SCRATCH, 8 * field_offset),
                        add(REG_RESULT, REG_RESULT, REG_SCRATCH)
                );
        }

        if (tree->children->head->next->data->data.type == SYMBOL_LBRACKET){
                const struct parse_tree *array_tree; load_child_at(array_tree, tree, 0);
                const struct parse_tree *index_tree; load_child_at(index_tree, tree, 2);
                uint32_t element_size = array_tree->type->element_type->word_count;
                char *array_address = generate_address(array_tree, functions, within);
                char *index = generate_value(index_tree, functions, within);

                return concat(7,
                        array_address,
                        push(REG_RESULT),
                        index,
                        movi(REG_SCRATCH, 8 * element_size),
                        mul(REG_RESULT, REG_RESULT, REG_SCRATCH),
                        pop(REG_SCRATCH),
                        add(REG_RESULT, REG_RESULT, REG_SCRATCH)
                );
        }

        print_parse_tree(tree);
        assert(0);
        return NULL;
}

char *generate_value(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        assert(tree->type->word_count == 1);
        return generate_from_tree(tree, functions, within);

        print_parse_tree(tree);
        assert(0);
        return NULL;
}

char *generate_from_tree(const struct parse_tree *tree, struct MAP(string, function) *functions, const struct function *within){
        enum symbol symbol = tree->data.type;

        if (symbol == SYMBOL_STMTS){
                return generate_stmts(tree, functions, within);
        } else if (symbol == SYMBOL_STMT){
                return generate_stmt(tree, functions, within);
        } else if (symbol == SYMBOL_IFSTMT){
                return generate_ifstmt(tree, functions, within);
        } else if (symbol == SYMBOL_WHILESTMT){
                return generate_whilestmt(tree, functions, within);
        } else if (symbol == SYMBOL_FORSTMT){
                return generate_forstmt(tree, functions, within);
        } else if (symbol == SYMBOL_SEMISTMT){
                return generate_first_child(tree, functions, within);
        } else if (symbol == SYMBOL_VARDEC){
                return generate_vardec(tree, functions, within);
        } else if (symbol == SYMBOL_VARASST){
                return generate_varasst(tree, functions, within);
        } else if (symbol == SYMBOL_RET){
                return generate_ret(tree, functions, within);
        } else if (symbol == SYMBOL_ANDCOND){
                return generate_andcond(tree, functions, within);
        } else if (symbol == SYMBOL_ORCOND){
                return generate_orcond(tree, functions, within);
        } else if (symbol == SYMBOL_UNCOND){
                return generate_uncond(tree, functions, within);
        } else if (symbol == SYMBOL_EXPR){
                return generate_first_child(tree, functions, within);
        } else if ((symbol == SYMBOL_ADDEXPR) || (symbol == SYMBOL_MULTEXPR) || (symbol == SYMBOL_BITEXPR)){
                return generate_addmultbitexpr(tree, functions, within);
        } else if (symbol == SYMBOL_UNEXPR){          
                return generate_unexpr(tree, functions, within);
        } else if (symbol == SYMBOL_CALL){
                return generate_call(tree, functions, within);
        } else if (symbol == SYMBOL_BODY){
                return generate_first_child(tree, functions, within);
        } else if (symbol == SYMBOL_CAST){
                return generate_cast(tree, functions, within);
        }

        assert(0);
        return NULL;
}

char *generate_code(const struct parse_tree *tree){
        struct MAP(string, function) functions; init_map((&functions), equals_string, free_string_function_entry, string, function);
        // program -> defndecls END
        const struct parse_tree *defndecls = tree->children->head->data;

        for (struct LIST_NODE(parse_tree) *node = defndecls->children->head; node != NULL; node = node->next) {
                const struct parse_tree *defndecl = node->data;
                const struct parse_tree *defn = defndecl->children->head->data;

                if (defn->data.type != SYMBOL_FNDEFN){
                        continue;
                }

                struct LIST(variable) params_list = get_parameters(defn);
                struct LIST(variable) locals_list = get_local_variables(defn);

                struct string *s = (struct string*) malloc(sizeof(struct string));
                const struct parse_tree *signature = defn->children->head->data;
                s->data = signature->children->head->next->data->data.value;
                bool is_main = strcmp("main", s->data) == 0;

                struct function *fn = new_function(params_list, locals_list, is_main);
                free_list((&locals_list), free_variable, variable);

                update_map((&functions), s, fn, string, function);
        }

        for (struct LIST_NODE(parse_tree) *node = defndecls->children->head; node != NULL; node = node->next) {
                const struct parse_tree *defndecl = node->data;
                const struct parse_tree *defn = defndecl->children->head->data;

                if (defn->data.type != SYMBOL_FNDEFN){
                        continue;
                }

                const struct parse_tree *signature = defn->children->head->data;
                struct string s;
                s.data = signature->children->head->next->data->data.value;;
                const struct function *fn; query_map((&functions), (&s), fn, string, function);

                const struct parse_tree *stmts; load_child_at(stmts, defn, 2);
                set_body((struct function*) fn, generate_from_tree(stmts, &functions, fn));
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
