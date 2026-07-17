#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "lang/symbol.h"

struct rule {
    enum symbol *rhs;
    enum symbol lhs;
    uint8_t rhs_len;
};

struct grammar {
    struct rule *rules;
    bool nullable[SYMBOL_COUNT];
    bool expanded[SYMBOL_COUNT];
    enum symbol start;
    uint8_t rules_len;
};

#endif