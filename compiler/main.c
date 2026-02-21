#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

int yyparse(void);
extern FILE *yyin;
extern int yylineno;
void lexer_reset(void);

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <input.asm> <output.bin>\n", prog);
}

static void parse_file(const char *path) {
    yyin = fopen(path, "r");
    if (!yyin) {
        perror("fopen input");
        exit(1);
    }
    lexer_reset();
    if (yyparse() != 0) {
        fprintf(stderr, "Parsing failed\n");
        exit(1);
    }
    fclose(yyin);
    yyin = NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    asm_init();

    asm_set_pass(1);
    asm_reset_pc();
    parse_file(input_path);

    FILE *out = fopen(output_path, "wb");
    if (!out) {
        perror("fopen output");
        return 1;
    }

    asm_set_output(out);
    asm_set_pass(2);
    asm_reset_pc();
    parse_file(input_path);

    fclose(out);
    asm_free();

    return 0;
}
