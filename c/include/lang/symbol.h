#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

enum symbol {
        SYMBOL_NULL,
        // terminal symbols
        SYMBOL_LPAREN,     // (
        SYMBOL_RPAREN,     // )
        SYMBOL_LBRACE,     // {
        SYMBOL_RBRACE,     // }
        SYMBOL_INC,        // ++
        SYMBOL_DEC,        // --
        SYMBOL_AND,        // &&
        SYMBOL_OR,         // ||
        SYMBOL_NOT,        // !
        SYMBOL_PLUS,       // +
        SYMBOL_MINUS,      // -
        SYMBOL_TIMES,      // *
        SYMBOL_DIVIDE,     // /
        SYMBOL_MOD,        // %
        SYMBOL_LT,         // <
        SYMBOL_GT,         // >
        SYMBOL_LE,         // <=
        SYMBOL_GE,         // >=
        SYMBOL_EQ,         // ==
        SYMBOL_NE,         // !=
        SYMBOL_ASSIGN,     // =
        SYMBOL_COMMA,      // ,
        SYMBOL_SEMICOLON,  // ;
        SYMBOL_ASM,
        SYMBOL_BOOL,
        SYMBOL_CHAR,
        SYMBOL_ELSE,
        SYMBOL_FALSE,
        SYMBOL_FOR,
        SYMBOL_HOP,
        SYMBOL_IF,
        SYMBOL_INT,
        SYMBOL_LET,
        SYMBOL_TRUE,
        SYMBOL_VOID,
        SYMBOL_WHILE,
        SYMBOL_IDENTIFIER,
        SYMBOL_CONSTANT,
        SYMBOL_CHARLIT,
        SYMBOL_STRINGLIT,
        SYMBOL_END,         // end of input
        // non-terminal symbols
        SYMBOL_PROGRAM,
        SYMBOL_DEFNS,
        SYMBOL_DEFN,
        SYMBOL_TYPE,
        SYMBOL_OPTPARAMS,
        SYMBOL_STMTS,
        SYMBOL_PARAMS,
        SYMBOL_PARAM,
        SYMBOL_STMT,
        SYMBOL_SEMISTMT,
        SYMBOL_EXPR,
        SYMBOL_ANDCOND,
        SYMBOL_ORCOND,
        SYMBOL_UNCOND,
        SYMBOL_OPTELSE,
        SYMBOL_ADDEXPR,
        SYMBOL_MULTEXPR,
        SYMBOL_UNEXPR,
        SYMBOL_OPTARGS,
        SYMBOL_ARGS,
        SYMBOL_VARDEC,
        SYMBOL_VARASST,
        SYMBOL_RET,
        SYMBOL_CALL,
        SYMBOL_IFSTMT,
        SYMBOL_WHILESTMT,
        SYMBOL_FORSTMT,
        SYMBOL_IFBODY,
        SYMBOL_COUNT
};

bool is_terminal(enum symbol s);
char * symbol_name(enum symbol s);

#endif
