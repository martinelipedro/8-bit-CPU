# Instruções

Tabela gerada a partir de `compiler/8BIT CPU - Instruction Table.csv`.

| Opcode | Instrução | Operando 1 | Operando 2 | Descrição |
|---:|---|---|---|---|
| 0 | NOP | X | X | No operation |
| 1 | MOV | A | B | Move data from regs A to B (Read Reg ID) |
| 2 | LDA | ADDR | X | Load reg A with data frrom memory at address ADDR |
| 3 | LDB | ADDR | X | Load reg B with data frrom memory at address ADDR |
| 4 | STA | ADDR | X | Store reg A contents to memory at address ADDR |
| 5 | STB | ADDR | X | Store reg B contents to memory at address ADDR |
| 6 | LDI | VALUE | X | Load Immediate Value VALUE at register A |
| 7 | AND | OPR | RES | Bitwise AND between registers OPR and A. Result goes in RES |
| 8 | OR | REG | X | Bitwise OR between registers OPR and A. Result goes in RES |
| 9 | XOR | REG | X | Bitwise XOR between registers OPR and A. Result goes in RES |
| 10 | NOT | REG | X | Bitwise NOT in register OPR. Result goes in RES |
| 11 | INC | REG | X | Increments 1 at register OPR and puts result in RES |
| 12 | DEC | REG | X | Decrements 1 at register OPR and puts result in RES |
| 13 | ADD | REG | X | Adds OPR and A register. Output goes in RES |
| 14 | SUB | REG | X | Subs OPR and A register. Output goes in RES |
| 15 | SHL | REG | X | Shifts REG data by 1 to the left |
| 16 | CMP | X | X | Compares OPR and A. Set comparision flags |
| 17 | JE | ADDR | X | Jumps to ADDR if equal |
| 18 | JNE | ADDR | X | Jumps to ADDR if not equal |
| 19 | JG | ADDR | X | Jumps to ADDR if greater than |
| 20 | JL | ADDR | X | Jumps to ADDR if A is less than B |
| 21 | JGE | ADDR | X | Jumps to ADDR if greater or equals |
| 22 | JLE | ADDR | X | Jumps to ADDR if less or equals |
| 23 | HLT | X | X | Halts the CPU |
| 24 | PUSHI | VALUE | X | Push an immediate value to the stack |
| 25 | PUSH | REG | X | Push the contents of register REG to the stack |
| 26 | POP | REG | X | Retrieve the top of the stack to the register REG |
| 27 | SETZ | REG | X | Set register REG if ZERO flag is up |
| 28 | SETC | REG | X | Set register REG if CARRY flag is up |
| 29 | CLF | X | X | Clear ALU flags |
| 30 | JMP | ADDR | X | Unconditional JUMP to address ADDR |
| 31 | LCD | X | X | Loads the LCD register value into the LCD |
| 32 | RESET | X | X | Resets the CPU state, except for the RAM |
| 33 | SET | REG | X | Write all ones in register REG |
| 34 | CLR | REG | X | Write all zeroes in register REG |
| 35 | INX | X | X | Increments counter (C) register |
| 36 | DEX | X | X | Decrements counter (C) register |
| 37 | SPU | X | X | Increment stack pointer by 1 |
| 38 | SPD | X | X | Decrement stack pointer by 1 |
| 39 | CALL | ADDR | X | Call subroutine |
| 40 | RET | X | X | Return from subroutine |
| 41 | CE | ADDR | X | Call if equal |
| 42 | CNE | ADDR | X | Call if not equal |
| 44 |  | - | - | - |
| 45 |  | - | - | - |
| 46 |  | - | - | - |