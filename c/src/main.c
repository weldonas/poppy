#include <stdio.h>
#include <stdlib.h>

#include "codegen/program.h"
#include "data/list.h"
#include "lang/lexer.h"
#include "lang/parse_tree.h"
#include "lang/parser.h"
#include "lang/poppy_grammar.h"
#include "lang/poppy_type_system.h"
#include "lang/preprocess.h"
#include "lang/type.h"
#include "lang/type_system.h"

char *intermediate_file = "inter.prog";

int main(int argc, char *argv[]){
        if (argc < 2){
                printf("No input file passed, terminating...\n");
                exit(1);
        }

        preprocess(argv[1], intermediate_file);
        printf("preprocessed\n");
        FILE *in = fopen(intermediate_file, "r");
        struct RESULT(token_list) list_result = lex(in);
        fclose(in);
        remove(intermediate_file);

        if (!list_result.is_ok){
                printf("error: %s\n", list_result.error);
                free(list_result.error);
                return 0;
        }

        struct LIST(token) *list = list_result.value;

        printf("lexed\n");

        const struct grammar *poppy_grammar = get_poppy_grammar();
        struct parse_tree *pt = parse(poppy_grammar, list);
        free_poppy_grammar();

        if (pt == NULL) {
                free_list(list, free_token, token);
                free(list);
                return 0;
        }

        printf("parsed\n");

        const struct type_system *const system = get_poppy_type_system();
        find_types(system, pt);
        if (pt->type != NULL){
                printf("typed\n");
                char *code = generate_code(pt);
                if (code != NULL){
                        printf("generation successful\n");
                        FILE *out = fopen("../assembly/out.s", "w");
                        fprintf(out, "%s", code);
                        free(code);
                        fclose(out);
                }
        }

        free_parse_tree(pt);

        free_poppy_type_system();
        free_types();
        free_list(list, free_token, token);
        free(list);

        return 0;
}
