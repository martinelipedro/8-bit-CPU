#ifndef ASM_H
#define ASM_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPERAND_REG,
    OPERAND_IMM,
    OPERAND_LABEL
} operand_kind_t;

typedef struct {
    operand_kind_t kind;
    int value;
    char *label;
} operand_t;

typedef struct {
    operand_t ops[2];
    int count;
} operand_list_t;

void asm_init(void);
void asm_free(void);
void asm_set_pass(int pass);
void asm_reset_pc(void);
void asm_set_output(FILE *fp);
int asm_get_pc(void);
void asm_set_line(int line);

void define_label(const char *name);
void emit_instruction(const char *mnemonic, const operand_list_t *ops);
void free_operand_list(operand_list_t *ops);

#ifdef __cplusplus
}
#endif

#endif
