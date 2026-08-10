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

        struct RESULT(unit) preprocess_result = preprocess(argv[1], intermediate_file);
        if (!preprocess_result.is_ok){
                printf("error: %s\n", preprocess_result.error);
                free((void*) preprocess_result.error);
                return 0;       
        }

        printf("preprocessed\n");
        FILE *in = fopen(intermediate_file, "r");
        struct RESULT(token_list) list_result = lex(in);
        fclose(in);
        remove(intermediate_file);

        if (!list_result.is_ok){
                printf("error: %s\n", list_result.error);
                free((void*) list_result.error);
                return 0;
        }

        const struct LIST(token) *list = list_result.value;

        printf("lexed\n");

        const struct grammar *poppy_grammar = get_poppy_grammar();
        struct RESULT(parse_tree) pt_result = parse(poppy_grammar, list);
        free_poppy_grammar();

        if (!pt_result.is_ok) {
                free_list(list, free_token, token);
                free((void*) list);
                printf("error: %s\n", pt_result.error);
                free((void*) pt_result.error);
                return 0;
        }

        printf("parsed\n");

        struct parse_tree *pt = (struct parse_tree*) pt_result.value;
        const struct type_system *const system = get_poppy_type_system();
        struct RESULT(unit) types_result = find_types(system, pt);

        if (!types_result.is_ok){
                printf("error: %s\n", types_result.error);
                free((void*) types_result.error);
                free_parse_tree(pt);
                
                free_poppy_type_system();
                free_types();
                free_list(list, free_token, token);
                free((void*) list);
                return 0;
        }

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
        free((void*) list);

        return 0;
}
