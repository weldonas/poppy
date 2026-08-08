#include "codegen/control.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/assem.h"

#define MAX_LABEL_INDEX_LENGTH 20 // 2^64 - 1 has 20 digits in base 10
size_t label_count = 0;

struct label {
        char *name;
};

struct label *new_label(const char *name){
        ++label_count;

        char *label_name;

        if (name){
                label_name = malloc((strlen(name) + 1) * sizeof(char));
                strcpy(label_name, name);
        }
        else {
                label_name = malloc((MAX_LABEL_INDEX_LENGTH + 4) * sizeof(char));
                strcpy(label_name, ".L");
                sprintf(label_name + 2, "%zu", label_count);
        }

        struct label *l = malloc(sizeof(struct label));
        l->name = label_name;
        return l;
}

void free_label(struct label *l){
        free(l->name);
        free(l);
}

char *declare_label(struct label *l){
        char *instr = malloc((strlen(l->name) + 2) * sizeof(char));
        strcpy(instr, l->name);
        strcat(instr, ":");
        return instr;
}

char *b(struct label *l){
        char *instr = malloc((strlen(l->name) + 3) * sizeof(char));
        strcpy(instr, "b ");
        strcat(instr, l->name);
        return instr;
}

char *bl(struct label *l){
        char *instr = malloc((strlen(l->name) + 4) * sizeof(char));
        strcpy(instr, "bl ");
        strcat(instr, l->name);
        return instr;
}

char *beq(struct label *l){
        char *instr = malloc((strlen(l->name) + 5) * sizeof(char));
        strcpy(instr, "beq ");
        strcat(instr, l->name);
        return instr;
}

char *bne(struct label *l){
        char *instr = malloc((strlen(l->name) + 5) * sizeof(char));
        strcpy(instr, "bne ");
        strcat(instr, l->name);
        return instr;
}

char *if_stmt(char* cond, char *then_block, char *else_block) {
        char* ret;

        if (else_block){
                struct label *after_then = new_label(NULL);
                struct label *after_else = new_label(NULL);
                ret = concat(8,
                        cond,
                        cmpi(REG_RESULT, 1),
                        bne(after_then),
                        then_block,
                        b(after_else),
                        declare_label(after_then),
                        else_block,
                        declare_label(after_else)
                );
                free_label(after_then);
                free_label(after_else);
        } else {
                struct label *after_then = new_label(NULL);
                ret = concat(5,
                        cond,
                        cmpi(REG_RESULT, 1),
                        bne(after_then),
                        then_block,
                        declare_label(after_then)
                );
                free_label(after_then);
        }

        return ret;
}

char *while_loop(char *cond, char *body){
        struct label *start = new_label(NULL);
        struct label *end = new_label(NULL);

        char *ret = concat(7,
                declare_label(start),
                cond,
                cmpi(REG_RESULT, 1),
                bne(end),
                body,
                b(start),
                declare_label(end)
        );

        free_label(start);
        free_label(end);
        return ret;
}

char *for_loop(char *init, char *cond, char *post, char *body){
        return concat(2,
                init,
                while_loop(cond, concat(2, body, post))
        );
}
