# 8-bit CPU Assembler

Assembler simples (C + Flex + Bison) para a CPU handmade. Cada instrução gera 2 bytes:

- Byte 0: opcode
- Byte 1: dados

## Build

```sh
make
```

## Uso

```sh
./assembler programa.asm saida.bin
```

## Sintaxe

- Comentários: `#` até o fim da linha
- Números: decimal (`42`), hex (`0x2A`), binário (`0b101010`)
- Labels: `minha_label:`
- Operandos separados por vírgula
- Maiúsculas/minúsculas são ignoradas
- Evite usar nomes de labels iguais aos registradores (`A`, `B`, `C`, `PAGE`, `7SEG`, `LCD`)

### Registradores

- `A` -> 0
- `B` -> 1
- `C` -> 2
- `PAGE` -> 3
- `7SEG` -> 4
- `LCD` -> 6

### Codificação de registradores

- Byte 1 (dados) guarda dois registradores.
- Nibble alto: registrador de saída (destino).
- Nibble baixo: registrador de entrada (origem).

Ex.: `MOV A, B` gera dados `0x10` (destino B=1, origem A=0).

### Endereço de labels

Labels apontam para o endereço em **bytes** (cada instrução ocupa 2 bytes).

## Instruções

Conforme a planilha `8BIT CPU - Instruction Table.csv` (opcodes 0–42).

Exemplos:

```asm
start:
    LDI 0x10
    MOV A, B
    ADD B, A
    JE start
    HLT
```
