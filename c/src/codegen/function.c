#include "codegen/function.h"

#include <stdlib.h>

#include "codegen/assem.h"
#include "codegen/chunk.h"
#include "codegen/control.h"
#include "lang/type.h"

#define CALLER_FRAME_PTR "!cfp"
#define SAVED_LINK "!sl"
#define ARG_CHUNK_PTR "!acp"

struct function {
        char *body;
        struct LIST(variable) params;
        struct label *start_label;
        struct label *after_body_label;
        struct chunk *frame;
        struct chunk *param_chunk;
        bool is_main;
};

struct function *new_function(struct LIST(variable) params, struct LIST(variable) vars, bool is_main){
        struct chunk *frame = new_chunk();
        struct chunk *param_chunk = new_chunk();

        for (struct LIST_NODE(variable) *param = params.head; param != NULL; param = param->next){
                add_variable(param_chunk, *param->data);
        }

        for (struct LIST_NODE(variable) *var = vars.head; var != NULL; var = var->next){
                add_variable(frame, *var->data);
        }

        struct variable caller_frame_ptr = { .string = CALLER_FRAME_PTR, .type = int_type() };
        struct variable saved_link = { .string = SAVED_LINK, .type = int_type() };
        struct variable arg_chunk_ptr = { .string = ARG_CHUNK_PTR, .type = int_type() };

        add_variable(frame, caller_frame_ptr);
        add_variable(frame, saved_link);
        add_variable(frame, arg_chunk_ptr);

        struct function *ptr = (struct function*) malloc(sizeof(struct function));
        ptr->body = NULL;
        ptr->params = params;
        ptr->start_label = new_label();
        ptr->after_body_label = new_label();
        ptr->frame = frame;
        ptr->param_chunk = param_chunk;
        ptr->is_main = is_main;

        return ptr;
}

void free_function(const struct function *function){
        free_chunk(function->frame);
        free_chunk(function->param_chunk);
        free_label(function->start_label);
        free_label(function->after_body_label);
        free_list((&function->params), free_variable, variable);
        free((void*) function);
}

size_t num_params(const struct function *function){
        return function->params.len;
}

char *read_function_variable(const struct function *function, enum reg reg, struct word *word){
        if (has_variable(function->frame, word_variable(word))){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return read_variable(function->frame, reg, word, REG_FP);
        }

        // read from arg chunk pointer
        struct word *arg_chunk_ptr = new_word((struct variable){.string = ARG_CHUNK_PTR, .type = int_type()}, 0);
        char *result = concat(2, 
                read_variable(function->frame, REG_SCRATCH, arg_chunk_ptr, REG_FP),
                read_variable(function->param_chunk, reg, word, REG_SCRATCH)
        );
        free_word(arg_chunk_ptr);
        return result;
}

char *write_function_variable(const struct function *function, struct word *word, enum reg reg){
        if (has_variable(function->frame, word_variable(word))){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return write_variable(function->frame, word, reg, REG_FP);
        }

        // read from arg chunk pointer
        struct word *arg_chunk_ptr = new_word((struct variable){.string = ARG_CHUNK_PTR, .type = int_type()}, 0);
        char *result = concat(2, 
                read_variable(function->frame, REG_SCRATCH, arg_chunk_ptr, REG_FP),
                write_variable(function->param_chunk, word, reg, REG_SCRATCH)
        );
        free_word(arg_chunk_ptr);
        return result;
}

void set_body(struct function *function, char *body){
        function->body = body;
}

char *declare_function(const struct function *function){
        struct word *caller_frame_ptr = new_word((struct variable){.string = CALLER_FRAME_PTR, .type = int_type()}, 0);
        struct word *saved_link = new_word((struct variable){.string = SAVED_LINK, .type = int_type()}, 0);
        struct word *arg_chunk_ptr = new_word((struct variable){.string = ARG_CHUNK_PTR, .type = int_type()}, 0);

        char *fn = concat(12,
                declare_label(function->start_label),
                mov(REG_ARG_CHUNK_PTR, REG_SP),
                push_chunk(function->frame),
                write_variable(function->frame, caller_frame_ptr, REG_FP, REG_SP),
                mov(REG_FP, REG_SP),
                write_variable(function->frame, saved_link, REG_LR, REG_SP),
                write_variable(function->frame, arg_chunk_ptr, REG_ARG_CHUNK_PTR, REG_SP),
                function->body,
                declare_label(function->after_body_label),
                read_variable(function->frame, REG_LR, saved_link, REG_SP),
                read_variable(function->frame, REG_FP, caller_frame_ptr, REG_SP),
                pop_chunk()
        );

        free_word(caller_frame_ptr);
        free_word(saved_link);
        free_word(arg_chunk_ptr);

        if (function->is_main){
                return fn;
        }

        return concat(2, fn, ret());
}

char *call_function(const struct function *function, char **args){
        char *arg_evals = NULL;

        size_t i = 0;
        for (struct LIST_NODE(variable) *param = function->params.head; param != NULL; param = param->next, ++i) {
                struct word *word = new_word(*param->data, 0);
                char *cur_eval = concat(2,
                        args[i],
                        write_variable(function->param_chunk, word, REG_ARITH_RESULT, REG_SP)
                );

                if (arg_evals == NULL){
                        arg_evals = cur_eval;
                } else {
                        arg_evals = concat(2, arg_evals, cur_eval);
                }

                free_word(word);
        }

        if (arg_evals == NULL){
                return concat(3, 
                        push_chunk(function->param_chunk),
                        bl(function->start_label),
                        pop_chunk()           
                );
        }

        return concat(4,
                push_chunk(function->param_chunk),
                arg_evals,
                bl(function->start_label),
                pop_chunk()
        );
}

char *hop(const struct function *function){
        return b(function->after_body_label);
}
