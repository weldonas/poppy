#include "codegen/ops.h"

#include "codegen/assem.h"
#include "codegen/control.h"
#include "codegen/register.h"

// op1 in REG_SCRATCH, op2 in REG_RESULT
char *binop(char *op1, char *op2, char *operation){
        return concat(5,
                op1,
                push(REG_RESULT),
                op2,
                pop(REG_SCRATCH),
                operation
        );
}

char *sum(char *op1, char *op2){
        return binop(op1, op2, add(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *subtract(char *op1, char *op2){
        return binop(op1, op2, sub(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *multiply(char *op1, char *op2){
        return binop(op1, op2, mul(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *divide(char *op1, char *op2){
        return binop(op1, op2, sdiv(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *modulo(char *op1, char *op2){
        // initially, x9 has original number (n), x10 has divisor (d)
        // x12 <- n / d
        // x10 <- n - d * (n / d)
        return binop(op1, op2, concat(2,
             sdiv(REG_SCRATCH2, REG_SCRATCH, REG_RESULT),
             msub(REG_RESULT, REG_RESULT, REG_SCRATCH2, REG_SCRATCH)
        ));
}

char *eq(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "eq")
        ));
}

char *ne(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "ne")
        ));
}

char *lt(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "lt")
        ));
}

char *gt(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "gt")
        ));
}

char *le(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "le")
        ));
}

char *ge(char *op1, char *op2){
        return binop(op1, op2, concat(2,
                cmp(REG_SCRATCH, REG_RESULT),
                cset(REG_RESULT, "ge")
        ));
}

char *cnjtn(char *op1, char *op2){
        struct label *short_circuit = new_label(NULL);

        char *result = concat(5,
                op1,
                // if op1 is false, the whole expression is false
                cmpi(REG_RESULT, 0),
                beq(short_circuit),
                // if op1 is true, the whole expression is equal to op2
                op2,
                declare_label(short_circuit)
        );

        free_label(short_circuit);
        return result;
}

char *dsjtn(char *op1, char *op2){
        struct label *short_circuit = new_label(NULL);

        char *result = concat(5,
                op1,
                // if op1 is true, the whole expression is true
                cmpi(REG_RESULT, 1),
                beq(short_circuit),
                // if op1 is false, the whole expression is equal to op2
                op2,
                declare_label(short_circuit)
        );

        free_label(short_circuit);
        return result;
}

char *ngtn(char *op){
        return concat(2, op, not(REG_RESULT, REG_RESULT));
}

char *band(char *op1, char *op2){
        return binop(op1, op2, and(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *bor(char *op1, char *op2){
        return binop(op1, op2, orr(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *bxor(char *op1, char *op2){
        return binop(op1, op2, eor(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *bleft(char *op1, char *op2){
        return binop(op1, op2, lsl(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *bright(char *op1, char *op2){
        return binop(op1, op2, lsr(REG_RESULT, REG_SCRATCH, REG_RESULT));
}

char *bnot(char *op){
        return concat(2, op, mvn(REG_RESULT, REG_RESULT));
}
