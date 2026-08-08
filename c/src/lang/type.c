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

void initialize_types(){
        assert(!initialized);
        init_list((&types));
        init_map((&record_map), equals_string, free_record_map_entry, string, type);
        initialized = true;
}

void add_type(struct type *new);

void add_builtin_types(){
        struct type *string_ptr = record_type();

        struct variable *ptr = malloc(sizeof(struct variable));
        ptr->string = "ptr";
        ptr->type = pointer_type(char_type());
        add_field(string_ptr, ptr);

        struct variable *length = malloc(sizeof(struct variable));
        length->string = "length";
        length->type = int_type();
        add_field(string_ptr, length);
        name_record_type(string_ptr, "string");
}

void add_type(struct type *new) {
        if (!initialized){
                initialize_types();
                add_builtin_types();
        }

        append_list((&types), new, type);
}

const struct type* const int_type(){
        if (int_ptr == NULL){
                int_ptr = malloc(sizeof(struct type));
                int_ptr->category = CATEGORY_PRIMITIVE;
                int_ptr->repr = INT_CHAR;
                int_ptr->byte_count = 8;
                int_ptr->is_assignable = false;
                add_type(int_ptr);
        }

        return int_ptr;
}

const struct type* const bool_type(){
        if (bool_ptr == NULL){
                bool_ptr = malloc(sizeof(struct type));
                bool_ptr->category = CATEGORY_PRIMITIVE;
                bool_ptr->repr = BOOL_CHAR;
                bool_ptr->byte_count = 1;
                bool_ptr->is_assignable = false;
                add_type(bool_ptr);
        }

        return bool_ptr;
}

const struct type* const void_type(){
        if (void_ptr == NULL){
                void_ptr = malloc(sizeof(struct type));
                void_ptr->category = CATEGORY_PRIMITIVE;
                void_ptr->repr = VOID_CHAR;
                void_ptr->byte_count = NOT_IN_MEMORY;
                void_ptr->is_assignable = false;
                add_type(void_ptr);
        }

        return void_ptr;
}

const struct type* const char_type(){
        if (char_ptr == NULL){
                char_ptr = malloc(sizeof(struct type));
                char_ptr->category = CATEGORY_PRIMITIVE;
                char_ptr->repr = CHAR_CHAR;
                char_ptr->byte_count = 1;
                char_ptr->is_assignable = false;
                add_type(char_ptr);
        }

        return char_ptr;
}

const struct type* const unit_type(){
        if (unit_ptr == NULL){
                unit_ptr = malloc(sizeof(struct type));
                unit_ptr->category = CATEGORY_UNIT;
                unit_ptr->byte_count = NOT_IN_MEMORY;
                unit_ptr->is_assignable = false;
                add_type(unit_ptr);
        }

        return unit_ptr;
}

const struct type* const function_type(const struct type *ret, const struct type *params){
        if (!is_returnable(ret)){
                return NULL;
        }

        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_FUNCTION;
        new->ret_type = ret;
        new->params_type = params;
        new->byte_count = NOT_IN_MEMORY;
        new->is_assignable = false;
        add_type(new);
        return new;
}
struct type* const param_type(){
        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_PARAMS;
        init_list((&new->subtypes));
        new->byte_count = NOT_IN_MEMORY;
        new->is_assignable = false;
        add_type(new);
        return new;
}

const struct type* const pointer_type(const struct type *type){
        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_POINTER;
        new->referenced_type = type;
        new->byte_count = 8;
        new->is_assignable = false;
        add_type(new);
        return new;
}

void add_param(struct type *params, const struct type *type_to_add){
        assert(type_to_add->byte_count != NOT_IN_MEMORY);
        append_list((&params->subtypes), (struct type*) type_to_add, type);
}

const struct type* const array_type(const struct type *element_type, char *length_str){
        if (element_type->byte_count == NOT_IN_MEMORY){
                return NULL;
        }

        long long length = strtoll(length_str, NULL, 10);
        if (length < 1){
                return NULL;
        }

        uint32_t byte_count = element_type->byte_count * length;
        byte_count = ((byte_count + 7) / 8) * 8;

        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_ARRAY;
        new->element_type = element_type;
        new->length = length;
        new->byte_count = byte_count;
        new->is_assignable = false;
        add_type(new);
        return new;
}

struct type* const record_type(){
        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_RECORD;
        init_list((&new->fields));
        new->byte_count = 8;
        new->is_assignable = false;
        new->name = NULL;

        add_type(new);
        return new;
}

bool add_field(struct type *record, struct variable *v){
        assert(v->type->byte_count > 0);

        if (!v->type || (v->type->byte_count == NOT_IN_MEMORY)){
                return false;
        }

        uint32_t prev_offset = 0; 
        uint32_t prev_size = 0; 
        if (record->fields.len > 0){
                prev_offset = record->fields.tail->data->offset;
                prev_size = record->fields.tail->data->var->type->byte_count;
        }

        uint32_t offset = prev_offset + prev_size;
        uint32_t size = v->type->byte_count;

        // align fields larger than 8 bytes
        if (size > 8) {
                offset = (offset + 7) & ~7U;
        }
        else {
                // prevent small fields crossing an 8-byte boundary
                if ((offset & 7) + size > 8) {
                        offset = (offset + 7) & ~7U;
                }
        }

        struct field *field = malloc(sizeof(struct field));
        field->var = v;
        field->offset = offset;

        append_list((&record->fields), field, field);

        record->byte_count = offset + size;
        record->byte_count = (record->byte_count + 7) & ~7U;

        return true;
}

const struct type* const return_type(const struct type *type){
        if (type->category == CATEGORY_FUNCTION){
                return type->ret_type;
        }

        return NULL;
}

bool name_record_type(struct type *record, char *name){
        assert(record->category == CATEGORY_RECORD);
        
        struct string *s = malloc(sizeof(struct string));
        s->data = name;
        record->name = name;

        const struct type *result = NULL;
        query_map((&record_map), s, result, string, type)
        if (result){
                return false;
        }

        update_map((&record_map), s, record, string, type);
        return true;
}

const struct type *query_record_type(const char *name){
        struct string *s = malloc(sizeof(struct string));
        s->data = name;

        const struct type *result;
        query_map((&record_map), s, result, string, type);

        free(s);
        return result;
}

const struct type *field_type(const struct type *record, const char *name){
        for (struct LIST_NODE(field) *node = record->fields.head; node != NULL; node = node->next){
                if (strcmp(node->data->var->string, name) == 0){
                        return node->data->var->type;
                }
        }

        return NULL;
}

size_t record_type_offset(const struct type *record, const char *name){
        for (struct LIST_NODE(field) *node = record->fields.head; node != NULL; node = node->next){
                if (strcmp(node->data->var->string, name) == 0){
                        return node->data->offset;
                }
        }

        return NOT_IN_MEMORY;
}

const struct type *make_assignable(const struct type *type){
        assert(type->byte_count != NOT_IN_MEMORY);

        struct type *new = malloc(sizeof(struct type));
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
                        for (struct LIST_NODE(field) *node = type->fields.head; node != NULL; node = node->next){
                                struct field *f = malloc(sizeof(struct field));
                                f->var = malloc(sizeof(struct variable));
                                f->var->type = make_assignable(node->data->var->type);
                                f->var->string = node->data->var->string;
                                f->offset = node->data->offset;
                                
                                append_list((&new->fields), f, field);
                        }
                        assert(equals_type(type, new));
                        return new;
                case CATEGORY_POINTER:
                        new->referenced_type = make_assignable(new->referenced_type);
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
        else if (t1->category == CATEGORY_POINTER){
                return equals_type(t1->referenced_type, t2->referenced_type);
        }

        return false;
}

bool is_numeric(const struct type *type){
        return (type->category == CATEGORY_PRIMITIVE) && ((type->repr == INT_CHAR) || (type->repr == CHAR_CHAR));
}

bool is_returnable(const struct type *type){
        return (type->byte_count > 0) || (equals_type(type, void_type()));
}

bool can_safe_cast(const struct type *src, const struct type *dst){
        if (src->category != dst->category){
                return false;
        }

        if (equals_type(src, dst)){
                return true;
        }

        switch (src->category){
                case CATEGORY_PRIMITIVE:
                        return is_numeric(src) && is_numeric(dst);
                case CATEGORY_ARRAY:
                        return can_safe_cast(src->element_type, dst->element_type);
                case CATEGORY_POINTER:
                        return can_safe_cast(src->referenced_type, dst->referenced_type);
                default:
                        return false; // this is unreachable
        }
}

void free_type_list_item(const struct type *type){}

void free_type(const struct type *type){
        if (type->category == CATEGORY_PARAMS){
                free_list((&type->subtypes), free_type_list_item, type);
        }

        else if (type->category == CATEGORY_RECORD){
                free_list((&type->fields), free_field, field);
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

void free_field(struct field *f){
        free_variable(f->var);
        free(f);
}
