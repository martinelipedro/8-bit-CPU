#include "microcode.hpp"
#include <array>
#include <cstdint>
#include <fstream>
using namespace ucode;

namespace {
struct AluCtrl {
    uint8_t m = 0;
    uint8_t s = 0; // 4-bit S3..S0
    uint8_t cn = 0;
};

// Output bit order (bit5..bit0): M S3 S2 S1 S0 Cn
uint8_t pack_alu(const AluCtrl& a) {
    return uint8_t(((a.m & 1u) << 5) | ((a.s & 0xFu) << 1) | (a.cn & 1u));
}

void write_alu_eeprom(const char* path) {
    // 4-bit input (0..15) -> 6-bit output: M S3 S2 S1 S0 Cn
    // Sequential mapping:
    // 0 AND, 1 OR, 2 XOR, 3 NOT A, 4 INC, 5 DEC, 6 ADD, 7 SUB, 8 ALUONES, 9 ALUZEROS
    const std::array<AluCtrl, 16> map = {{
        {1, 0b1011, 0}, // 0 AND
        {1, 0b1110, 0}, // 1 OR
        {1, 0b0110, 0}, // 2 XOR
        {1, 0b0000, 0}, // 3 NOT A
        {0, 0b0000, 0}, // 4 INC (A+1)
        {0, 0b1111, 1}, // 5 DEC (A-1)
        {0, 0b1001, 1}, // 6 ADD (A+B)
        {0, 0b0110, 0}, // 7 SUB (A-B)
        {1, 0b1100, 0}, // 8 ALUONES (0xFF)
        {1, 0b0011, 0}, // 9 ALUZEROS (0x00)
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
    }};

    std::array<uint8_t, 16> data{};
    for (size_t i = 0; i < data.size(); ++i) data[i] = pack_alu(map[i]);

    std::ofstream f(path, std::ios::binary);
    f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}
} // namespace

int main()
{
    SignalMap sm;

    /*  LOW - INPUT EEPROM */
    sm.define_decoder_line("EN_IN", DecoderSel::D1, 1);
    sm.define_decoder_line("OP_IN", DecoderSel::D1, 2);
    sm.define_decoder_line("INST_IN", DecoderSel::D1, 3);
    sm.define_decoder_line("STACK_IN", DecoderSel::D1, 4);
    sm.define_decoder_line("MEMORY_IN", DecoderSel::D1, 5);
    sm.define_decoder_line("ALU_IN", DecoderSel::D1, 6);
    sm.define_decoder_line("JUMP", DecoderSel::D1, 7);
    sm.define_decoder_line("COUNTER_EN", DecoderSel::D1, 8);
    sm.define_decoder_line("ADDR_IN", DecoderSel::D1, 9);
    sm.define_decoder_line("n10", DecoderSel::D1, 10);
    sm.define_decoder_line("ALU_FLAG_CLEAR", DecoderSel::D1, 11);
    sm.define_decoder_line("n11", DecoderSel::D1, 12);
    sm.define_decoder_line("STACK_ENABLE", DecoderSel::D1, 13);
    sm.define_decoder_line("STACK_DEC", DecoderSel::D1, 14);
    sm.define_decoder_line("STACK_INC", DecoderSel::D1, 15);


    /* HIGH - OUTPUT EEPROM */
    sm.define_decoder_line("EN_OUT", DecoderSel::D2, 1);
    sm.define_decoder_line("OP_OUT", DecoderSel::D2, 2);
    sm.define_decoder_line("INST_OUT", DecoderSel::D2, 3);
    sm.define_decoder_line("STACK_OUT", DecoderSel::D2, 4);
    sm.define_decoder_line("MEMORY_OUT", DecoderSel::D2, 5);
    sm.define_decoder_line("ALU_OUT", DecoderSel::D2, 6);
    sm.define_decoder_line("COUNTER_OUT", DecoderSel::D2, 7);
    sm.define_decoder_line("INPUT_OUT", DecoderSel::D2, 8);
    sm.define_decoder_line("n20", DecoderSel::D2, 9);
    sm.define_decoder_line("n21", DecoderSel::D2, 10);
    sm.define_decoder_line("n22", DecoderSel::D2, 11);
    sm.define_decoder_line("LCD_PULSE", DecoderSel::D2, 12);
    sm.define_decoder_line("HLT", DecoderSel::D2, 13);
    sm.define_decoder_line("C_DEC", DecoderSel::D2, 14);
    sm.define_decoder_line("C_INC", DecoderSel::D2, 15);

    /* EXCLUSIVE EEPROM */
    sm.define_bit("REG_CONTROL", DirectEeprom::E2, 0);
    sm.define_bit("ALU_DATA0", DirectEeprom::E2, 1);
    sm.define_bit("ALU_DATA1", DirectEeprom::E2, 2);
    sm.define_bit("ALU_DATA2", DirectEeprom::E2, 3);
    sm.define_bit("ALU_DATA3", DirectEeprom::E2, 4);
    sm.define_bit("RCTR", DirectEeprom::E2, 5);
    sm.define_bit("ALU_FLAG_IN", DirectEeprom::E2, 6);
    sm.define_bit("STEP_RESET", DirectEeprom::E2, 7);

    /* ---------- */

    auto with_alu_code = [&](MicroAction act, uint8_t code) {
        // ALU_DATA0..3 represent the 4-bit input for the ALU EEPROM (LSB..MSB)
        if (code & 0x1) act.set(sm, "ALU_DATA0"); else act.clr(sm, "ALU_DATA0");
        if (code & 0x2) act.set(sm, "ALU_DATA1"); else act.clr(sm, "ALU_DATA1");
        if (code & 0x4) act.set(sm, "ALU_DATA2"); else act.clr(sm, "ALU_DATA2");
        if (code & 0x8) act.set(sm, "ALU_DATA3"); else act.clr(sm, "ALU_DATA3");
        return act;
    };

    MicrocodeImage img;
    img.set_default_word(MicroWord{ .d1_line=0, .d2_line=0, .e2_bits=0, .e3_bits=0 });

    /* FETCH CYCLE */
    img.add_rule(
        Step(0), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
          .select(sm, "COUNTER_OUT")
          .select(sm, "ADDR_IN")
    );
    img.add_rule(
        Step(1), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
          .select(sm, "MEMORY_OUT")
          .select(sm, "INST_IN")
    );
    img.add_rule(
        Step(2), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
          .select(sm, "COUNTER_EN")
    );
    img.add_rule(
        Step(3), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
          .select(sm, "COUNTER_OUT")
          .select(sm, "ADDR_IN")
    );
    img.add_rule(
        Step(4), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
          .select(sm, "MEMORY_OUT")
          .select(sm, "OP_IN")
    );

    /* MOV */
    img.opcode(0x01)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "EN_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* LDA */
    img.opcode(0x2)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").select(sm, "ADDR_IN"),
        MicroAction{}.set(sm, "REG_CONTROL").clr(sm, "RCTR").select(sm, "MEMORY_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* LDB */
    img.opcode(0x3)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").select(sm, "ADDR_IN"),
        MicroAction{}.set(sm, "REG_CONTROL").set(sm, "RCTR").select(sm, "MEMORY_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* STA */
    img.opcode(0x4)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").select(sm, "ADDR_IN"),
        MicroAction{}.set(sm, "REG_CONTROL").clr(sm, "RCTR").select(sm, "EN_OUT").select(sm, "MEMORY_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* STB */
    img.opcode(0x5)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").select(sm, "ADDR_IN"),
        MicroAction{}.set(sm, "REG_CONTROL").set(sm, "RCTR").select(sm, "EN_OUT").select(sm, "MEMORY_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* LDI */
    img.opcode(0x6)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").set(sm, "REG_CONTROL").clr(sm, "RCTR").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* AND */
    img.opcode(0x7)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x0),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* OR */
    img.opcode(0x8)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x1),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* XOR */
    img.opcode(0x9)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x2),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* NOT */
    img.opcode(0xA)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x3),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* INC */
    img.opcode(0xB)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x4),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* DEC */
    img.opcode(0xC)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x5),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* ADD */
    img.opcode(0xD)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x6),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* SUB */
    img.opcode(0xE)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"), 0x7),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* SHL */
    img.opcode(0xF)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN"),
        MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* CMP */
    img.opcode(0x10)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "EN_OUT").select(sm, "ALU_IN").set(sm, "ALU_FLAG_IN"), 0x7),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* JE (ZF=1) */
    img.add_rule(
        Step(5), Op(0x11), Z(1), AnyFlag(),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x11), Z(0), AnyFlag(),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* JNE (ZF=0) */
    img.add_rule(
        Step(5), Op(0x12), Z(0), AnyFlag(),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x12), Z(1), AnyFlag(),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* JG unsigned (ZF=0, CF=0) */
    img.add_rule(
        Step(5), Op(0x13), Z(0), C(0),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x13), AnyFlag(), AnyFlag(),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* JL unsigned (CF=1) */
    img.add_rule(
        Step(5), Op(0x14), AnyFlag(), C(1),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x14), AnyFlag(), C(0),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* JGE unsigned (CF=0) */
    img.add_rule(
        Step(5), Op(0x15), AnyFlag(), C(0),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x15), AnyFlag(), C(1),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* JLE unsigned (CF=1 or ZF=1) */
    img.add_rule(
        Step(5), Op(0x16), Z(1), AnyFlag(),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x16), Z(0), C(1),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP").set(sm, "STEP_RESET")
    );
    img.add_rule(
        Step(5), Op(0x16), Z(0), C(0),
        MicroAction{}.set(sm, "STEP_RESET")
    );

    /* HLT */
    img.opcode(0x17)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "HLT")
      });

    /* PUSHI */
    img.opcode(0x18)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "STACK_INC"),
        MicroAction{}.select(sm, "OP_OUT").select(sm, "STACK_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* PUSH */
    img.opcode(0x19)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "STACK_INC"),
        MicroAction{}.select(sm, "EN_OUT").select(sm, "STACK_IN"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* POP */
    img.opcode(0x1A)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "STACK_OUT").select(sm, "EN_IN"),
        MicroAction{}.select(sm, "STACK_DEC"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* SETZ */
    img.opcode(0x1B)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"), 0x8),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* SETC */
    img.opcode(0x1C)
      .from_step(5)
      .seq({
        with_alu_code(MicroAction{}.select(sm, "ALU_OUT").select(sm, "EN_IN"), 0x8),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* CLF */
    img.opcode(0x1D)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "ALU_FLAG_CLEAR"),
        MicroAction{}.select(sm, "COUNTER_EN"),
        MicroAction{}.set(sm, "STEP_RESET")
      });

    /* JMP */
    img.opcode(0x1E)
      .from_step(5)
      .seq({
        MicroAction{}.select(sm, "OP_OUT").select(sm, "JUMP"),
        MicroAction{}.set(sm, "STEP_RESET")
      });











    img.build(true);
    img.write("eeprom1.bin", "eeprom2.bin", "eeprom3.bin");
    write_alu_eeprom("alu_eeprom.bin");
}
