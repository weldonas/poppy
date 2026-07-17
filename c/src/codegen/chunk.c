#include "codegen/chunk.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/assem.h"
#include "codegen/register.h"
#include "data/map.h"

struct index {
        uint32_t data;
};

DEFINE_MAP(string, index);

struct chunk {
        struct MAP(string, index) offsets; 
        uint32_t next_offset;
        uint32_t size;
};

void free_chunk_entry(const struct MAP_ENTRY(string, index) *entry){
        free((void*) entry->key);
        free((void*) entry->value);
        free((void*) entry);
}

struct chunk *new_chunk(){
        struct chunk *ptr = (struct chunk*) malloc(sizeof(struct chunk));
        init_map((&ptr->offsets), equals_string, free_chunk_entry, string, index);
        ptr->next_offset = 8;
        ptr->size = 16;
        return ptr;
}

void free_chunk(struct chunk *chunk){
        free_map((&chunk->offsets), string, index);
        free(chunk);
}

void add_variable(struct chunk *chunk, struct variable var){
        struct string *v = (struct string*) malloc(sizeof(struct string));
        v->data = var.string;
        struct index *i = (struct index*) malloc(sizeof(struct index));
        i->data = chunk->next_offset;
        update_map((&chunk->offsets), v, i, string, index);

        size_t num_vars = chunk->offsets.list->len;
        // if we have an even number of variables, we use an odd number of words
        // and we have to increment by 16 to keep the stack pointer aligned
        if (num_vars % 2 == 0){
                chunk->size += 16;
        }

        // chunk->next_offset holds the next available offset, which is also the size of the chunk
        chunk->next_offset += var.type->word_count * 8;

        if ((chunk->next_offset % 16) == 0){
                chunk->size = chunk->next_offset;
        }
        else {
                chunk->size = chunk->next_offset + 8;
        }
        assert(!(chunk->size % 16));
}

bool has_variable(struct chunk *chunk, struct variable var){
        struct string v = {var.string};
        const struct index *result;
        query_map((&chunk->offsets), &v, result, string, index);
        return result != NULL;
}

char *variable_address(struct chunk *chunk, struct variable var, enum reg chunk_address, enum reg dest){
        struct string v = {var.string};
        const struct index *result;
        query_map((&chunk->offsets), &v, result, string, index);
        return concat(2, 
                movi(dest, result->data), 
                add(dest,  chunk_address, dest)
        );
}

char *push_chunk(struct chunk *chunk){
        char *instr = malloc(13 * sizeof(char));
        strcpy(instr, "str x9, [sp]");

        // subtract the size from the stack pointer
        // store the size at the top of the chunk
        return concat(3, 
                movi(REG_SCRATCH, chunk->size),
                sub(REG_SP, REG_SP, REG_SCRATCH),
                instr // str x9, [sp]
        );
}

char *pop_chunk(){
        char *instr = (char*) malloc(13 * sizeof(char));
        strcpy(instr, "ldr x9, [sp]");

        // add the size back to the stack pointer
        return concat(2, 
                instr, // ldr x9, [sp]
                add(REG_SP, REG_SP, REG_SCRATCH)
        );
}

void num_to_string(size_t num, char *ret){
        char tmp[16];
        size_t cur = num;
        size_t len = 0;

        do {
                tmp[len++] = '0' + (cur % 10);
                cur /= 10;
        } while (cur > 0);

        for (size_t i = 0; i < len; ++i){
                ret[i] = tmp[len - i - 1];
        }
        ret[len] = '\0';
}
