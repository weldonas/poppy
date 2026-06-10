#ifndef PROGRAM_H
#define PROGRAM_H

#include "lang/parser.h"
#include "lang/type_system.h"

char *generate_code(const struct parse_tree *tree);

#endif
