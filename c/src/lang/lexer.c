#include "lang/lexer.h"
#include "data/result.h"
#include "lang/symbol.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_LENGTH 127

bool is_numeric_char(char c){
        return (c >= '0') && (c <= '9');
}

bool is_alphabetic(char c){
        return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) || (c == '_');
}

struct lex_data {
        enum symbol type;
        char excess;
        int val_len;
};

struct character_symbol_map {
        char expected;
        enum symbol if_expected;
};

struct lex_data find_prefixed_type(FILE* file, struct character_symbol_map *map, size_t map_len, enum symbol not_second, char *val){
        struct lex_data ret;
        ret.excess = 0;
        ret.val_len = 1;
        ret.type = not_second;

        if (feof(file)){
                return ret;
        }
        
        char second = fgetc(file);
        for (size_t i = 0; i < map_len; ++i){
                if (second == map[i].expected){
                        val[1] = second;
                        ret.val_len = 2;
                        ret.type = map[i].if_expected;

                        return ret;
                }
        }

        ret.excess = second;        
        return ret;

}

struct lex_data find_numeric_value (FILE* file, char *val){
        struct lex_data ret;
        ret.type = SYMBOL_CONSTANT;
        ret.excess = 0;
        ret.val_len = 1;
        
        while (!feof(file) && (ret.val_len <= MAX_TOKEN_LENGTH) && is_numeric_char(val[ret.val_len - 1])){
                val[ret.val_len] = fgetc(file);
                ++ret.val_len;
        }

        if (!feof(file)){
                --ret.val_len;
                ret.excess = val[ret.val_len];                        
        }

        return ret;
}

struct lex_data find_alphanumeric_value (FILE* file, char *val){
        struct lex_data ret;
        ret.excess = 0;
        ret.val_len = 1;
        
        while (!feof(file) && (ret.val_len <= MAX_TOKEN_LENGTH) && (is_numeric_char(val[ret.val_len - 1]) || is_alphabetic(val[ret.val_len - 1]))){
                val[ret.val_len] = fgetc(file);
                ++ret.val_len;
        }

        if (!feof(file)){
                --ret.val_len;
                ret.excess = val[ret.val_len];                        
        }
        if ((ret.val_len == 3) && (strncmp(val, "asm", 3) == 0)){
                ret.type = SYMBOL_ASM;
        } else if ((ret.val_len == 4) && (strncmp(val, "bool", 4) == 0)){
                ret.type = SYMBOL_BOOL;
        } else if ((ret.val_len == 4) && (strncmp(val, "char", 4) == 0)){
                ret.type = SYMBOL_CHAR;
        } else if ((ret.val_len == 7) && (strncmp(val, "declare", 7) == 0)){
                ret.type = SYMBOL_DECLARE;
        } else if ((ret.val_len == 4) && (strncmp(val, "else", 4) == 0)){
                ret.type = SYMBOL_ELSE;
        } else if ((ret.val_len == 4) && (strncmp(val, "enum", 4) == 0)){
                ret.type = SYMBOL_ENUM;
        } else if ((ret.val_len == 5) && (strncmp(val, "false", 5) == 0)){
                ret.type = SYMBOL_FALSE;
        } else if ((ret.val_len == 3) && (strncmp(val, "for", 3) == 0)){
                ret.type = SYMBOL_FOR;
        } else if ((ret.val_len == 3) && (strncmp(val, "hop", 3) == 0)){
                ret.type = SYMBOL_HOP;
        } else if ((ret.val_len == 2) && (strncmp(val, "if", 2) == 0)){
                ret.type = SYMBOL_IF;
        } else if ((ret.val_len == 3) && (strncmp(val, "int", 3) == 0)){
                ret.type = SYMBOL_INT;
        } else if ((ret.val_len == 3) && (strncmp(val, "let", 3) == 0)){
                ret.type = SYMBOL_LET;
        } else if ((ret.val_len == 6) && (strncmp(val, "record", 6) == 0)){
                ret.type = SYMBOL_RECORD;
        } else if ((ret.val_len == 4) && (strncmp(val, "safe", 4) == 0)){
                ret.type = SYMBOL_SAFE;
        } else if ((ret.val_len == 4) && (strncmp(val, "true", 4) == 0)){
                ret.type = SYMBOL_TRUE;
        } else if ((ret.val_len == 6) && (strncmp(val, "unsafe", 6) == 0)){
                ret.type = SYMBOL_UNSAFE;
        } else if ((ret.val_len == 4) && (strncmp(val, "void", 4) == 0)){
                ret.type = SYMBOL_VOID;
        } else if ((ret.val_len == 5) && (strncmp(val, "while", 3) == 0)){
                ret.type = SYMBOL_WHILE;
        } else {
                ret.type = SYMBOL_IDENTIFIER;
        }

        return ret;
}

bool lex_single_char(FILE *file, char *chr, bool *escaped){
        // precondition: !feof(file)
        if (escaped){
                *escaped = false;
        }

        *chr = fgetc(file);

        if (*chr == '\\'){
                if (feof(file)){
                        return false;
                }

                if (escaped){
                        *escaped = true;
                }

                switch(fgetc(file)){
                        case 'a':
                                *chr = '\a';
                                return true;
                        case 'b':
                                *chr = '\b';
                                return true;
                        case 'f':
                                *chr = '\f';
                                return true;
                        case 'n':
                                *chr = '\n';
                                return true;
                        case 'r':
                                *chr = '\r';
                                return true;
                        case 't':
                                *chr = '\t';
                                return true;
                        case 'v':
                                *chr = '\v';
                                return true;
                        case '\\':
                                *chr = '\\';
                                return true;
                        case '\'':
                                *chr = '\'';
                                return true;
                        case '\"':
                                *chr = '\"';
                                return true;
                        default:
                                return false;
                }
        }

        return true;
}

/*
match escape sequences to the chars they escape by editing val[0]
return true if the lex is still valid
*/
bool lex_char_literal(FILE *file, char val[MAX_TOKEN_LENGTH + 1]){
        // ensure we have char after opening single quote
        if (feof(file)){
                return false;
        }

        // ensure any escape sequences are valid
        if (!lex_single_char(file, val, NULL)){
                return false;
        }

        // ensure we have a closing quote
        if (feof(file) || (fgetc(file) != '\'')){
                return false;
        }

        return true;
}

bool lex_string_literal(FILE *file, char val[MAX_TOKEN_LENGTH + 1]){
        // ensure we have char after opening single quote
        if (feof(file)){
                return false;
        }
        
        size_t i = 0;
        bool escaped;
        do {
                if (!lex_single_char(file, val + i, &escaped)){
                        return false;
                }
                ++i;
        } while (((val[i - 1] != '\"') || escaped) && (i - 1 <= MAX_TOKEN_LENGTH));
        if (i - 1 > MAX_TOKEN_LENGTH){
                return false;
        }

        val[i - 1] = 0;
        return true;
}


struct RESULT(token_list) lex(FILE *file){
        char val[MAX_TOKEN_LENGTH + 1];
        val[0] = 0;
        struct LIST(token) *list = malloc(sizeof(struct LIST(token)));
        init_list(list);
        struct RESULT(token_list) result;
        make_ok(result, list);

        while (!feof(file)){
                if (val[0] == 0){
                        val[0] = fgetc(file);
                }

                struct lex_data data;
                data.excess = 0;

                if (isspace(val[0])){
                        val[0] = 0;
                        continue;
                }

                struct character_symbol_map map[2];

                switch (val[0]){
                        case -1:
                                data.type = SYMBOL_END;
                                data.val_len = 0;
                                break;
                        case '(':
                                data.type = SYMBOL_LPAREN;
                                data.val_len = 1;
                                break;
                        case ')':
                                data.type = SYMBOL_RPAREN;
                                data.val_len = 1;
                                break;
                        case '{':
                                data.type = SYMBOL_LBRACE;
                                data.val_len = 1;
                                break;
                        case '}':
                                data.type = SYMBOL_RBRACE;
                                data.val_len = 1;
                                break;
                        case '*':
                                data.type = SYMBOL_STAR;
                                data.val_len = 1;
                                break;
                        case '/':
                                data.type = SYMBOL_DIVIDE;
                                data.val_len = 1;
                                break;
                        case '%':
                                data.type = SYMBOL_MOD;
                                data.val_len = 1;
                                break;
                        case ',':
                                data.type = SYMBOL_COMMA;
                                data.val_len = 1;
                                break;
                        case ';':
                                data.type = SYMBOL_SEMICOLON;
                                data.val_len = 1;
                                break;
                        case '[':
                                data.type = SYMBOL_LBRACKET;
                                data.val_len = 1;
                                break;
                        case ']':
                                data.type = SYMBOL_RBRACKET;
                                data.val_len = 1;
                                break;
                        case '^':
                                data.type = SYMBOL_BXOR;
                                data.val_len = 1;
                                break;
                        case '~':
                                data.type = SYMBOL_BNOT;
                                data.val_len = 1;
                                break;
                        case '\'':
                                data.val_len = 1;
                                if(lex_char_literal(file, val)){
                                        data.type = SYMBOL_CHARLIT;
                                } else {
                                        make_error_lit(result, "Invalid character literal");
                                }
                                break;
                        case '\"':
                                if (lex_string_literal(file, val)){
                                        data.type = SYMBOL_STRINGLIT;
                                        data.val_len = strlen(val);
                                } else {
                                        make_error_lit(result, "Invalid string literal");
                                }
                                break;
                        case '+':
                                map[0] = (struct character_symbol_map){.expected = '+', .if_expected = SYMBOL_INC};
                                data = find_prefixed_type(file, map, 1, SYMBOL_PLUS, val);
                                break;
                        case '-':
                                map[0] = (struct character_symbol_map){.expected = '-', .if_expected = SYMBOL_DEC};
                                data = find_prefixed_type(file, map, 1, SYMBOL_MINUS, val);
                                break;

                        case '&':
                                map[0] = (struct character_symbol_map){.expected = '&', .if_expected = SYMBOL_AND};
                                data = find_prefixed_type(file, map, 1, SYMBOL_AMP, val);
                                break;

                        case '|':
                                map[0] = (struct character_symbol_map){.expected = '|', .if_expected = SYMBOL_OR};
                                data = find_prefixed_type(file, map, 1, SYMBOL_BOR, val);
                                break;

                        case '!':
                                map[0] = (struct character_symbol_map){.expected = '=', .if_expected = SYMBOL_NE};
                                data = find_prefixed_type(file, map, 1, SYMBOL_NOT, val);
                                break;

                        case '>':
                                map[0] = (struct character_symbol_map){.expected = '=', .if_expected = SYMBOL_GE};
                                map[1] = (struct character_symbol_map){.expected = '>', .if_expected = SYMBOL_BRIGHT};
                                data = find_prefixed_type(file, map, 2, SYMBOL_GT, val);
                                break;

                        case '<':
                                map[0] = (struct character_symbol_map){.expected = '=', .if_expected = SYMBOL_LE};
                                map[1] = (struct character_symbol_map){.expected = '<', .if_expected = SYMBOL_BLEFT};
                                data = find_prefixed_type(file, map, 2, SYMBOL_LT, val);
                                break;

                        case '=':
                                map[0] = (struct character_symbol_map){.expected = '=', .if_expected = SYMBOL_EQ};
                                data = find_prefixed_type(file, map, 1, SYMBOL_ASSIGN, val);
                                break;
                        case '.':
                                map[0] = (struct character_symbol_map){.expected = '.', .if_expected = SYMBOL_DOTDOT};
                                data = find_prefixed_type(file, map, 1, SYMBOL_DOT, val);
                                break;
                        default:
                                if (is_alphabetic(val[0])){
                                        data = find_alphanumeric_value(file, val);
                                } else if (is_numeric_char(val[0])){
                                        data = find_numeric_value(file, val);
                                } else {
                                        char *lit = "Unrecognized character ";
                                        char *err = malloc((strlen(lit) + 2) * sizeof(char));
                                        strcpy(err, lit);
                                        strcat(err, val);
                                        make_error(result, err);
                                }
                }

                if (!result.is_ok){
                        free_list(list, free_token, token);
                        free(list);
                        return result;
                }

                struct token *new_token = malloc(sizeof(struct token));

                new_token->value = malloc((data.val_len + 1) * sizeof(char));
                val[data.val_len] = 0;
                strcpy(new_token->value, val);
                new_token->type = data.type;
                append_list(list, new_token, token);

                val[0] = data.excess;
                data.excess = 0;
        }

        make_ok(result, list);
        return result;
}

void free_token(struct token *t){
        free(t->value);
        free(t);
}
