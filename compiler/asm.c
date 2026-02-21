#include "asm.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    FORM_NONE,
    FORM_ADDR,
    FORM_VALUE,
    FORM_REG,
    FORM_REG_REG
} instr_form_t;

typedef struct {
    const char *name;
    uint8_t opcode;
    instr_form_t form;
} instr_def_t;

static const instr_def_t k_instructions[] = {
    {"NOP", 0, FORM_NONE},
    {"MOV", 1, FORM_REG_REG},
    {"LDA", 2, FORM_ADDR},
    {"LDB", 3, FORM_ADDR},
    {"STA", 4, FORM_ADDR},
    {"STB", 5, FORM_ADDR},
    {"LDI", 6, FORM_VALUE},
    {"AND", 7, FORM_REG_REG},
    {"OR", 8, FORM_REG_REG},
    {"XOR", 9, FORM_REG_REG},
    {"NOT", 10, FORM_REG},
    {"INC", 11, FORM_REG},
    {"DEC", 12, FORM_REG},
    {"ADD", 13, FORM_REG_REG},
    {"SUB", 14, FORM_REG_REG},
    {"SHL", 15, FORM_REG},
    {"CMP", 16, FORM_REG},
    {"JE", 17, FORM_ADDR},
    {"JNE", 18, FORM_ADDR},
    {"JG", 19, FORM_ADDR},
    {"JL", 20, FORM_ADDR},
    {"JGE", 21, FORM_ADDR},
    {"JLE", 22, FORM_ADDR},
    {"HLT", 23, FORM_NONE},
    {"PUSHI", 24, FORM_VALUE},
    {"PUSH", 25, FORM_REG},
    {"POP", 26, FORM_REG},
    {"SETZ", 27, FORM_REG},
    {"SETC", 28, FORM_REG},
    {"CLF", 29, FORM_NONE},
    {"JMP", 30, FORM_ADDR},
    {"LCD", 31, FORM_NONE},
    {"RESET", 32, FORM_NONE},
    {"SET", 33, FORM_REG},
    {"CLR", 34, FORM_REG},
    {"INX", 35, FORM_NONE},
    {"DEX", 36, FORM_NONE},
    {"SPU", 37, FORM_NONE},
    {"SPD", 38, FORM_NONE},
    {"CALL", 39, FORM_ADDR},
    {"RET", 40, FORM_NONE},
    {"CE", 41, FORM_ADDR},
    {"CNE", 42, FORM_ADDR}
};

typedef struct {
    char *name;
    int addr;
} symbol_t;

static symbol_t *symbols = NULL;
static size_t symbol_count = 0;
static size_t symbol_cap = 0;

static int current_pass = 1;
static int current_pc = 0;
static FILE *out_fp = NULL;
static int current_line = 1;

static const instr_def_t *find_instruction(const char *name) {
    size_t count = sizeof(k_instructions) / sizeof(k_instructions[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcmp(k_instructions[i].name, name) == 0) {
            return &k_instructions[i];
        }
    }
    return NULL;
}

static void die(const char *msg) {
    fprintf(stderr, "Error (line %d): %s\n", current_line, msg);
    exit(1);
}

static void dief(const char *fmt, const char *detail) {
    fprintf(stderr, "Error (line %d): ", current_line);
    fprintf(stderr, fmt, detail);
    fprintf(stderr, "\n");
    exit(1);
}

static int symtab_lookup(const char *name, int *addr_out) {
    for (size_t i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) {
            if (addr_out) {
                *addr_out = symbols[i].addr;
            }
            return 1;
        }
    }
    return 0;
}

static void symtab_add(const char *name, int addr) {
    if (symtab_lookup(name, NULL)) {
        dief("duplicate label: %s", name);
    }
    if (symbol_count == symbol_cap) {
        size_t new_cap = symbol_cap ? symbol_cap * 2 : 32;
        symbol_t *new_syms = realloc(symbols, new_cap * sizeof(*new_syms));
        if (!new_syms) {
            die("out of memory");
        }
        symbols = new_syms;
        symbol_cap = new_cap;
    }
    symbols[symbol_count].name = strdup(name);
    symbols[symbol_count].addr = addr;
    symbol_count++;
}

static void emit_byte(uint8_t byte) {
    if (current_pass == 2) {
        if (!out_fp) {
            die("output file not set");
        }
        if (fputc(byte, out_fp) == EOF) {
            die("failed to write output");
        }
    }
    current_pc += 1;
}

static uint8_t encode_regpair(int src, int dest) {
    return (uint8_t)(((dest & 0x0F) << 4) | (src & 0x0F));
}

static int eval_immediate(const operand_t *op) {
    int value = 0;
    if (op->kind == OPERAND_IMM) {
        value = op->value;
    } else if (op->kind == OPERAND_LABEL) {
        if (current_pass == 1) {
            value = 0;
        } else {
            int addr = 0;
            if (!symtab_lookup(op->label, &addr)) {
                dief("unknown label: %s", op->label);
            }
            value = addr;
        }
    } else {
        die("expected immediate or label");
    }

    if (value < 0 || value > 255) {
        die("immediate/address out of range (0-255)");
    }
    return value;
}

void asm_init(void) {
    symbols = NULL;
    symbol_count = 0;
    symbol_cap = 0;
    current_pass = 1;
    current_pc = 0;
    out_fp = NULL;
}

void asm_free(void) {
    for (size_t i = 0; i < symbol_count; i++) {
        free(symbols[i].name);
    }
    free(symbols);
    symbols = NULL;
    symbol_count = 0;
    symbol_cap = 0;
}

void asm_set_pass(int pass) {
    current_pass = pass;
}

void asm_reset_pc(void) {
    current_pc = 0;
}

void asm_set_output(FILE *fp) {
    out_fp = fp;
}

int asm_get_pc(void) {
    return current_pc;
}

void asm_set_line(int line) {
    current_line = line;
}

void define_label(const char *name) {
    if (current_pass == 1) {
        symtab_add(name, current_pc);
        return;
    }

    int addr = 0;
    if (!symtab_lookup(name, &addr)) {
        die("label missing in pass 2");
    }
    if (addr != current_pc) {
        dief("label address mismatch for %s", name);
    }
}

void emit_instruction(const char *mnemonic, const operand_list_t *ops) {
    const instr_def_t *instr = find_instruction(mnemonic);
    if (!instr) {
        dief("unknown instruction: %s", mnemonic);
    }

    switch (instr->form) {
    case FORM_NONE:
        if (ops->count != 0) {
            dief("instruction takes no operands: %s", mnemonic);
        }
        emit_byte(instr->opcode);
        emit_byte(0x00);
        break;
    case FORM_ADDR:
    case FORM_VALUE: {
        if (ops->count != 1) {
            dief("instruction expects one operand: %s", mnemonic);
        }
        int value = eval_immediate(&ops->ops[0]);
        emit_byte(instr->opcode);
        emit_byte((uint8_t)value);
        break;
    }
    case FORM_REG: {
        if (ops->count != 1) {
            dief("instruction expects one register operand: %s", mnemonic);
        }
        if (ops->ops[0].kind != OPERAND_REG) {
            dief("expected register operand for %s", mnemonic);
        }
        int reg = ops->ops[0].value;
        emit_byte(instr->opcode);
        emit_byte(encode_regpair(reg, reg));
        break;
    }
    case FORM_REG_REG: {
        if (ops->count != 2) {
            dief("instruction expects two register operands: %s", mnemonic);
        }
        if (ops->ops[0].kind != OPERAND_REG || ops->ops[1].kind != OPERAND_REG) {
            dief("expected register operands for %s", mnemonic);
        }
        int src = ops->ops[0].value;
        int dest = ops->ops[1].value;
        emit_byte(instr->opcode);
        emit_byte(encode_regpair(src, dest));
        break;
    }
    default:
        die("invalid instruction form");
    }
}

void free_operand_list(operand_list_t *ops) {
    for (int i = 0; i < ops->count; i++) {
        if (ops->ops[i].kind == OPERAND_LABEL && ops->ops[i].label) {
            free(ops->ops[i].label);
            ops->ops[i].label = NULL;
        }
    }
}
