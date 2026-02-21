# Microcode

Tabela gerada a partir de `microcode/8BIT CPU - Microcode.csv`.

## FETCH

| Step | Microcode |
|---:|---|
| 1 | COUNTER_OUT |
| 2 | MEMORY_OUT |
| 3 | COUNTER_EN |
| 4 | COUNTER_OUT |
| 5 | MEMORY_OUT |

## MOV

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |

## LDA

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |
| 2 | REG_CONTROL |

## LDB

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |
| 2 | REG_CONTROL |

## STA

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |
| 2 | REG_CONTROL |

## STB

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |
| 2 | REG_CONTROL |

## LDI

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## AND

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## OR

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## XOR

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## NOT

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## INC

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## DEC

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## ADD

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## SUB

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## SHL

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |
| 2 | ALU_OUT |

## CMP

| Step | Microcode |
|---:|---|
| 1 | EN_OUT |

## JE (ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## JNE (NOT ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## JG (NOT ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## JL (NOT ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## JGE (NOT ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## JLE (NOT ZERO)

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |

## HLT

| Step | Microcode |
|---:|---|
| 1 | HLT |

## PUSHI

| Step | Microcode |
|---:|---|
| 1 | STACK_INC |
| 2 | OP_OUT |

## PUSH

| Step | Microcode |
|---:|---|
| 1 | STACK_INC |
| 2 | EN_OUT |

## POP

| Step | Microcode |
|---:|---|
| 1 | STACK_OUT |
| 2 | STACK_DEC |

## SETZ (ZERO)

| Step | Microcode |
|---:|---|
| 1 | ALU_OUT |

## SETC (CARRY)

| Step | Microcode |
|---:|---|
| 1 | ALU_OUT |

## CLF

| Step | Microcode |
|---:|---|
| 1 | ALU_FLAG_CLEAR |

## JMP

| Step | Microcode |
|---:|---|
| 1 | OP_OUT |
