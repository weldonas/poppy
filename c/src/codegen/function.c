#include "codegen/function.h"

#include <stdlib.h>

#include "codegen/assem.h"
#include "codegen/chunk.h"
#include "codegen/control.h"

#define CALLER_FRAME_PTR "!cfp"
#define SAVED_LINK "!sl"
#define ARG_CHUNK_PTR "!acp"

struct function {
        char *body;
        char **params;
        size_t params_len;
        struct label *start_label;
        struct label *after_body_label;
        struct chunk *frame;
        struct chunk *param_chunk;
        bool is_main;
};

struct function *new_function(char **params, size_t params_len, char **vars, size_t vars_len, bool is_main){
        struct chunk *frame = new_chunk();
        struct chunk *param_chunk = new_chunk();

        for (size_t i = 0; i < params_len; ++i){
                add_variable(param_chunk, params[i], 1);
        }

        for(size_t i = 0; i < vars_len; ++i){
                add_variable(frame, vars[i], 1);
        }

        add_variable(frame, CALLER_FRAME_PTR, 1);
        add_variable(frame, SAVED_LINK, 1);
        add_variable(frame, ARG_CHUNK_PTR, 1);

        struct function *ptr = (struct function*) malloc(sizeof(struct function));
        ptr->body = NULL;
        ptr->params = params;
        ptr->params_len = params_len;
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
        free((void*) function->params);
        free((void*) function);
}

size_t num_params(const struct function *function){
        return function->params_len;
}

char *read_function_variable(const struct function *function, enum reg reg, char *varname){
        if (has_variable(function->frame, varname)){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return read_variable(function->frame, reg, varname, REG_FP);
        }

        // read from arg chunk pointer
        return concat(2, 
                read_variable(function->frame, REG_SCRATCH, ARG_CHUNK_PTR, REG_FP),
                read_variable(function->param_chunk, reg, varname, REG_SCRATCH)
        );
}

char *write_function_variable(const struct function *function, char *varname, enum reg reg){
        if (has_variable(function->frame, varname)){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return write_variable(function->frame, varname, reg, REG_FP);
        }

        // read from arg chunk pointer
        return concat(2, 
                read_variable(function->frame, REG_SCRATCH, ARG_CHUNK_PTR, REG_FP),
                write_variable(function->param_chunk, varname, reg, REG_SCRATCH)
        );
}

void set_body(struct function *function, char *body){
        function->body = body;
}

char *declare_function(const struct function *function){
        char *fn = concat(12,
                declare_label(function->start_label),
                mov(REG_ARG_CHUNK_PTR, REG_SP),
                push_chunk(function->frame),
                write_variable(function->frame, CALLER_FRAME_PTR, REG_FP, REG_SP),
                mov(REG_FP, REG_SP),
                write_variable(function->frame, SAVED_LINK, REG_LR, REG_SP),
                write_variable(function->frame, ARG_CHUNK_PTR, REG_ARG_CHUNK_PTR, REG_SP),
                function->body,
                declare_label(function->after_body_label),
                read_variable(function->frame, REG_LR, SAVED_LINK, REG_SP),
                read_variable(function->frame, REG_FP, CALLER_FRAME_PTR, REG_SP),
                pop_chunk()
        );

        if (function->is_main){
                return fn;
        }

        return concat(2, fn, ret());
}

char *call_function(const struct function *function, char **args){
        char *arg_evals = NULL;

        for (size_t i = 0; i < function->params_len; ++i){
                char *cur_eval = concat(2,
                        args[i],
                        write_variable(function->param_chunk, function->params[i], REG_ARITH_RESULT, REG_SP)
                );

                if (arg_evals == NULL){
                        arg_evals = cur_eval;
                } else {
                        arg_evals = concat(2, arg_evals, cur_eval);
                }
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
