#include "lang/symbol.h"

bool is_terminal(enum symbol s){
        return (s > SYMBOL_NULL) && (s <= SYMBOL_END);
}

char * symbol_name(enum symbol s){
        switch(s) {
                case SYMBOL_NULL: return "null";
                case SYMBOL_LPAREN: return "(";
                case SYMBOL_RPAREN: return ")";
                case SYMBOL_LBRACE: return "{";
                case SYMBOL_RBRACE: return "}";
                case SYMBOL_INC: return "++";
                case SYMBOL_DEC: return "--";
                case SYMBOL_AND: return "&&";
                case SYMBOL_OR: return "||";
                case SYMBOL_NOT: return "!";
                case SYMBOL_PLUS: return "+";
                case SYMBOL_MINUS: return "-";
                case SYMBOL_TIMES: return "*";
                case SYMBOL_DIVIDE: return "/";
                case SYMBOL_MOD: return "%";
                case SYMBOL_LT: return "<";
                case SYMBOL_GT: return ">";
                case SYMBOL_LE: return "<=";
                case SYMBOL_GE: return ">=";
                case SYMBOL_EQ: return "==";
                case SYMBOL_NE: return "!=";
                case SYMBOL_ASSIGN: return "=";
                case SYMBOL_COMMA: return ",";
                case SYMBOL_SEMICOLON: return ";";
                case SYMBOL_LBRACKET: return "[";
                case SYMBOL_RBRACKET: return "]";
                case SYMBOL_DOT: return ".";
                case SYMBOL_ELSE: return "else";
                case SYMBOL_FOR: return "for";
                case SYMBOL_HOP: return "hop";
                case SYMBOL_IF: return "if";
                case SYMBOL_INT: return "int";
                case SYMBOL_LET: return "let";
                case SYMBOL_VOID: return "void";
                case SYMBOL_WHILE: return "while";
                case SYMBOL_IDENTIFIER: return "ID";
                case SYMBOL_CONSTANT: return "CONST";
                case SYMBOL_END: return "END";
                case SYMBOL_PROGRAM: return "PROGRAM";
                case SYMBOL_DEFNDECLS: return "DEFNDECLS";
                case SYMBOL_DEFN: return "DEFN";
                case SYMBOL_TYPE: return "TYPE";
                case SYMBOL_OPTPARAMS: return "OPTPARAMS";
                case SYMBOL_STMTS: return "STMTS";
                case SYMBOL_PARAMS: return "PARAMS";
                case SYMBOL_PARAM: return "PARAM";
                case SYMBOL_STMT: return "STMT";
                case SYMBOL_SEMISTMT: return "SEMISTMT";
                case SYMBOL_EXPR: return "EXPR";
                case SYMBOL_ANDCOND: return "ANDCOND";
                case SYMBOL_ORCOND: return "ORCOND";
                case SYMBOL_UNCOND: return "UNCOND";
                case SYMBOL_OPTELSE: return "OPTELSE";
                case SYMBOL_ADDEXPR: return "ADDEXPR";
                case SYMBOL_MULTEXPR: return "MULTEXPR";
                case SYMBOL_UNEXPR: return "UNEXPR";
                case SYMBOL_OPTARGS: return "OPTARGS";
                case SYMBOL_ARGS: return "ARGS";
                case SYMBOL_VARDEC: return "VARDEC";
                case SYMBOL_VARASST: return "VARASST";
                case SYMBOL_RET: return "RET";
                case SYMBOL_COUNT: return "COUNT";
                case SYMBOL_BOOL: return "bool";
                case SYMBOL_CHAR: return "char";
                case SYMBOL_FALSE: return "false";
                case SYMBOL_TRUE: return "true";
                case SYMBOL_RECORD: return "record";
                case SYMBOL_CHARLIT: return "CHARLIT";
                case SYMBOL_CALL: return "CALL";
                case SYMBOL_STRINGLIT: return "STRINGLIT";
                case SYMBOL_ASM: return "asm";
                case SYMBOL_IFSTMT: return "IFSTMT";
                case SYMBOL_FORSTMT: return "FORSTMT";
                case SYMBOL_WHILESTMT: return "WHILESTMT";
                case SYMBOL_BODY: return "BODY";
                case SYMBOL_SIGNATURE: return "SIGNATURE";
                case SYMBOL_DECLARE: return "declare";
                case SYMBOL_DECL: return "DECL";
                case SYMBOL_DEFNDECL: return "DEFNDECL";
                case SYMBOL_ADDRESSABLE: return "ADDRESSABLE";
                case SYMBOL_FIELDS: return "FIELDS";
                case SYMBOL_FIELD: return "FIELD";
        }
        return 0;
}
