#include "codegen/program.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/assem.h"
#include "codegen/control.h"
#include "codegen/function.h"
#include "codegen/ops.h"
#include "codegen/register.h"
#include "data/list.h"
#include "lang/parse_tree.h"
#include "lang/symbol.h"
#include "lang/type.h"

DEFINE_MAP(string, function);


struct literal {
        char *value;
};

DEFINE_LIST(literal);

void free_literal(struct literal *l){
        free(l);
}

struct codegen_params {
        struct MAP(string, function) *functions;
        const struct function *within;
        struct LIST(literal) literals;
};

const char *tail = "mov x0, #0\n"
                  "mov w8, #93\n"
                  "svc #0\n";

const char *start_label = "_start:";

void free_string_function_entry(const struct MAP_ENTRY(string, function) *entry) {
        free((void*) entry->key);
        free_function(entry->value);
        free((void*) entry);
}

char *generate_head(struct LIST(literal) literals){
        // see if we can make this readonly
        char *literal_declare = literal(".data");

        uint64_t i = 0;
        for (struct LIST_NODE(literal) *node = literals.head; node != NULL; node = node->next){
                char *cur = malloc((43 + strlen(node->data->value)) * sizeof(char));
                *cur = 0;

                char i_str[20];
                sprintf(i_str, "%lu", i);

                strcat(cur, "__stringlit");
                strcat(cur, i_str);
                strcat(cur, ": .ascii \"");
                strcat(cur, node->data->value);
                strcat(cur, "\"");

                literal_declare = concat(2, literal_declare, cur);

                ++i;
        }

        return concat(11,
                literal(".bss"),
                literal("__heapmeta:"),
                literal(".skip 8192"),
                literal("__heap:"),
                literal(".skip 65536"),
                literal("__stringlittemp:"),
                literal(".skip 16"),
                literal_declare,
                literal(".text"),
                literal(".include \"utils.s\""),
                literal(".global _start")
        );
}

char *generate_from_tree(const struct parse_tree *tree, struct codegen_params *params);
char *generate_value(const struct parse_tree *tree, struct codegen_params *params);
char *generate_address(const struct parse_tree *tree, struct codegen_params *params);

char *generate_first_child(const struct parse_tree *tree, struct codegen_params *params){
        return generate_from_tree(tree->children->head->data, params);
}

char *generate_stmts(const struct parse_tree *tree, struct codegen_params *params){
        char *cur = NULL;

        for (struct LIST_NODE(parse_tree) *child = tree->children->head; child != NULL; child = child->next){
                char *child_code = generate_from_tree(child->data, params);
                if (cur){
                        cur = concat(2, cur, child_code);
                }
                else {
                        cur = child_code;
                }
        }

        return cur;
}

char *generate_stmt(const struct parse_tree *tree, struct codegen_params *params){
        return generate_first_child(tree, params);
}

char *generate_ifstmt(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *cond; load_child_at(cond, tree, 2);
        char *cond_code = generate_value(cond, params);
        const struct parse_tree *then; load_child_at(then, tree, 5);
        char *then_code = generate_from_tree(then, params);
        const struct parse_tree *optelse;  load_child_at(optelse, tree, 7);

        if ((optelse->children == NULL) || (optelse->children->len == 0)){
                return if_stmt(cond_code, then_code, NULL);
        }
        const struct parse_tree *else_stmts; load_child_at(else_stmts, optelse, 2);
        char *else_code = generate_from_tree(else_stmts, params);
        return if_stmt(cond_code, then_code, else_code);
}

char *generate_whilestmt(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *cond; load_child_at(cond, tree, 2);
        char *cond_code = generate_value(cond, params);
        const struct parse_tree *stmts; load_child_at(stmts, tree, 5);
        char *stmts_code = generate_from_tree(stmts, params);
        return while_loop(cond_code, stmts_code);
}

char *generate_forstmt(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *init; load_child_at(init, tree, 2);
        char *init_code = generate_from_tree(init, params);
        const struct parse_tree *cond; load_child_at(cond, tree, 4);
        char *cond_code = generate_value(cond, params);
        const struct parse_tree *post; load_child_at(post, tree, 6);
        char *post_code = generate_from_tree(post, params);
        const struct parse_tree *body; load_child_at(body, tree, 9);
        char *body_code = generate_from_tree(body, params);
        return for_loop(init_code, cond_code, post_code, body_code); 
}

char *generate_vardec(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 5){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 2);
                char *find_memory_addr = generate_address(addressable, params);
                const struct parse_tree *expr; load_child_at(expr, tree, 4);

                if (expr->type->byte_count > 8){
                        char *address_code = generate_address(expr, params);
                        return concat(5,
                                find_memory_addr,
                                push(REG_RESULT),
                                address_code,
                                pop(REG_SCRATCH),
                                memory_copy(REG_RESULT, REG_SCRATCH, expr->type->byte_count)
                        );
                }
                else if (expr->type->byte_count == 8){
                        char *value_code = generate_value(expr, params);
                        return concat(5,
                                find_memory_addr,
                                push(REG_RESULT),
                                value_code,
                                pop(REG_SCRATCH),
                                str(REG_RESULT, REG_SCRATCH)
                        );
                }
                else {
                        char *value_code = generate_value(expr, params);
                        return concat(5,
                                find_memory_addr,
                                push(REG_RESULT),
                                value_code,
                                pop(REG_SCRATCH),
                                set_bytes_addr(REG_RESULT, REG_SCRATCH, expr->type->byte_count)
                        );
                }
        }

        char *ret = malloc(sizeof(char));
        *ret = 0;
        return ret;
}


char *generate_plain_varasst(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *addressable; load_child_at(addressable, tree, 0);
        char *find_memory_addr = generate_address(addressable, params);
        const struct parse_tree *expr; load_child_at(expr, tree, 2);
        if (expr->type->byte_count > 8){
                char *address_code = generate_address(expr, params);
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        address_code,
                        pop(REG_SCRATCH),
                        memory_copy(REG_RESULT, REG_SCRATCH, expr->type->byte_count)
                );
        }
        else if (expr->type->byte_count == 8){
                char *value_code = generate_value(expr, params);
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        value_code,
                        pop(REG_SCRATCH),
                        str(REG_RESULT, REG_SCRATCH)
                );
        }
        else {
                char *value_code = generate_value(expr, params);
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        value_code,
                        pop(REG_SCRATCH),
                        set_bytes_addr(REG_RESULT, REG_SCRATCH, expr->type->byte_count)
                );
        }
}

char *generate_compound_varasst(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *addressable; load_child_at(addressable, tree, 0);
        char *find_memory_addr = generate_address(addressable, params);
        const struct parse_tree *expr; load_child_at(expr, tree, 3);
        assert(expr->type->byte_count <= 8);

        char *value_code = generate_value(expr, params);

        char *compound_op;
        enum symbol operator = tree->children->head->next->data->data.type;
        char *deref = ldr(REG_RESULT, REG_RESULT);
        switch(operator){
                case SYMBOL_PLUS:
                        compound_op = sum(deref, value_code);
                        break;
                case SYMBOL_MINUS:
                        compound_op = subtract(deref, value_code);
                        break;
                case SYMBOL_STAR:
                        compound_op = multiply(deref, value_code);
                        break;
                case SYMBOL_DIVIDE:
                        compound_op = divide(deref, value_code);
                        break;
                case SYMBOL_MOD:
                        compound_op = modulo(deref, value_code);
                        break;
                case SYMBOL_AMP:
                        compound_op = band(deref, value_code);
                        break;
                case SYMBOL_BOR:
                        compound_op = bor(deref, value_code);
                        break;
                case SYMBOL_BXOR:
                        compound_op = bxor(deref, value_code);
                        break;
                case SYMBOL_BLEFT:
                        compound_op = bleft(deref, value_code);
                        break;
                case SYMBOL_BRIGHT:
                        compound_op = bright(deref, value_code);
                        break;
                default:
                        assert(0);
        }

        if (expr->type->byte_count == 8){
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        compound_op,
                        pop(REG_SCRATCH),
                        str(REG_RESULT, REG_SCRATCH)
                );
        }
        else if (expr->type->byte_count < 8){
                return concat(5,
                        find_memory_addr,
                        push(REG_RESULT),
                        compound_op,
                        pop(REG_SCRATCH),
                        set_bytes_addr(REG_RESULT, REG_SCRATCH, expr->type->byte_count)
                );
        }
        else{
                assert(0);
                return NULL;
        }
}

char *generate_varasst(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 3){
                return generate_plain_varasst(tree, params);
        }
        else if (tree->children->len == 4){
                return generate_compound_varasst(tree, params);
        }
        assert(0);
        return NULL;
}


char *generate_ret(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 1){
                return hop(params->within);
        }

        const struct parse_tree *second; load_child_at(second, tree, 1);

        if (second->data.type == SYMBOL_TYPE){
                char *ret = malloc(sizeof(char));
                *ret = 0;
                return ret;
        }
        
        if (second->type->byte_count > 8){
                char *address = generate_address(second, params);
                return concat(2, address, hop(params->within));
        }

        char *value = generate_value(second, params);
        return concat(2, value, hop(params->within));
}

char *generate_binaryexpr(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 1){
                return generate_first_child(tree, params);
        }

        char *op1 = generate_value(tree->children->head->data, params);
        char *op2 = generate_value(tree->children->head->next->next->data, params);
        enum symbol op_type = tree->children->head->next->data->data.type;

        switch (op_type){
                case SYMBOL_OR:
                        return dsjtn(op1, op2);
                case SYMBOL_AND:
                        return cnjtn(op1, op2);
                case SYMBOL_EQ:
                        return eq(op1, op2);
                case SYMBOL_NE:
                        return ne(op1, op2);
                case SYMBOL_LT:
                        return lt(op1, op2);
                case SYMBOL_GT:
                        return gt(op1, op2);
                case SYMBOL_LE:
                        return le(op1, op2);
                case SYMBOL_GE:
                        return ge(op1, op2);
                case SYMBOL_BOR:
                        return bor(op1, op2);
                case SYMBOL_BXOR:
                        return bxor(op1, op2);
                case SYMBOL_AMP:
                        return band(op1, op2);
                case SYMBOL_BLEFT:
                        return bleft(op1, op2);
                case SYMBOL_BRIGHT:
                        return bright(op1, op2);
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
                default:
                        assert(0);
                        return NULL;
        }
}

char *generate_unaryexpr(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 1){
                return generate_first_child(tree, params);
        }

        enum symbol first = tree->children->head->data->data.type;

        if (first == SYMBOL_INC){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 1);
                char *find_memory_addr = generate_address(addressable, params);

                return concat(5,
                        find_memory_addr,
                        ldr(REG_SCRATCH2, REG_RESULT),
                        movi(REG_SCRATCH, 1),
                        add(REG_SCRATCH, REG_SCRATCH2, REG_SCRATCH),
                        set_bytes_addr(REG_SCRATCH, REG_RESULT, addressable->type->byte_count)
                );
        }

        if (first == SYMBOL_DEC){
                const struct parse_tree *addressable; load_child_at(addressable, tree, 1);
                char *find_memory_addr = generate_address(addressable, params);

                return concat(5,
                        find_memory_addr,
                        ldr(REG_SCRATCH2, REG_RESULT),
                        movi(REG_SCRATCH, 1),
                        sub(REG_SCRATCH, REG_SCRATCH2, REG_SCRATCH),
                        set_bytes_addr(REG_SCRATCH, REG_RESULT,addressable->type->byte_count)
                );
        }   

        if (first == SYMBOL_MINUS){
                return subtract(
                        movi(REG_RESULT, 0), 
                        generate_value(tree->children->head->next->data, params)
                );
        }

        if (first == SYMBOL_NOT){
                return ngtn(generate_value(tree->children->head->next->data, params));
        }

        if (first == SYMBOL_BNOT){
                char *op = generate_value(tree->children->head->next->data, params);
                return bnot(op);
        }

        if (first == SYMBOL_STAR){
                char *find_memory_address = generate_value(tree->children->head->next->data, params);
                return concat(2,
                        find_memory_address,
                        ldr(REG_RESULT, REG_RESULT)
                );
        }

        if (first == SYMBOL_AMP){
                return generate_address(tree->children->head->next->data, params);
        }

        assert(0);
        return NULL;
}

char *generate_memberexpr(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->children->len == 1){
                return generate_first_child(tree, params);
        }

        if (tree->children->head->next->data->data.type == SYMBOL_LBRACKET){
                char *address = generate_address(tree, params);
                return concat(3, 
                        address, 
                        mov(REG_SCRATCH, REG_RESULT),
                        get_bytes_addr(REG_RESULT, REG_SCRATCH, tree->type->byte_count) 
                );
        }

        if (tree->children->head->next->data->data.type == SYMBOL_DOT){
                char *address = generate_address(tree, params);
                return concat(3, 
                        address, 
                        mov(REG_SCRATCH, REG_RESULT),
                        get_bytes_addr(REG_RESULT, REG_SCRATCH, tree->type->byte_count) 
                );
        }

        assert(0);
        return NULL;
}

char *generate_baseexpr(const struct parse_tree *tree, struct codegen_params *params){
        enum symbol child_type = tree->children->head->data->data.type;

        if (child_type == SYMBOL_LPAREN){
                return generate_from_tree(tree->children->head->next->data, params);
        }

        if (child_type == SYMBOL_TRUE){
                return movi(REG_RESULT, 1);
        }

        if (child_type == SYMBOL_FALSE){
                return movi(REG_RESULT, 0);
        }

        if (child_type == SYMBOL_CALL){
                return generate_first_child(tree, params);
        }

        if (child_type == SYMBOL_CAST){
                return generate_first_child(tree, params);
        }

        if (child_type == SYMBOL_IDENTIFIER){
                char *address = generate_address(tree, params);
                return concat(3, 
                        address, 
                        mov(REG_SCRATCH, REG_RESULT),
                        get_bytes_addr(REG_RESULT, REG_SCRATCH,tree->type->byte_count) 
                );  
        }

        if (child_type == SYMBOL_CONSTANT){
                long long imm = strtoll(tree->children->head->data->data.value, NULL, 10);
                return movi(REG_RESULT, imm);
        }

        if (child_type == SYMBOL_CHARLIT){
                char *data = tree->children->head->data->data.value;
                long long imm = data[0];
                return movi(REG_RESULT, imm);
        }

        if (child_type == SYMBOL_ASM){
                const struct parse_tree *instr_tree; load_child_at(instr_tree, tree, 2);
                char *ret = malloc((strlen(instr_tree->data.value) + 1) * sizeof(char));
                strcpy(ret, instr_tree->data.value);
                return ret;
        }

        assert(0);
        return NULL;
}

char *generate_call(const struct parse_tree *tree, struct codegen_params *params){
        char *id = tree->children->head->data->data.value;
        struct string s;
        s.data = id;
        const struct function *f; query_map(params->functions, (&s), f, string, function);
        char **args_code = malloc(num_params(f) * sizeof(char*));

        size_t i = 0;
        
        const struct parse_tree *optargs = tree->children->head->next->next->data;
        if ((optargs->children != NULL) && (optargs->children->len != 0)){
                const struct parse_tree *args = optargs->children->head->data;

                for (struct LIST_NODE(parse_tree) *child = args->children->head; child != NULL; child = child->next ? child->next->next : NULL){
                        args_code[i++] = child->data->type->byte_count > 8
                                ? generate_address(child->data, params)
                                : generate_value(child->data, params);
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

char *generate_cast(const struct parse_tree *tree, struct codegen_params *params){
        const struct parse_tree *src_tree; 
        if (tree->children->len == 4){
                load_child_at(src_tree, tree, 2);
        }
        else {
                load_child_at(src_tree, tree, 3);
        }

        return generate_from_tree(src_tree, params);
}

char *generate_address(const struct parse_tree *tree, struct codegen_params *params){
        if (tree->data.type == SYMBOL_IDENTIFIER){
                struct variable var = find_symbol_variable(tree);
                return function_variable_address(params->within, var, REG_RESULT);
        }
        
        if (tree->data.type == SYMBOL_STRINGLIT){
                struct literal *l = malloc(sizeof(struct literal));
                l->value = tree->data.value;

                char *load = malloc(42 * sizeof(char));
                strcpy(load, "ldr x9, =__stringlit");
                char len[20];
                sprintf(len, "%u", params->literals.len);

                strcat(load, len);

                char *ret = concat(5,
                        literal("ldr x10, =__stringlittemp"),
                        load,
                        str(REG_SCRATCH, REG_RESULT),
                        movi(REG_SCRATCH, strlen(l->value)),
                        literal("str x9, [x10, #8]")
                );

                append_list((&params->literals), l, literal);
        
                return ret;
        }

        if (tree->children->len == 1){
                return generate_address(tree->children->head->data, params);
        }

        if (tree->data.type == SYMBOL_CALL){
                return generate_call(tree, params);
        }

        if (tree->data.type == SYMBOL_CAST){
                return generate_address(tree->children->head->next->next->data, params);
        }

        if (tree->children->head->data->data.type == SYMBOL_LPAREN){
                return generate_address(tree->children->head->next->data, params);
        }

        if (tree->children->head->data->data.type == SYMBOL_STAR){
                return generate_value(tree->children->head->next->data, params);
        }

        if (tree->children->head->next->data->data.type == SYMBOL_DOT){
                const struct parse_tree *record_tree; load_child_at(record_tree, tree, 0);
                const struct parse_tree *field_tree; load_child_at(field_tree, tree, 2);
                size_t field_offset = record_type_offset(record_tree->type, field_tree->data.value);

                char *record = generate_address(record_tree, params);

                return concat(3,
                        record,
                        movi(REG_SCRATCH, field_offset),
                        add(REG_RESULT, REG_RESULT, REG_SCRATCH)
                );
        }

        if (tree->children->head->next->data->data.type == SYMBOL_LBRACKET){
                const struct parse_tree *array_tree; load_child_at(array_tree, tree, 0);
                const struct parse_tree *index_tree; load_child_at(index_tree, tree, 2);
                uint32_t element_size = array_tree->type->element_type->byte_count;
                char *array_address = generate_address(array_tree, params);
                char *index = generate_value(index_tree, params);

                return concat(7,
                        array_address,
                        push(REG_RESULT),
                        index,
                        movi(REG_SCRATCH, element_size),
                        mul(REG_RESULT, REG_RESULT, REG_SCRATCH),
                        pop(REG_SCRATCH),
                        add(REG_RESULT, REG_RESULT, REG_SCRATCH)
                );
        }

        print_parse_tree(tree);
        assert(0);
        return NULL;
}

char *generate_value(const struct parse_tree *tree, struct codegen_params *params){
        assert(tree->type->byte_count <= 8);
        return generate_from_tree(tree, params);

        print_parse_tree(tree);
        assert(0);
        return NULL;
}

char *generate_from_tree(const struct parse_tree *tree, struct codegen_params *params){
        enum symbol symbol = tree->data.type;

        switch (symbol) {
                case SYMBOL_STMTS:
                        return generate_stmts(tree, params);
                case SYMBOL_STMT:
                        return generate_stmt(tree, params);
                case SYMBOL_IFSTMT:
                        return generate_ifstmt(tree, params);
                case SYMBOL_WHILESTMT:
                        return generate_whilestmt(tree, params);
                case SYMBOL_FORSTMT:
                        return generate_forstmt(tree, params);
                case SYMBOL_SEMISTMT:
                        return generate_first_child(tree, params);
                case SYMBOL_VARDEC:
                        return generate_vardec(tree, params);
                case SYMBOL_VARASST:
                        return generate_varasst(tree, params);
                case SYMBOL_RET:
                        return generate_ret(tree, params);
                case SYMBOL_EXPR:
                        return generate_first_child(tree, params);
                case SYMBOL_OREXPR:
                case SYMBOL_ANDEXPR:
                case SYMBOL_EQEXPR:
                case SYMBOL_COMPEXPR:
                case SYMBOL_BOREXPR:
                case SYMBOL_BXOREXPR:
                case SYMBOL_BANDEXPR:
                case SYMBOL_BSHIFTEXPR:
                case SYMBOL_ADDEXPR:
                case SYMBOL_MULTEXPR:
                        return generate_binaryexpr(tree, params);
                case SYMBOL_UNARYEXPR:
                        return generate_unaryexpr(tree, params);
                case SYMBOL_MEMBEREXPR:
                        return generate_memberexpr(tree, params);
                case SYMBOL_BASEEXPR:
                        return generate_baseexpr(tree, params);
                case SYMBOL_CALL:
                        return generate_call(tree, params);
                case SYMBOL_BODY:
                        return generate_first_child(tree, params);
                case SYMBOL_CAST:
                        return generate_cast(tree, params);
                default:
                        assert(0);
                        return NULL;
        }
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

                struct string *s = malloc(sizeof(struct string));
                const struct parse_tree *signature = defn->children->head->data;
                s->data = signature->children->head->next->data->data.value;
                bool is_main = strcmp("main", s->data) == 0;

                struct function *fn = new_function(params_list, locals_list, is_main, s->data);
                free_list((&locals_list), free_variable, variable);

                update_map((&functions), s, fn, string, function);
        }

        struct codegen_params params = {.functions = &functions};
        init_list((&params.literals));

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
                params.within = fn;
                set_body((struct function*) fn, generate_from_tree(stmts, &params));
        }

        // char *prog = malloc(44 * sizeof(char));
        // strcpy(prog, head);
        char *prog = generate_head(params.literals);
        
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

        char *sl = malloc(8 * sizeof(char));
        char *t = malloc(31 * sizeof(char));
        strcpy(sl, start_label);
        strcpy(t, tail);
        prog = concat(4, prog, sl, declare_function(main), t);
        free_map((&functions), string, function);
        free_list((&params.literals), free_literal, literal);
        return prog;
}
