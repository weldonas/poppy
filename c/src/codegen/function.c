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

char *read_function_variable(const struct function *function, enum reg reg, struct variable var){
        if (has_variable(function->frame, var)){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return concat(2, 
                        variable_address(function->frame, var, REG_FP),
                        ldr(reg, REG_ADDRESS_RESULT)
                );
        }

        // read from arg chunk pointer
        struct variable arg_chunk_ptr = {.string = ARG_CHUNK_PTR, .type = int_type()};
        char *result = concat(4,
                variable_address(function->frame, arg_chunk_ptr, REG_FP),
                ldr(REG_SCRATCH, REG_ADDRESS_RESULT),
                variable_address(function->param_chunk, var, REG_SCRATCH),
                ldr(reg, REG_ADDRESS_RESULT)
        );
        return result;
}

char *write_function_variable(const struct function *function, struct variable var, enum reg reg){
        if (has_variable(function->frame, var)){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                return concat(2, 
                        variable_address(function->frame, var, REG_FP),
                        str(reg, REG_ADDRESS_RESULT)
                );
        }

        // read from arg chunk pointer
        struct variable arg_chunk_ptr = {.string = ARG_CHUNK_PTR, .type = int_type()};
        char *result = concat(4, 
                variable_address(function->frame, arg_chunk_ptr, REG_FP),
                ldr(REG_SCRATCH, REG_ADDRESS_RESULT),
                variable_address(function->param_chunk, var, REG_SCRATCH),
                str(reg, REG_ADDRESS_RESULT)
        );
        return result;
}

void set_body(struct function *function, char *body){
        function->body = body;
}

char *declare_function(const struct function *function){
        struct variable caller_frame_ptr = {.string = CALLER_FRAME_PTR, .type = int_type()};
        struct variable saved_link = {.string = SAVED_LINK, .type = int_type()};
        struct variable arg_chunk_ptr = {.string = ARG_CHUNK_PTR, .type = int_type()};

        char *fn = concat(17,
                declare_label(function->start_label),
                mov(REG_ARG_CHUNK_PTR, REG_SP),
                push_chunk(function->frame),
                variable_address(function->frame, caller_frame_ptr, REG_SP),
                str(REG_FP, REG_ADDRESS_RESULT),
                mov(REG_FP, REG_SP),
                variable_address(function->frame, saved_link, REG_SP),
                str(REG_LR, REG_ADDRESS_RESULT),
                variable_address(function->frame, arg_chunk_ptr, REG_SP),
                str(REG_ARG_CHUNK_PTR, REG_ADDRESS_RESULT),
                function->body,
                declare_label(function->after_body_label),
                variable_address(function->frame, saved_link, REG_SP),
                ldr(REG_LR, REG_ADDRESS_RESULT),
                variable_address(function->frame, caller_frame_ptr, REG_SP),
                ldr(REG_FP, REG_ADDRESS_RESULT),
                pop_chunk()
        );

        if (function->is_main){
                return fn;
        }

        return concat(2, fn, ret());
}

char *call_function(const struct function *function, char **args){
        char *arg_evals = NULL;

        size_t i = 0;
        for (struct LIST_NODE(variable) *param = function->params.head; param != NULL; param = param->next, ++i) {
                char *cur_eval = concat(3,
                        args[i],
                        variable_address(function->param_chunk, *param->data, REG_SP),
                        str(REG_ARITH_RESULT, REG_ADDRESS_RESULT)
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
