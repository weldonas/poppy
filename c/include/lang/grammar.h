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
    bool nullable[SYMBOL_COUNT];    // if this symbol can be null
    bool expanded[SYMBOL_COUNT];    // if instances of this symbol can be expanded to be on one level
    bool skippable[SYMBOL_COUNT];   // if this symbol can be skipped over in the tree if it only has one child
    enum symbol start;
    uint8_t rules_len;
};

#endif