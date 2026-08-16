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

struct record_data {
        struct LIST(field) fields;
        uint32_t byte_count;
};

DEFINE_MAP(string, type);
DEFINE_MAP(string, record_data);
DEFINE_MAP(string, LIST(string));

void free_record_field_entry(const struct MAP_ENTRY(string, record_data) *entry){
        free((void*) entry->key);
        free_list((&entry->value->fields), free_field, field);
        free((void*) entry->value);
        free((void*) entry);
}

void free_enum_item_entry(const struct MAP_ENTRY(string, string_list) *entry){
        free((void*) entry->key);
        free_list((entry->value), free_string, string);
        free((void*) entry->value);
        free((void*) entry);
}

struct LIST(type) types;
struct MAP(string, record_data) record_fields;
struct MAP(string, string_list) enum_items;

struct record_data *query_record_fields(const char *name){
        struct string s = {.data = name};
        const struct record_data *result;
        query_map((&record_fields), (&s), result, string, record_data);
        return (struct record_data*) result;
}

const struct LIST(string) *query_enum_items(const char *name){
        struct string s = {.data = name};
        const struct LIST(string) *result;
        query_map((&enum_items), (&s), result, string, LIST(string));
        return (struct LIST(string)*) result;
}

bool initialized = false;

struct type *int_ptr = NULL;
struct type *void_ptr = NULL;
struct type *bool_ptr = NULL;
struct type *char_ptr = NULL;
struct type *unit_ptr = NULL;
const struct type *string_ptr = NULL;


void add_builtin_types(){
        struct LIST(variable) string_vars;
        init_list((&string_vars));

        struct variable *ptr = malloc(sizeof(struct variable));
        ptr->string = "ptr";
        ptr->type = pointer_type(char_type());
        append_list((&string_vars), ptr, variable);

        struct variable *length = malloc(sizeof(struct variable));
        length->string = "length";
        length->type = int_type();
        append_list((&string_vars), length, variable);

        string_ptr = record_type("string", string_vars);
}

void initialize_types(){
        assert(!initialized);
        initialized = true;
        init_list((&types));
        init_map((&record_fields), equals_string, free_record_field_entry, string, record_data);
        init_map((&enum_items), equals_string, free_enum_item_entry, string, LIST(string));

        add_builtin_types();
}

void add_type(struct type *new) {
        if (!initialized){
                initialize_types();
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

const struct type* const string_type(){
        if (string_ptr == NULL){
                initialize_types();
        }

        return string_ptr;
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

const struct type* const enum_type(const char *name, struct LIST(string) *items){
        if (query_named_type(name)){
                return NULL;
        }
        
        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_ENUM;
        new->name = name;
        new->byte_count = 8;
        new->is_assignable = false;
        add_type(new);

        struct string *s = malloc(sizeof(struct string));
        s->data = name;
        update_map((&enum_items), s, items, string, LIST(string));
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

bool add_field(struct type *record, struct variable *v){
        assert(v->type->byte_count > 0);

        if (!v->type || (v->type->byte_count == NOT_IN_MEMORY)){
                return false;
        }

        uint32_t prev_offset = 0; 
        uint32_t prev_size = 0; 
        struct record_data *data = query_record_fields(record->name);
        if (data->fields.len > 0){
                prev_offset = data->fields.tail->data->offset;
                prev_size = data->fields.tail->data->var->type->byte_count;
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

        append_list((&data->fields), field, field);

        record->byte_count = offset + size;
        record->byte_count = (record->byte_count + 7) & ~7U;
        data->byte_count = record->byte_count;

        return true;
}

const struct type *query_named_type(const char *name){
        struct record_data *record_data = query_record_fields(name);
        const struct LIST(string) *enum_items = query_enum_items(name);

        if (!record_data && !enum_items){
                return NULL;
        }

        struct type *new = malloc(sizeof(struct type));
        new->category = record_data ? CATEGORY_RECORD : CATEGORY_ENUM;
        new->name = name;
        new->byte_count = record_data ? record_data->byte_count : 8;
        new->is_assignable = false;
        add_type(new);     
        return new;
}

void free_variable_nop(struct variable *v){}

const struct type* const record_type(const char *name, struct LIST(variable) fields){
        if (query_named_type(name)){
                return NULL;
        }
        
        struct type *new = malloc(sizeof(struct type));
        new->category = CATEGORY_RECORD;
        new->name = name;
        new->byte_count = 8;
        new->is_assignable = false;
        add_type(new);

        struct string *s = malloc(sizeof(struct string));
        s->data = name;
        struct record_data *data = malloc(sizeof(struct record_data));
        init_list((&data->fields));
        data->byte_count = 8;
        update_map((&record_fields), s, data, string, record_data);

        for (struct LIST_NODE(variable) *node = fields.head; node != NULL; node = node->next){
                if(!add_field(new, node->data)){
                        free_list((&fields), free_variable_nop, variable);
                        update_map((&record_fields), s, NULL, string, record_data);
                        return NULL;
                }
        }

        free_list((&fields), free_variable_nop, variable);
        return new;
}

const struct type* const return_type(const struct type *type){
        if (type->category == CATEGORY_FUNCTION){
                return type->ret_type;
        }

        return NULL;
}


const struct type *field_type(const struct type *record, const char *name){
        struct record_data *data = query_record_fields(record->name);
        for (struct LIST_NODE(field) *node = data->fields.head; node != NULL; node = node->next){
                if (strcmp(node->data->var->string, name) == 0){
                        return node->data->var->type;
                }
        }

        return NULL;
}

size_t record_type_offset(const struct type *record, const char *name){
        struct record_data *data = query_record_fields(record->name);
        for (struct LIST_NODE(field) *node = data->fields.head; node != NULL; node = node->next){
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
                case CATEGORY_RECORD:
                case CATEGORY_ENUM:
                        assert(equals_type(type, new));
                        return new;
                case CATEGORY_ARRAY:
                        new->element_type = make_assignable(new->element_type);
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

        else if ((t1->category == CATEGORY_RECORD) || (t1->category == CATEGORY_ENUM)){
                // unlike other types, we require that records/enums' names are the same (not just their internals)
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
        if (src->category == CATEGORY_ENUM){
                return is_numeric(dst);
        }

        if (dst->category == CATEGORY_ENUM){
                return is_numeric(src);
        }

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

        free((void*) type);
}

void free_types(){
        free_list((&types), free_type, type);
        free_map((&record_fields), string, record_data);
        free_map((&enum_items), string, LIST(string));
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

void free_enum_item(struct enum_item *i){
        free(i);
}
