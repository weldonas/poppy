#ifndef BINOPS_H
#define BINOPS_H

#include "codegen/chunk.h"

char *sum(char *op1, char *op2);
char *subtract(char *op1, char *op2);
char *multiply(char *op1, char *op2);
char *divide(char *op1, char *op2);
char *modulo(char *op1, char *op2);
char *eq(char *op1, char *op2);
char *ne(char *op1, char *op2);
char *lt(char *op1, char *op2);
char *le(char *op1, char *op2);
char *gt(char *op1, char *op2);
char *ge(char *op1, char *op2);
char *cnjtn(char *op1, char *op2); // conjunction (AND)
char *dsjtn( char *op1, char *op2); // disjunction (OR)
char *ngtn(char *op); // negation (NOT)
char *band(char *op1, char *op2);
char *bor(char *op1, char *op2);
char *bxor(char *op1, char *op2);
char *bleft(char *op1, char *op2);
char *bright(char *op1, char *op2);
char *bnot(char *op);
#endif
