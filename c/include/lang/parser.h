#ifndef PARSER_H
#define PARSER_H

#include "data/list.h"
#include "data/result.h"
#include "lang/grammar.h"
#include "lang/lexer.h"
#include "lang/parse_tree.h"

DEFINE_RESULT(parse_tree);

struct RESULT(parse_tree) parse(const struct grammar *grammar, const struct LIST(token) *head);

#endif
