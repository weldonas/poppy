#include "codegen/assem.h"

#include "assert.h"
#include "codegen/register.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


char *mov(enum reg dest, enum reg src){
        char *instr = (char*) malloc(13 * sizeof(char));
        strcpy(instr, "mov ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src));
        return instr;
}

char *movi_small(enum reg dest, long long imm){
        assert((-65537 <= imm) && (imm <= 65535));
        char *instr = (char*) malloc(17 * sizeof(char));
        strcpy(instr, "mov ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", #");
        char imm_str[7];
        sprintf(imm_str, "%lli", imm);
        strcat(instr, imm_str);
        return instr;
}

char *movi(enum reg dest, long long imm){
        // this instruction needs to support 64-bit immediate values
        // ARM only supports 16-bit, so we recursively split the immediate value

        if ((imm <= 65535) && (imm >= -65537)){
                return movi_small(dest, imm);
        }

        long long upper = imm >> 12;
        long long lower = imm & 0xFFF;

        char *mov_upper = movi(dest, upper);

        char *lsl = (char*) malloc(18 * sizeof(char));
        strcpy(lsl, "lsl ");
        strcat(lsl, reg_to_string(dest));
        strcat(lsl, ", ");
        strcat(lsl, reg_to_string(dest));
        strcat(lsl, ", #12");

        char *addi = (char*) malloc(22 * sizeof(char));
        char imm_str[7];
        sprintf(imm_str, "%lli", lower);

        strcpy(addi, "add ");
        strcat(addi, reg_to_string(dest));
        strcat(addi, ", ");
        strcat(addi, reg_to_string(dest));
        strcat(addi, ", #");
        strcat(addi, imm_str);
        
        return concat(3, 
                mov_upper,
                lsl,
                addi
        );
}

char *push(enum reg reg){
        char *instr = (char*) malloc(21 * sizeof(char));
        strcpy(instr, "str ");
        strcat(instr, reg_to_string(reg));
        strcat(instr, ", [sp, #-16]!");
        return instr;
}

char *push_pair(enum reg reg1, enum reg reg2){
        char *instr = (char*) malloc(26 * sizeof(char));
        strcpy(instr, "stp ");
        strcat(instr, reg_to_string(reg1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(reg2));
        strcat(instr, ", [sp, #-16]!");
        return instr;
}

char *pop(enum reg reg){
        char *instr = (char*) malloc(19 * sizeof(char));
        strcpy(instr, "ldr ");
        strcat(instr, reg_to_string(reg));
        strcat(instr, ", [sp], #16");
        return instr;  
}

char *pop_pair(enum reg reg1, enum reg reg2){
        char *instr = (char*) malloc(24 * sizeof(char));
        strcpy(instr, "ldp ");
        strcat(instr, reg_to_string(reg1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(reg2));
        strcat(instr, ", [sp], #16");
        return instr;
}

char *add(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "add ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *sub(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "sub ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *mul(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "mul ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *sdiv(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(19 * sizeof(char));
        strcpy(instr, "sdiv ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *msub(enum reg dest, enum reg mplcand, enum reg mplier, enum reg mnend){
        char *instr = (char*) malloc(24 * sizeof(char));
        strcpy(instr, "msub ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(mplcand));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(mplier));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(mnend));
        return instr;
}

char *cmp(enum reg reg1, enum reg reg2){
        char *instr = (char*) malloc(13 * sizeof(char));
        strcpy(instr, "cmp ");
        strcat(instr, reg_to_string(reg1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(reg2));
        return instr;
}

char *cset(enum reg reg, char cond[3]){
        char *instr = (char*) malloc(13 * sizeof(char));
        strcpy(instr, "cset ");
        strcat(instr, reg_to_string(reg));
        strcat(instr, ", ");
        strcat(instr, cond);
        return instr;
}

char *and(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "and ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *orr(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "orr ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *eor(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "eor ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *lsl(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "lsl ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *lsr(enum reg dest, enum reg src1, enum reg src2){
        char *instr = (char*) malloc(18 * sizeof(char));
        strcpy(instr, "lsr ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src1));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src2));
        return instr;
}

char *mvn(enum reg dest, enum reg src){
        char *instr = (char*) malloc(13 * sizeof(char));
        strcpy(instr, "mvn ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src));
        return instr;
}

char *not(enum reg dest, enum reg src){
        char *instr = (char*) malloc(17 * sizeof(char));
        strcpy(instr, "eor ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", ");
        strcat(instr, reg_to_string(src));
        strcat(instr, ", #1");
        return instr;
}

char *cmpi(enum reg reg, long long imm){
        assert((-65537 <= imm) && (imm <= 65535));
        char *instr = (char*) malloc(16 * sizeof(char));
        strcpy(instr, "cmp ");
        strcat(instr, reg_to_string(reg));
        strcat(instr, ", #");
        char imm_str[7];
        sprintf(imm_str, "%lli", imm);
        strcat(instr, imm_str);
        return instr;
}

char *ldr(enum reg dest, enum reg addr){
        char *instr = (char*) malloc(15 * sizeof(char));
        strcpy(instr, "ldr ");
        strcat(instr, reg_to_string(dest));
        strcat(instr, ", [");
        strcat(instr, reg_to_string(addr));
        strcat(instr, "]");
        return instr;
}

char *str(enum reg src, enum reg addr){
        char *instr = (char*) malloc(15 * sizeof(char));
        strcpy(instr, "str ");
        strcat(instr, reg_to_string(src));
        strcat(instr, ", [");
        strcat(instr, reg_to_string(addr));
        strcat(instr, "]");
        return instr;   
}

char *memory_copy(enum reg src, enum reg dest, long long bytes){
        assert((bytes % 8) == 0);
        long long words = bytes / 8;
        char *store_lr = (char*) malloc(20 * sizeof(char));
        strcpy(store_lr, "str lr, [sp, #-16]!");
        char *bl = (char*) malloc(10 * sizeof(char));
        strcpy(bl, "bl memcpy");
        char *restore_lr = (char*) malloc(18 * sizeof(char));
        strcpy(restore_lr, "ldr lr, [sp], #16");

        return concat(6,
                mov(REG_0, src),
                mov(REG_1, dest),
                movi(REG_2, words),
                store_lr,
                bl,
                restore_lr
        );
}

char *ret(){
        char *instr = (char*) malloc(4 * sizeof(char));
        strcpy(instr, "ret");
        return instr;
}

char *comment(char *text){
    char *val = (char*) malloc((strlen(text) + 4) * sizeof(char));
    strcpy(val, "// ");
    strcat(val, text);
    return val;
}

char *literal(char *text){
        char *val = (char*) malloc((strlen(text) + 1) * sizeof(char));
        strcpy(val, text);
        return val;
}

char *include(char *module){
        char *instr = (char*) malloc((strlen(module) + 14) * sizeof(char));
        strcpy(instr, ".include \"");
        strcat(instr, module);
        strcat(instr, ".s\"");
        return instr;
}

char *concat(size_t count, ...){
        va_list args, args2;
        va_start(args, count);
        va_copy(args2, args);
        size_t len = 1; // for terminating with '\0'

        // find total length
        for (size_t i = 0; i < count; ++i){
                char *cur = va_arg(args, char*);
                size_t curlen = strlen(cur);
                len += curlen;
                if ((curlen >= 1) && (cur[curlen - 1] != '\n')){
                        ++len;
                }
        }
        va_end(args);

        char *res = malloc(len * sizeof(char));
        char *dst = res;

        // copy strings
        for (size_t i = 0; i < count; ++i) {
                char *cur = va_arg(args2, char*);
                size_t curlen = strlen(cur);

                memcpy(dst, cur, curlen);
                dst += curlen;

                if ((curlen >= 1) && (cur[curlen - 1] != '\n')){
                        *dst = '\n';
                        ++dst;
                }

                free(cur);
        }

        *dst = '\0';
        va_end(args2);
        return res;
}
