# 8-bit CPU

Projeto de uma CPU 8-bit desenvolvida em protoboards (Hardware & Software). Este repositório reúne:
- esquemáticos de hardware;
- documentação e mídia do hardware
- tabela de instruções e assembler customizado;
- geração de microcódigo para CPU multi-cycle

## Estrutura

- `microcode/`: gerador das EEPROMs e planilha de microcódigo
- `compiler/`: assembler e tabela de instruções
- `docs/`: documentação geral do projeto
- `hardware/`: esquemáticos, PCBs, notas de hardware
- `media/`: fotos e vídeos

## Notas

- A tabela de instruções está em `docs/instructions.md`
- A tabela do microcódigo está em `docs/microcode.md`
- O microcódigo está em evolução; nem todos os opcodes já estão implementados no hardware.

## Capacidades

- Stack com `CALL` e `RET`
- Display 7 segmentos
- Display LCD com driver em 4 bits (expansível para displays maiores)
- Clock base testado e estável até 100 kHz (com margem para operar acima disso)
- 3 registradores multipurpose, com `C` contando up/down sem passar pela ALU e usado em loops
- Acumulador `A` dedicado na ALU
- 13 bits de endereços com paginação
- RAM de `2^13 bits = 8kB`
- LEDs de status para acompanhar o funcionamento
- ALU com operações aritméticas e bitwise (AND/OR/XOR/NOT/INC/DEC/ADD/SUB)
- Decoders na EEPROM com capacidade inicial de 64 control signals
