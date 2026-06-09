#ifndef PARSER_H
#define PARSER_H

#include "data/list.h"
#include "lang/grammar.h"
#include "lang/lexer.h"
#include "lang/parse_tree.h"

const struct parse_tree * const parse(const struct grammar *grammar, const struct LIST(token) *head);

#endif
