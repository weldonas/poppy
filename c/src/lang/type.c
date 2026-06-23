#include "lang/type.h"

#include <assert.h>
#include <stdlib.h>

#include "data/list.h"
#include "data/map.h"
#include "lang/parse_tree.h"

#define INT_CHAR 'i'
#define VOID_CHAR 'v'
#define BOOL_CHAR 'b'
#define CHAR_CHAR 'c'
#define UNASSIGNABLE -1

DEFINE_MAP(string, type);

void free_record_map_entry(const struct MAP_ENTRY(string, type) *entry){
        free((void*) entry->key);
        free((void*) entry);
}

struct LIST(type) types;
struct MAP(string, type) record_map;
bool initialized = false;

struct type *int_ptr = NULL;
struct type *void_ptr = NULL;
struct type *bool_ptr = NULL;
struct type *char_ptr = NULL;
struct type *unit_ptr = NULL;

void add_type(struct type *new, char *name) {
        if (!initialized) {
                init_list((&types));
                init_map((&record_map), equals_string, free_record_map_entry, string, type);
                initialized = true;
        }

        append_list((&types), new, type);

        if (name){
                struct string *s = (struct string*) malloc(sizeof(struct string));
                s->data = name;
                update_map((&record_map), s, new, string, type);
        }
}

const struct type* const int_type(){
        if (int_ptr == NULL){
                int_ptr = (struct type*) malloc(sizeof(struct type));
                int_ptr->category = CATEGORY_PRIMITIVE;
                int_ptr->repr = INT_CHAR;
                int_ptr->word_count = 1;
                add_type(int_ptr, 0);
        }

        return int_ptr;
}

const struct type* const bool_type(){
        if (bool_ptr == NULL){
                bool_ptr = (struct type*) malloc(sizeof(struct type));
                bool_ptr->category = CATEGORY_PRIMITIVE;
                bool_ptr->repr = BOOL_CHAR;
                bool_ptr->word_count = 1;
                add_type(bool_ptr, 0);
        }

        return bool_ptr;
}

const struct type* const void_type(){
        if (void_ptr == NULL){
                void_ptr = (struct type*) malloc(sizeof(struct type));
                void_ptr->category = CATEGORY_PRIMITIVE;
                void_ptr->repr = VOID_CHAR;
                void_ptr->word_count = UNASSIGNABLE;
                add_type(void_ptr, 0);
        }

        return void_ptr;
}

const struct type* const char_type(){
        if (char_ptr == NULL){
                char_ptr = (struct type*) malloc(sizeof(struct type));
                char_ptr->category = CATEGORY_PRIMITIVE;
                char_ptr->repr = CHAR_CHAR;
                char_ptr->word_count = 1;
                add_type(char_ptr, 0);
        }

        return char_ptr;
}

const struct type* const unit_type(){
        if (unit_ptr == NULL){
                unit_ptr = (struct type*) malloc(sizeof(struct type));
                unit_ptr->category = CATEGORY_UNIT;
                unit_ptr->word_count = UNASSIGNABLE;
                add_type(unit_ptr, 0);
        }

        return unit_ptr;
}

const struct type* const function_type(const struct type *ret, const struct type *params){
        if (!is_returnable(ret)){
                return NULL;
        }

        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_FUNCTION;
        new->ret_type = ret;
        new->params_type = params;
        new->word_count = UNASSIGNABLE;
        add_type(new, 0);
        return new;
}
struct type* const param_type(){
        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_PARAMS;
        init_list((&new->subtypes));
        new->word_count = UNASSIGNABLE;
        add_type(new, 0);
        return new;
}

void add_param(struct type *params, const struct type *type_to_add){
        assert(is_assignable(type_to_add));
        append_list((&params->subtypes), (struct type*) type_to_add, type);
}

const struct type* const array_type(const struct type *element_type, char *length_str){
        if (!is_assignable(element_type)){
                return NULL;
        }

        long long length = strtoll(length_str, NULL, 10);
        if (length < 1){
                return NULL;
        }

        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_ARRAY;
        new->element_type = element_type;
        new->length = length;
        new->word_count = element_type->word_count * length;
        add_type(new, 0);
        return new;
}

const struct type* const record_type(char *name, const struct LIST(variable) fields){
        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_RECORD;
        new->fields = fields;
        new->word_count = 0;

        for (struct LIST_NODE(variable) *node = fields.head; node != NULL; node = node->next){
                new->word_count += node->data->type->word_count;
        }

        add_type(new, name);
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
                return equals_type(t1->params_type, t2->params_type) && equals_type(t1->ret_type, t2->ret_type);
        }

        else if (t1->category == CATEGORY_PARAMS){
                if (t1->subtypes.len != t2->subtypes.len){
                        return false;
                }

                for (
                        struct LIST_NODE(type) *t1node = t1->subtypes.head, *t2node = t2->subtypes.head; 
                        t1node != NULL; 
                        t1node = t1node->next, t2node = t2node->next)
                {
                        if (!equals_type(t1node->data, t2node->data)){
                                return false;
                        }
                }

                return true;
        }

        else if (t1->category == CATEGORY_ARRAY){
                if (!equals_type(t1->element_type, t2->element_type)){
                        return false;
                }

                return t1->length == t2->length;
        }

        else if (t1->category == CATEGORY_UNIT){
                return true;
        }

        return false;
}

bool is_numeric(const struct type *type){
        return (type->category == CATEGORY_PRIMITIVE) && ((type->repr == INT_CHAR) || (type->repr == CHAR_CHAR));
}

bool is_assignable(const struct type *type){
        return type->word_count != UNASSIGNABLE;
}

bool is_returnable(const struct type *type){
        return type->category == CATEGORY_PRIMITIVE;
}

void free_type_list_item(const struct type *type){}

void free_type(const struct type *type){
        if (type->category == CATEGORY_PARAMS){
                free_list((&type->subtypes), free_type_list_item, type);
        }

        free((void*) type);
}

void free_types(){
        free_list((&types), free_type, type);
        free_map((&record_map), string, type);
        int_ptr = NULL;
        bool_ptr = NULL;
        void_ptr = NULL;
        char_ptr = NULL;
        initialized = false;
}

void free_variable(struct variable *v){
        free(v);
}
