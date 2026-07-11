#include "lang/type.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data/list.h"
#include "data/map.h"
#include "lang/parse_tree.h"

#define INT_CHAR 'i'
#define VOID_CHAR 'v'
#define BOOL_CHAR 'b'
#define CHAR_CHAR 'c'

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

void add_type(struct type *new) {
        if (!initialized) {
                init_list((&types));
                init_map((&record_map), equals_string, free_record_map_entry, string, type);
                initialized = true;
        }

        append_list((&types), new, type);
}

const struct type* const int_type(){
        if (int_ptr == NULL){
                int_ptr = (struct type*) malloc(sizeof(struct type));
                int_ptr->category = CATEGORY_PRIMITIVE;
                int_ptr->repr = INT_CHAR;
                int_ptr->word_count = 1;
                int_ptr->is_assignable = false;
                add_type(int_ptr);
        }

        return int_ptr;
}

const struct type* const bool_type(){
        if (bool_ptr == NULL){
                bool_ptr = (struct type*) malloc(sizeof(struct type));
                bool_ptr->category = CATEGORY_PRIMITIVE;
                bool_ptr->repr = BOOL_CHAR;
                bool_ptr->word_count = 1;
                bool_ptr->is_assignable = false;
                add_type(bool_ptr);
        }

        return bool_ptr;
}

const struct type* const void_type(){
        if (void_ptr == NULL){
                void_ptr = (struct type*) malloc(sizeof(struct type));
                void_ptr->category = CATEGORY_PRIMITIVE;
                void_ptr->repr = VOID_CHAR;
                void_ptr->word_count = NOT_IN_MEMORY;
                void_ptr->is_assignable = false;
                add_type(void_ptr);
        }

        return void_ptr;
}

const struct type* const char_type(){
        if (char_ptr == NULL){
                char_ptr = (struct type*) malloc(sizeof(struct type));
                char_ptr->category = CATEGORY_PRIMITIVE;
                char_ptr->repr = CHAR_CHAR;
                char_ptr->word_count = 1;
                char_ptr->is_assignable = false;
                add_type(char_ptr);
        }

        return char_ptr;
}

const struct type* const unit_type(){
        if (unit_ptr == NULL){
                unit_ptr = (struct type*) malloc(sizeof(struct type));
                unit_ptr->category = CATEGORY_UNIT;
                unit_ptr->word_count = NOT_IN_MEMORY;
                unit_ptr->is_assignable = false;
                add_type(unit_ptr);
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
        new->word_count = NOT_IN_MEMORY;
        new->is_assignable = false;
        add_type(new);
        return new;
}
struct type* const param_type(){
        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_PARAMS;
        init_list((&new->subtypes));
        new->word_count = NOT_IN_MEMORY;
        new->is_assignable = false;
        add_type(new);
        return new;
}

void add_param(struct type *params, const struct type *type_to_add){
        assert(type_to_add->word_count != NOT_IN_MEMORY);
        append_list((&params->subtypes), (struct type*) type_to_add, type);
}

const struct type* const array_type(const struct type *element_type, char *length_str){
        if (element_type->word_count == NOT_IN_MEMORY){
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
        new->is_assignable = false;
        add_type(new);
        return new;
}

struct type* const record_type(){
        struct type *new = (struct type*) malloc(sizeof(struct type));
        new->category = CATEGORY_RECORD;
        init_list((&new->fields));
        new->word_count = 0;
        new->is_assignable = false;
        new->name = NULL;

        add_type(new);
        return new;
}

bool add_field(struct type *record, struct variable *v){
        if (!v->type || (v->type->word_count == NOT_IN_MEMORY)){
                return false;
        }
        append_list((&record->fields), v, variable);
        record->word_count += v->type->word_count;
        return true;
}

const struct type* const return_type(const struct type *type){
        if (type->category == CATEGORY_FUNCTION){
                return type->ret_type;
        }

        return NULL;
}

void name_record_type(struct type *record, char *name){
        assert(record->category == CATEGORY_RECORD);
        
        struct string *s = (struct string*) malloc(sizeof(struct string));
        s->data = name;
        record->name = name;
        update_map((&record_map), s, record, string, type);
}

const struct type *query_record_type(const char *name){
        struct string *s = (struct string*) malloc(sizeof(struct string));
        s->data = name;

        const struct type *result;
        query_map((&record_map), s, result, string, type);

        free(s);
        return result;
}

const struct type *field_type(const struct type *record, const char *name){
        for (struct LIST_NODE(variable) *node = record->fields.head; node != NULL; node = node->next){
                if (strcmp(node->data->string, name) == 0){
                        return node->data->type;
                }
        }

        return NULL;
}

size_t record_type_offset(const struct type *record, const char *name){
        size_t current = 0;

        for (struct LIST_NODE(variable) *node = record->fields.head; node != NULL; node = node->next){
                if (strcmp(node->data->string, name) == 0){
                        return current;
                }

                current += node->data->type->word_count;
        }

        return NOT_IN_MEMORY;
}

const struct type *make_assignable(const struct type *type){
        assert(type->word_count != NOT_IN_MEMORY);

        struct type *new = (struct type*) malloc(sizeof(struct type));
        add_type(new);

        memcpy(new, type, sizeof(struct type));
        new->is_assignable = true;

        switch(type->category){
                case CATEGORY_PRIMITIVE:
                        assert(equals_type(type, new));
                        return new;
                case CATEGORY_ARRAY:
                        new->element_type = make_assignable(new->element_type);
                        assert(equals_type(type, new));
                        return new;
                case CATEGORY_RECORD:
                        init_list((&new->fields));
                        for (struct LIST_NODE(variable) *node = type->fields.head; node != NULL; node = node->next){
                                struct variable *v = (struct variable*) malloc(sizeof(struct variable));
                                v->type = make_assignable(node->data->type);
                                v->string = node->data->string;
                                
                                append_list((&new->fields), v, variable);
                        }
                        assert(equals_type(type, new));
                        return new;
                default:
                        assert(0);
                        return NULL;
        }
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

        else if (t1->category == CATEGORY_RECORD){
                // unlike other types, we require that records' names are the same (not just their internals)
                // assuming uniqueness among type names, this is true iff the names are equal

                return strcmp(t1->name, t2->name) == 0;
        }

        else if (t1->category == CATEGORY_UNIT){
                return true;
        }

        return false;
}

bool is_numeric(const struct type *type){
        return (type->category == CATEGORY_PRIMITIVE) && ((type->repr == INT_CHAR) || (type->repr == CHAR_CHAR));
}

bool is_returnable(const struct type *type){
        return (type->word_count > 0) || (equals_type(type, void_type()));
}

bool is_composite(const struct type *type){
        return (type->category == CATEGORY_ARRAY) || (type->category == CATEGORY_RECORD);
}

void free_type_list_item(const struct type *type){}

void free_type(const struct type *type){
        if (type->category == CATEGORY_PARAMS){
                free_list((&type->subtypes), free_type_list_item, type);
        }

        else if (type->category == CATEGORY_RECORD){
                free_list((&type->fields), free_variable, variable);
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
