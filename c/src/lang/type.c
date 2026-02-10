#include "lang/type.h"

#include "data/list.h"

#define INT_CHAR 'i'
#define VOID_CHAR 'v'
#define BOOL_CHAR 'b'
#define CHAR_CHAR 'c'

DEFINE_LIST(type);

struct LIST(type) types;
bool initialized = false;

struct type *int_ptr = NULL;
struct type *void_ptr = NULL;
struct type *bool_ptr = NULL;
struct type *char_ptr = NULL;

void add_type(struct type *new) {
        if (!initialized) {
                init_list((&types));
                initialized = true;
        }

        append_list((&types), new, type);
}

const struct type* const int_type(){
        if (int_ptr == NULL){
                int_ptr = (struct type*) malloc(sizeof(struct type));
                int_ptr->category = CATEGORY_PRIMITIVE;
                int_ptr->repr = INT_CHAR;
                add_type(int_ptr);
        }

        return int_ptr;
}

const struct type* const bool_type(){
        if (bool_ptr == NULL){
                bool_ptr = (struct type*) malloc(sizeof(struct type));
                bool_ptr->category = CATEGORY_PRIMITIVE;
                bool_ptr->repr = BOOL_CHAR;
                add_type(bool_ptr);
        }

        return bool_ptr;
}

const struct type* const void_type(){
        if (void_ptr == NULL){
                void_ptr = (struct type*) malloc(sizeof(struct type));
                void_ptr->category = CATEGORY_PRIMITIVE;
                void_ptr->repr = VOID_CHAR;
                add_type(void_ptr);
        }

        return void_ptr;
}

const struct type* const char_type(){
        if (char_ptr == NULL){
                char_ptr = (struct type*) malloc(sizeof(struct type));
                char_ptr->category = CATEGORY_PRIMITIVE;
                char_ptr->repr = CHAR_CHAR;
                add_type(char_ptr);
        }

        return char_ptr;
}

const struct type* const function_type(const struct type *ret, const struct type *params){
        if (!is_returnable(ret)){
                return NULL;
        }

        for (const struct type *param = params; param != NULL; param = param->previous){
                if (!is_assignable(param->current_type)){
                        return NULL;
                }
        }

        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_FUNCTION;
        new->ret_type = ret;
        new->params_type = params;
        add_type(new);
        return new;
}


const struct type* const param_type(const struct type *current, const struct type *previous){
        if (!is_returnable(current)){
                return NULL;
        }

        if (previous && (previous->category != CATEGORY_PARAMS)){
                return NULL;
        }

        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_PARAMS;
        new->current_type = current;
        new->previous = previous;
        add_type(new);
        return new;
}

const struct type* const return_type(const struct type *type){
        if (type->category == CATEGORY_FUNCTION){
                return type->ret_type;
        }

        return NULL;
}

bool equals_type(const struct type *t1, const struct type *t2){
        if (t1->category != t2->category){
                return false;
        }

        if (t1->category == CATEGORY_PRIMITIVE){
                return t1->repr == t2->repr;
        }

        else if (t1->category == CATEGORY_FUNCTION){
                if ((t1->params_type == NULL) != (t2->params_type == NULL)){
                        return false;
                }

                if (t1->params_type){
                        return equals_type(t1->params_type, t2->params_type) && equals_type(t1->ret_type, t2->ret_type);
                }

                return equals_type(t1->ret_type, t2->ret_type);
        }

        else if (t1->category == CATEGORY_PARAMS){
                if ((!t1->previous) || (!t2->previous)){
                        return (!t1->previous) && (!t2->previous); // if either doesn't have previous, return true iff both don't
                }

                return equals_type(t1->current_type, t2->current_type) && equals_type(t1->previous, t2->previous);
        }

        return false;
}

bool is_numeric(const struct type *type){
        return (type->category == CATEGORY_PRIMITIVE) && ((type->repr == INT_CHAR) || (type->repr == CHAR_CHAR));
}

bool is_assignable(const struct type *type){
        return is_numeric(type) || ((type->category == CATEGORY_PRIMITIVE) && (type->repr == BOOL_CHAR));
}

bool is_returnable(const struct type *type){
        return is_assignable(type) || ((type->category == CATEGORY_PRIMITIVE) && (type->repr == VOID_CHAR));
}

void free_type(const struct type *type){
        free((void*) type);
}

void free_types(){
        free_list((&types), free_type, type);
        int_ptr = NULL;
        bool_ptr = NULL;
        void_ptr = NULL;
        initialized = false;
}
