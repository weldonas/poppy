#ifndef ASSEM_H
#define ASSEM_H

#include <stddef.h>
#include <stdint.h>
#include "codegen/register.h"

char *mov(enum reg dest, enum reg src);
char *movi(enum reg dest, long long imm);
char *push(enum reg reg);
char *push_pair(enum reg reg1, enum reg reg2);
char *pop(enum reg reg);
char *pop_pair(enum reg reg1, enum reg reg2);
char *add(enum reg dest, enum reg src1, enum reg src2);
char *sub(enum reg dest, enum reg src1, enum reg src2);
char *mul(enum reg dest, enum reg src1, enum reg src2);
char *sdiv(enum reg dest, enum reg src1, enum reg src2);
char *msub(enum reg dest, enum reg mplcand, enum reg mplier, enum reg mend);
char *cmp(enum reg reg1, enum reg reg2);
char *cset(enum reg reg, char cond[3]);
char *and(enum reg dest, enum reg src1, enum reg src2);
char *orr(enum reg dest, enum reg src1, enum reg src2);
char *eor(enum reg dest, enum reg src1, enum reg src2);
char *lsl(enum reg dest, enum reg src1, enum reg src2);
char *lsr(enum reg dest, enum reg src1, enum reg src2);
char *mvn(enum reg dest, enum reg src);
char *not(enum reg dest, enum reg src);
char *cmpi(enum reg reg, long long imm);
char *ldr(enum reg dest, enum reg addr);
char *str(enum reg src, enum reg addr);
char *memory_copy(enum reg src, enum reg dest, long long bytes);
// extract selected bytes from src into the LSBs of dest
char *get_bytes(enum reg dest, enum reg src, uint8_t starting_byte, uint8_t count);
// put selected bytes from src into the specified location in dest (while preserving the rest of dest)
char *set_bytes(enum reg dest, enum reg src, uint8_t starting_byte, uint8_t count);
// extract selected bytes from value at addr into the LSBs of dest
char *get_bytes_addr(enum reg dest, enum reg addr, uint8_t starting_byte, uint8_t count);
// put selected bytes from src into the specified location in the value at addr (while preserving the rest)
char *set_bytes_addr(enum reg src, enum reg addr, uint8_t starting_byte, uint8_t count);
char *ret();
char *comment(char *text);
char *literal(char *text);
char *include(char *module);
char *concat(size_t count, ...);
#endif
