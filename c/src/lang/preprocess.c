#include "lang/preprocess.h"

#include <assert.h>
#include <ctype.h>
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

struct RESULT(unit) preprocess_module_line(char line[256], FILE *out, struct LIST(module) *included){
        if (strncmp(line, "!!", 2) == 0){
                if (strncmp(line + 2, "munch", 5) == 0){

                        char file_name[256];
                        strcpy(file_name, "../modules/");
                        strcat(file_name, line + 8);

                        char *last = file_name + strlen(file_name) - 1;
                        while (isspace(*last)){
                                *last = 0;
                                --last;
                        }

                        strcat(file_name, ".pop");

                        if (contains(included, file_name)){
                                struct RESULT(unit) r;
                                make_ok(r, 0);
                                return r;
                        }

                        FILE *module = fopen(file_name, "r");
                        if (!module){
                                struct RESULT(unit) r;
                                char *err = (char*) malloc((23 + strlen(file_name)) * sizeof(char));
                                strcpy(err, "Could not find module ");
                                strcat(err, file_name);
                                make_error(r, err);
                                return r;
                        }

                        while (fgets(line, 256, module)){
                                struct RESULT(unit) r = preprocess_module_line(line, out, included);
                                if (!r.is_ok){
                                        fclose(module);
                                        return r;
                                }
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

        struct RESULT(unit) r;
        make_ok(r, 0);
        return r;
}

struct RESULT(unit) include_modules(char *in_name, char *out_name){
        FILE *in = fopen(in_name, "r");
        FILE *out = fopen(out_name, "w");

        struct LIST(module) included;
        init_list((&included));

        char line[256];
        while(fgets(line, 256, in)){
                struct RESULT(unit) r = preprocess_module_line(line, out, &included);
                if (!r.is_ok){
                        fclose(in);
                        fclose(out);
                        free_list((&included), free_module, module);
                        return r;
                }
        }
        fclose(in);
        fclose(out);
        free_list((&included), free_module, module);

        struct RESULT(unit) r;
        make_ok(r, 0);
        return r;
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

struct RESULT(unit) preprocess(char *in_name, char *out_name){
        struct RESULT(unit) module_result = include_modules(in_name, "a.pop");
        if (!module_result.is_ok){
                return module_result;
        }

        remove_comments("a.pop", out_name);
        remove("a.pop");

        struct RESULT(unit) r;
        make_ok(r, 0);
        return r;
}
