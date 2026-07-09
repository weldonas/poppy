#include "codegen/register.h"

#include <assert.h>

#define CASE(i) case REG_##i: return "x"#i;


char *reg_to_string(enum reg reg){
        switch(reg){
                case REG_SP:
                        return "sp";
                case REG_LR:
                        return "lr";
                case REG_FP:
                        return "fp";
                case REG_SCRATCH:
                        return "x9";
                case REG_ARITH_RESULT:
                        return "x10";
                case REG_ARG_CHUNK_PTR:
                        return "x11";
                case REG_SCRATCH2:
                        return "x12";
                CASE(0)
                CASE(1)
                CASE(2)
                CASE(3)
                CASE(4)
                CASE(5)
                CASE(6)
                CASE(7)
                CASE(8)
                CASE(13)
                CASE(14)
                CASE(15)
                CASE(16)
                CASE(17)
                CASE(18)
                CASE(19)
                CASE(20)
                CASE(21)
                CASE(22)
                CASE(23)
                CASE(24)
                CASE(25)
                CASE(26)
                CASE(27)
                CASE(28)
        }
        assert(0);
}
