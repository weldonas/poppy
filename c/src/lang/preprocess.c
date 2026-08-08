#include "lang/preprocess.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data/list.h"

struct module {
        char data[256];
};

DEFINE_LIST(module);

bool contains(struct LIST(module) *list, char *str){
        for (struct LIST_NODE(module) *node = list->head; node != NULL; node = node->next){
                if (strcmp(str, node->data->data) == 0){
                        return true;
                }
        }

        return false;
}

void free_module(struct module *m){
        free(m);
}

void preprocess_module_line(char line[256], FILE *out, struct LIST(module) *included){
        if (strncmp(line, "!!", 2) == 0){
                if (strncmp(line + 2, "munch", 5) == 0){

                        char file_name[256];
                        strcpy(file_name, "../modules/");
                        strcat(file_name, line + 8);

                        file_name[strlen(file_name) - 1] = 0;

                        strcat(file_name, ".pop");

                        if (contains(included, file_name)){
                                return;
                        }

                        FILE *module = fopen(file_name, "r");

                        while (fgets(line, 256, module)){
                                preprocess_module_line(line, out, included);
                        }

                        fclose(module);
                        struct module *s = (struct module*) malloc(sizeof(struct module));
                        strcpy(s->data, file_name);
                        append_list(included, s, module);
                }
                else if (strncmp(line + 2, "-", 1) != 0){
                        // invalid preprocessor directive
                        assert(0);
                }
        }
        else {
                fputs(line, out);
        }
}

void include_modules(char *in_name, char *out_name){
        FILE *in = fopen(in_name, "r");
        FILE *out = fopen(out_name, "w");

        struct LIST(module) included;
        init_list((&included));

        char line[256];
        while(fgets(line, 256, in)){
                preprocess_module_line(line, out, &included);
        }
        fclose(in);
        fclose(out);
        free_list((&included), free_module, module);
}

void remove_comments(char *in_name, char *out_name){
        FILE *in = fopen(in_name, "r");
        FILE *out = fopen(out_name, "w");

        char line[256];
        while(fgets(line, 256, in)){
                // check if comment present in line
                char *pos = strstr(line, "!!-");
                if (pos){
                        // if one is, then ignore comment text by null-terminating whe comment starts
                        *pos = 0;
                }

                fputs(line, out);
        }

        fclose(in);
        fclose(out);
}

void preprocess(char *in_name, char *out_name){
        include_modules(in_name, "a.pop");
        remove_comments("a.pop", out_name);
        remove("a.pop");
}
