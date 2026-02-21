%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;

static int parse_register(const char *name, int *value) {
    if (strcmp(name, "A") == 0) { *value = 0; return 1; }
    if (strcmp(name, "B") == 0) { *value = 1; return 1; }
    if (strcmp(name, "C") == 0) { *value = 2; return 1; }
    if (strcmp(name, "PAGE") == 0) { *value = 3; return 1; }
    if (strcmp(name, "7SEG") == 0) { *value = 4; return 1; }
    if (strcmp(name, "LCD") == 0) { *value = 6; return 1; }
    return 0;
}
%}

%code requires {
    #include "asm.h"
}

%define parse.error detailed

%union {
    int ival;
    char *str;
    operand_t operand;
    operand_list_t op_list;
}

%token <str> ID
%token <ival> NUMBER
%token EOL

%type <operand> operand
%type <op_list> operand_list opt_operands

%start program

%%

program:
    lines
    ;

lines:
    /* empty */
    | lines line
    ;

line:
    EOL
    | instruction EOL
    | ID ':' EOL {
        asm_set_line(yylineno);
        define_label($1);
        free($1);
    }
    | ID ':' {
        asm_set_line(yylineno);
        define_label($1);
        free($1);
    } instruction EOL
    ;

instruction:
    ID opt_operands {
        asm_set_line(yylineno);
        emit_instruction($1, &$2);
        free($1);
        free_operand_list(&$2);
    }
    ;

opt_operands:
    /* empty */ {
        $$ = (operand_list_t){ .count = 0 };
    }
    | operand_list
    ;

operand_list:
    operand {
        $$ = (operand_list_t){ .count = 1 };
        $$.ops[0] = $1;
    }
    | operand_list ',' operand {
        if ($1.count >= 2) {
            yyerror("too many operands (max 2)");
            if ($3.kind == OPERAND_LABEL && $3.label) {
                free($3.label);
            }
            $$ = $1;
        } else {
            $$ = $1;
            $$.ops[$$.count++] = $3;
        }
    }
    ;

operand:
    NUMBER {
        $$ = (operand_t){ .kind = OPERAND_IMM, .value = $1, .label = NULL };
    }
    | ID {
        int reg = 0;
        if (parse_register($1, &reg)) {
            free($1);
            $$ = (operand_t){ .kind = OPERAND_REG, .value = reg, .label = NULL };
        } else {
            $$ = (operand_t){ .kind = OPERAND_LABEL, .value = 0, .label = $1 };
        }
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error (line %d): %s\n", yylineno, s);
    exit(1);
}
