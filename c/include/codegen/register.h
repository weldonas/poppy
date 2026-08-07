#ifndef REGISTER_H
#define REGISTER_H
enum reg {
    REG_0,
    REG_1,
    REG_2,
    REG_3,
    REG_4,
    REG_5,
    REG_6,
    REG_7,
    REG_8,
    REG_SCRATCH,
    REG_RESULT,
    REG_ARG_CHUNK_PTR,
    REG_SCRATCH2,
    REG_SCRATCH3,
    REG_BYTE_OFFSET,
    REG_15,
    REG_16,
    REG_17,
    REG_18,
    REG_19,
    REG_20,
    REG_21,
    REG_22,
    REG_23,
    REG_24,
    REG_25,
    REG_26,
    REG_27,
    REG_28,
    REG_FP,
    REG_LR,
    REG_SP
};

char *reg_to_string(enum reg reg);
#endif
