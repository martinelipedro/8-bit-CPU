%option noyywrap
%option yylineno
%option noinput
%option nounput

%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.tab.h"

static char *to_upper_dup(const char *text) {
    size_t len = strlen(text);
    char *out = malloc(len + 1);
    if (!out) {
        return NULL;
    }
    for (size_t i = 0; i < len; i++) {
        out[i] = (char)toupper((unsigned char)text[i]);
    }
    out[len] = '\0';
    return out;
}

static int parse_binary(const char *text) {
    int value = 0;
    for (const char *p = text; *p; p++) {
        value = (value << 1) | (*p == '1');
    }
    return value;
}

int lexer_eof_emitted = 0;
void lexer_reset(void) {
    lexer_eof_emitted = 0;
    yylineno = 1;
}
%}

%%

[ \t\r]+            ;
"#"[^\n]*           ;

0[xX][0-9a-fA-F]+    { yylval.ival = (int)strtol(yytext, NULL, 0); return NUMBER; }
0[bB][01]+           { yylval.ival = parse_binary(yytext + 2); return NUMBER; }
[0-9]+               { yylval.ival = (int)strtol(yytext, NULL, 10); return NUMBER; }

"\n"                { return EOL; }
","                  { return ','; }
":"                  { return ':'; }

[A-Za-z_][A-Za-z0-9_]* {
    char *upper = to_upper_dup(yytext);
    if (!upper) {
        return 0;
    }
    yylval.str = upper;
    return ID;
}

7[[:alpha:]][[:alnum:]]* {
    char *upper = to_upper_dup(yytext);
    if (!upper) {
        return 0;
    }
    yylval.str = upper;
    return ID;
}

<<EOF>> {
    if (!lexer_eof_emitted) {
        lexer_eof_emitted = 1;
        return EOL;
    }
    return 0;
}

. {
    return yytext[0];
}

%%
