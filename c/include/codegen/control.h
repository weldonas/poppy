#ifndef CONTROL_H
#define CONTROL_H

struct label;

struct label *new_label(const char *name);
void free_label(struct label *l);

char *declare_label(struct label *l);
char *b(struct label *l);
char *bl(struct label *l);
char *beq(struct label *l);
char *bne(struct label *l);

char *if_stmt(char* cond, char *then_block, char *else_block);
char *while_loop(char *cond, char *body);
char *for_loop(char *init, char *cond, char *post, char *body);

#endif
