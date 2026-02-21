# 8-bit CPU

Projeto de uma CPU 8-bit (hardware + microcódigo + assembler). Este repositório reúne:
- geração de microcódigo (EEPROMs)
- tabela de instruções e assembler
- documentação e mídia do hardware

## Estrutura

- `microcode/`: gerador das EEPROMs e planilha de microcódigo
- `compiler/`: assembler e tabela de instruções
- `docs/`: documentação geral do projeto
- `hardware/`: esquemáticos, PCBs, notas de hardware
- `media/`: fotos e vídeos

## Gerar microcódigo

```sh
cd microcode
g++ -std=c++17 -O2 -o microcode main.cpp
./microcode
```

Saídas geradas:
- `microcode/eeprom1.bin`, `microcode/eeprom2.bin`, `microcode/eeprom3.bin`
- `microcode/alu_eeprom.bin`

## Assembler

```sh
cd compiler
make
./assembler programa.asm saida.bin
```

## Codificação

- Cada instrução ocupa 2 bytes.
- Byte 0: opcode.
- Byte 1: dados (nibble alto = registrador de saída, nibble baixo = registrador de entrada).
- Labels apontam para endereço em bytes.

## Notas

- A tabela de instruções está em `compiler/8BIT CPU - Instruction Table.csv`.
- O microcódigo está em evolução; nem todos os opcodes já estão implementados no hardware.
