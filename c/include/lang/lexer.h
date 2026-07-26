#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#include "data/list.h"
#include "data/result.h"
#include "lang/symbol.h"

struct token {
        enum symbol type;
        char *value;
};

void free_token(struct token *t);

DEFINE_LIST(token);
DEFINE_RESULT(token_list);

struct RESULT(token_list) lex(FILE *file);

#endif