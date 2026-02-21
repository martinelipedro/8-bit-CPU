# Documentação

- `instructions.md`: tabela de instruções (opcodes)
- `microcode.md`: tabela de microcódigo
- `registers.md`: tabela de registradores

## Regras de codificação

- Cada instrução ocupa 2 bytes.
- Byte 0: opcode.
- Byte 1: dados (nibble alto = registrador de saída, nibble baixo = registrador de entrada).
- Labels apontam para endereço em bytes.

## Instruções

Conforme a planilha `instructions.md`.

Exemplos:

```asm
start:
    LDI 0x10
    MOV A, B
    ADD B, A
    JE start
    HLT
```
