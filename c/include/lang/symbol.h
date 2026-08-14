#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>
#include <stdint.h>

enum symbol : uint8_t {
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
        SYMBOL_STAR,       // *
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
        SYMBOL_LBRACKET,   // [
        SYMBOL_RBRACKET,   // ]
        SYMBOL_DOT,        // .
        SYMBOL_AMP,        // &
        SYMBOL_BOR,        // |
        SYMBOL_BXOR,       // ^
        SYMBOL_BLEFT,      // <<
        SYMBOL_BRIGHT,     // >>
        SYMBOL_BNOT,       // ~
        SYMBOL_ASM,
        SYMBOL_BOOL,
        SYMBOL_CHAR,
        SYMBOL_ELSE,
        SYMBOL_ENUM,
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
        SYMBOL_DECLARE,
        SYMBOL_RECORD,
        SYMBOL_UNSAFE,
        SYMBOL_SAFE,
        SYMBOL_END,         // end of input
        // non-terminal symbols
        SYMBOL_PROGRAM,
        SYMBOL_DEFNDECLS,
        SYMBOL_FNDEFN,
        SYMBOL_TYPE,
        SYMBOL_OPTPARAMS,
        SYMBOL_STMTS,
        SYMBOL_PARAMS,
        SYMBOL_PARAM,
        SYMBOL_STMT,
        SYMBOL_SEMISTMT,
        SYMBOL_EXPR,
        SYMBOL_OREXPR,
        SYMBOL_ANDEXPR,
        SYMBOL_EQEXPR,
        SYMBOL_COMPEXPR,
        SYMBOL_BOREXPR,
        SYMBOL_BXOREXPR,
        SYMBOL_BANDEXPR,
        SYMBOL_BSHIFTEXPR,
        SYMBOL_ADDEXPR,
        SYMBOL_MULTEXPR,
        SYMBOL_UNARYEXPR,
        SYMBOL_MEMBEREXPR,
        SYMBOL_BASEEXPR,
        SYMBOL_OPTELSE,
        SYMBOL_OPTARGS,
        SYMBOL_ARGS,
        SYMBOL_VARDEC,
        SYMBOL_VARASST,
        SYMBOL_RET,
        SYMBOL_CALL,
        SYMBOL_IFSTMT,
        SYMBOL_WHILESTMT,
        SYMBOL_FORSTMT,
        SYMBOL_BODY,
        SYMBOL_SIGNATURE,
        SYMBOL_DECL,
        SYMBOL_DEFNDECL,
        SYMBOL_FIELDS,
        SYMBOL_FIELD,
        SYMBOL_RECDEFN,
        SYMBOL_CAST,
        SYMBOL_GLOBALVARDEC,
        SYMBOL_ENUMDEFN,
        SYMBOL_ENUMITEMS,
        SYMBOL_ENUMITEM,
        SYMBOL_COUNT
};

bool is_terminal(enum symbol s);
char * symbol_name(enum symbol s);

#endif
