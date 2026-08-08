#include "codegen/function.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "codegen/assem.h"
#include "codegen/chunk.h"
#include "codegen/control.h"
#include "codegen/register.h"
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

struct function *new_function(struct LIST(variable) params, struct LIST(variable) vars, bool is_main, const char *name){
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

        struct function *ptr = malloc(sizeof(struct function));
        ptr->body = NULL;
        ptr->params = params;
        ptr->start_label = new_label(name);
        ptr->after_body_label = new_label(NULL);
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

char *function_variable_address(const struct function *function, struct variable var, enum reg dest){
        assert(dest != REG_SCRATCH);

        if (has_variable(function->frame, var)){
                // frame is on top of the stack (otherwise we wouldn't be in this function)
                char *result = variable_address(function->frame, var, REG_FP, dest);
                return result;
        }

        // read from arg chunk pointer
        struct variable arg_chunk_ptr = {.string = ARG_CHUNK_PTR, .type = int_type()};
        char *result = concat(3,
                variable_address(function->frame, arg_chunk_ptr, REG_FP, dest),
                ldr(REG_SCRATCH, dest),
                variable_address(function->param_chunk, var, REG_SCRATCH, dest)
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
                variable_address(function->frame, caller_frame_ptr, REG_SP, REG_SCRATCH),
                str(REG_FP, REG_SCRATCH),
                mov(REG_FP, REG_SP),
                variable_address(function->frame, saved_link, REG_SP, REG_SCRATCH),
                str(REG_LR, REG_SCRATCH),
                variable_address(function->frame, arg_chunk_ptr, REG_SP, REG_SCRATCH),
                str(REG_ARG_CHUNK_PTR, REG_SCRATCH),
                function->body,
                declare_label(function->after_body_label),
                variable_address(function->frame, saved_link, REG_SP, REG_SCRATCH),
                ldr(REG_LR, REG_SCRATCH),
                variable_address(function->frame, caller_frame_ptr, REG_SP, REG_SCRATCH),
                ldr(REG_FP, REG_SCRATCH),
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
                char *cur_eval;
                if (param->data->type->byte_count > 8) {
                        cur_eval = concat(3,
                                args[i],
                                variable_address(function->param_chunk, *param->data, REG_SP, REG_SCRATCH),
                                memory_copy(REG_RESULT, REG_SCRATCH, param->data->type->byte_count)
                        );
                }
                else if (param->data->type->byte_count == 8){
                        cur_eval = concat(3,
                                args[i],
                                variable_address(function->param_chunk, *param->data, REG_SP, REG_SCRATCH),
                                str(REG_RESULT, REG_SCRATCH)
                        );
                }
                else {
                        cur_eval = concat(3,
                                args[i],
                                variable_address(function->param_chunk, *param->data, REG_SP, REG_SCRATCH),
                                set_bytes_addr(REG_RESULT, REG_SCRATCH, param->data->type->byte_count)
                        );
                }

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
