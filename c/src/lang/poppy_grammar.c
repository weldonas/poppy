#include "lang/poppy_grammar.h"
#include "lang/symbol.h"

#include <stdlib.h>
#include <string.h>

#define RULE_COUNT 110
#define COMMA ,
#define populate(lh_symbol, rh_symbols, ctr, grmr)                               \
        do {                                                                     \
                grmr->rules[ctr].lhs = lh_symbol;                                \
                enum symbol rhs[] = rh_symbols;                                  \
                grmr->rules[ctr].rhs = malloc(sizeof(rhs));       \
                grmr->rules[ctr].rhs_len = sizeof(rhs) / sizeof(enum symbol);    \
                memcpy(grmr->rules[ctr].rhs, rhs, sizeof(rhs));                  \
        } while (0)

struct grammar * poppy_grammar = NULL;

const struct grammar * const get_poppy_grammar(){
        if (poppy_grammar != NULL){
                return poppy_grammar;
        }

        poppy_grammar = malloc(sizeof(struct grammar));
        poppy_grammar->start = SYMBOL_PROGRAM;
        poppy_grammar->rules_len = RULE_COUNT;

        for (size_t i = 0; i < SYMBOL_COUNT; ++i){
                poppy_grammar->nullable[i] = false;
                poppy_grammar->expanded[i] = false;
        }

        poppy_grammar->rules = malloc(RULE_COUNT * sizeof(struct rule));

        int i = 0;
        populate(SYMBOL_PROGRAM, {SYMBOL_DEFNDECLS COMMA SYMBOL_END}, i, poppy_grammar); ++i;
        populate(SYMBOL_DEFNDECLS, {SYMBOL_DEFNDECL}, i, poppy_grammar); ++i;
        populate(SYMBOL_DEFNDECLS, {SYMBOL_DEFNDECL COMMA SYMBOL_DEFNDECLS}, i, poppy_grammar); ++i;
        populate(SYMBOL_DEFNDECL, {SYMBOL_FNDEFN}, i, poppy_grammar); ++i;
        populate(SYMBOL_DEFNDECL, {SYMBOL_RECDEFN}, i, poppy_grammar); ++i;
        populate(SYMBOL_DEFNDECL, {SYMBOL_DECL}, i, poppy_grammar); ++i;
        populate(SYMBOL_FNDEFN, {SYMBOL_SIGNATURE COMMA SYMBOL_LBRACE COMMA SYMBOL_BODY COMMA SYMBOL_RBRACE}, i, poppy_grammar); ++i;
        populate(SYMBOL_RECDEFN, {SYMBOL_RECORD COMMA SYMBOL_IDENTIFIER COMMA SYMBOL_LPAREN COMMA SYMBOL_FIELDS COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_DECL, {SYMBOL_DECLARE COMMA SYMBOL_SIGNATURE}, i, poppy_grammar); ++i;
        populate(SYMBOL_SIGNATURE, {SYMBOL_TYPE COMMA SYMBOL_IDENTIFIER COMMA SYMBOL_LPAREN COMMA SYMBOL_OPTPARAMS COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_INT}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_VOID}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_CHAR}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_BOOL}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_RECORD COMMA SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_TYPE COMMA SYMBOL_LBRACKET COMMA SYMBOL_CONSTANT COMMA SYMBOL_RBRACKET}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_AMP COMMA SYMBOL_TYPE}, i, poppy_grammar); ++i;
        populate(SYMBOL_TYPE, {SYMBOL_LPAREN COMMA SYMBOL_TYPE COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTPARAMS, {}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTPARAMS, {SYMBOL_PARAMS}, i, poppy_grammar); ++i;
        populate(SYMBOL_PARAMS, {SYMBOL_PARAM COMMA SYMBOL_COMMA COMMA SYMBOL_PARAMS}, i, poppy_grammar); ++i;
        populate(SYMBOL_PARAMS, {SYMBOL_PARAM}, i, poppy_grammar); ++i;
        populate(SYMBOL_PARAM, {SYMBOL_TYPE COMMA SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_FIELDS, {SYMBOL_FIELD COMMA SYMBOL_COMMA COMMA SYMBOL_FIELDS}, i, poppy_grammar); ++i;
        populate(SYMBOL_FIELDS, {SYMBOL_FIELD}, i, poppy_grammar); ++i;
        populate(SYMBOL_FIELD, {SYMBOL_TYPE COMMA SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMTS, {SYMBOL_STMT}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMTS, {SYMBOL_STMT COMMA SYMBOL_STMTS}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMT, {SYMBOL_SEMISTMT COMMA SYMBOL_SEMICOLON}, i, poppy_grammar); ++i;
        populate(SYMBOL_SEMISTMT, {SYMBOL_VARDEC}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARDEC, {SYMBOL_LET COMMA SYMBOL_TYPE COMMA SYMBOL_IDENTIFIER COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARDEC, {SYMBOL_LET COMMA SYMBOL_TYPE COMMA SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_SEMISTMT, {SYMBOL_VARASST}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_PLUS COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_MINUS COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_STAR COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_DIVIDE COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_MOD COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_BLEFT COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_BRIGHT COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_AMP COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_BXOR COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_VARASST, {SYMBOL_EXPR COMMA SYMBOL_BOR COMMA SYMBOL_ASSIGN COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_SEMISTMT, {SYMBOL_RET}, i, poppy_grammar); ++i;
        populate(SYMBOL_RET, {SYMBOL_HOP COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_RET, {SYMBOL_HOP}, i, poppy_grammar); ++i;
        populate(SYMBOL_RET, {SYMBOL_HOP COMMA SYMBOL_TYPE}, i, poppy_grammar); ++i;
        populate(SYMBOL_SEMISTMT, {SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMT, {SYMBOL_IFSTMT}, i, poppy_grammar); ++i;
        populate(SYMBOL_IFSTMT, {SYMBOL_IF COMMA SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN COMMA SYMBOL_LBRACE COMMA SYMBOL_BODY COMMA SYMBOL_RBRACE COMMA SYMBOL_OPTELSE }, i, poppy_grammar); ++i;
        populate(SYMBOL_BODY, {SYMBOL_STMTS}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTELSE, {}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTELSE, {SYMBOL_ELSE COMMA SYMBOL_LBRACE COMMA SYMBOL_STMTS COMMA SYMBOL_RBRACE}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMT, {SYMBOL_WHILESTMT}, i, poppy_grammar); ++i;
        populate(SYMBOL_WHILESTMT, {SYMBOL_WHILE COMMA SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN COMMA SYMBOL_LBRACE COMMA SYMBOL_BODY COMMA SYMBOL_RBRACE}, i, poppy_grammar); ++i;
        populate(SYMBOL_STMT, {SYMBOL_FORSTMT}, i, poppy_grammar); ++i;
        populate(SYMBOL_FORSTMT, {SYMBOL_FOR COMMA SYMBOL_LPAREN COMMA SYMBOL_SEMISTMT COMMA SYMBOL_SEMICOLON COMMA SYMBOL_EXPR COMMA SYMBOL_SEMICOLON COMMA SYMBOL_SEMISTMT COMMA SYMBOL_RPAREN COMMA SYMBOL_LBRACE COMMA SYMBOL_STMTS COMMA SYMBOL_RBRACE}, i, poppy_grammar); ++i;
        populate(SYMBOL_EXPR, {SYMBOL_ORCOND}, i, poppy_grammar); ++i;
        populate(SYMBOL_ORCOND, {SYMBOL_ORCOND COMMA SYMBOL_OR COMMA SYMBOL_ANDCOND}, i, poppy_grammar); ++i;
        populate(SYMBOL_ORCOND, {SYMBOL_ANDCOND}, i, poppy_grammar); ++i;
        populate(SYMBOL_ANDCOND, {SYMBOL_ANDCOND COMMA SYMBOL_AND COMMA SYMBOL_UNCOND}, i, poppy_grammar); ++i;
        populate(SYMBOL_ANDCOND, {SYMBOL_UNCOND}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_NOT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_LT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_GT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_LE COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_GE COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_EQ COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_EXPR COMMA SYMBOL_NE COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_TRUE}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_FALSE}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNCOND, {SYMBOL_CALL}, i, poppy_grammar); ++i;
        populate(SYMBOL_EXPR, {SYMBOL_ADDEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_ADDEXPR, {SYMBOL_ADDEXPR COMMA SYMBOL_PLUS COMMA SYMBOL_MULTEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_ADDEXPR, {SYMBOL_ADDEXPR COMMA SYMBOL_MINUS COMMA SYMBOL_MULTEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_ADDEXPR, {SYMBOL_MULTEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_MULTEXPR, {SYMBOL_MULTEXPR COMMA SYMBOL_STAR COMMA SYMBOL_UNEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_MULTEXPR, {SYMBOL_MULTEXPR COMMA SYMBOL_DIVIDE COMMA SYMBOL_UNEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_MULTEXPR, {SYMBOL_MULTEXPR COMMA SYMBOL_MOD COMMA SYMBOL_UNEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_MULTEXPR, {SYMBOL_UNEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_MINUS COMMA SYMBOL_UNEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_CALL}, i, poppy_grammar); ++i;
        populate(SYMBOL_CALL, {SYMBOL_IDENTIFIER COMMA SYMBOL_LPAREN COMMA SYMBOL_OPTARGS COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTARGS, {}, i, poppy_grammar); ++i;
        populate(SYMBOL_OPTARGS, {SYMBOL_ARGS}, i, poppy_grammar); ++i;
        populate(SYMBOL_ARGS, {SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_ARGS, {SYMBOL_EXPR COMMA SYMBOL_COMMA COMMA SYMBOL_ARGS}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_CAST}, i, poppy_grammar); ++i;
        populate(SYMBOL_CAST, {SYMBOL_TYPE COMMA SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_CAST, {SYMBOL_UNSAFE COMMA SYMBOL_TYPE COMMA SYMBOL_LPAREN COMMA SYMBOL_EXPR COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_EXPR COMMA SYMBOL_LBRACKET COMMA SYMBOL_EXPR COMMA SYMBOL_RBRACKET}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_EXPR COMMA SYMBOL_DOT COMMA SYMBOL_IDENTIFIER}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_AMP COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_BITEXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_EXPR COMMA SYMBOL_AMP COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_EXPR COMMA SYMBOL_BOR COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_EXPR COMMA SYMBOL_BXOR COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_EXPR COMMA SYMBOL_BLEFT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_EXPR COMMA SYMBOL_BRIGHT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_BITEXPR, {SYMBOL_BNOT COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_STAR COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_INC COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_DEC COMMA SYMBOL_EXPR}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_CONSTANT}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_CHARLIT}, i, poppy_grammar); ++i;
        populate(SYMBOL_UNEXPR, {SYMBOL_ASM COMMA SYMBOL_LPAREN COMMA SYMBOL_STRINGLIT COMMA SYMBOL_RPAREN}, i, poppy_grammar); ++i;

        // this assumes a symbol is nullable if and only if it's the LHS of a rule with an empty RHS
        for (size_t i = 0; i < RULE_COUNT; ++i){
                if (poppy_grammar->rules[i].rhs_len == 0){
                        poppy_grammar->nullable[poppy_grammar->rules[i].lhs] = true;
                }
        }

        poppy_grammar->expanded[SYMBOL_DEFNDECLS] = true;
        poppy_grammar->expanded[SYMBOL_PARAMS] = true;
        poppy_grammar->expanded[SYMBOL_STMTS] = true;
        poppy_grammar->expanded[SYMBOL_ARGS] = true;
        poppy_grammar->expanded[SYMBOL_FIELDS] = true;

        return poppy_grammar;
}

void free_poppy_grammar(){
        if(poppy_grammar == NULL){
                return;
        }

        for(int i = 0; i < poppy_grammar->rules_len; ++i){
                free(poppy_grammar->rules[i].rhs);
        }

        free(poppy_grammar->rules);
        free(poppy_grammar);
        poppy_grammar = NULL;
}
