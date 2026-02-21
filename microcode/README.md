# Microcode

Gerador das EEPROMs de microcódigo da CPU (3 EEPROMs) + EEPROM da ALU.

## Arquivos

- `main.cpp`: define sinais, sequências de microinstruções e gera as EEPROMs
- `microcode.hpp`: infraestrutura do gerador (layout, regras, builder)
- `8BIT CPU - Microcode.csv`: planilha base das microinstruções
- `generate_alu_eeprom.py`: referência para gerar a EEPROM da ALU (mantido como script auxiliar)

## Build e geração

```sh
g++ -std=c++17 -O2 -o microcode main.cpp
./microcode
```

Saídas:
- `eeprom1.bin`, `eeprom2.bin`, `eeprom3.bin`
- `alu_eeprom.bin`

## ALU

A EEPROM da ALU usa 4 bits de entrada (definidos em `ALU_DATA0..3`) para gerar 6 bits de controle
com a ordem `M S3 S2 S1 S0 Cn` (bit5..bit0). O mapeamento atual é sequencial:

```
0 AND
1 OR
2 XOR
3 NOT A
4 INC (A+1)
5 DEC (A-1)
6 ADD (A+B)
7 SUB (A-B)
8 ALUONES (0xFF)
9 ALUZEROS (0x00)
```

Obs.: o microcódigo ainda está em evolução.
