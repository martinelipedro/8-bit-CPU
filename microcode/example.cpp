// main.cpp (exemplo de uso com 3 EEPROMs)
#include "microcode.hpp"
using namespace ucode;

int main() {
    SignalMap sm;

    // EEPROM1 -> dois decoders 4->16
    sm.define_decoder_line("MI_LOAD_A",    DecoderSel::D1, 0);
    sm.define_decoder_line("MI_LOAD_B",    DecoderSel::D1, 1);
    sm.define_decoder_line("MI_ALU_OP",    DecoderSel::D2, 3);
    sm.define_decoder_line("MI_WB",        DecoderSel::D2, 7);

    // EEPROM2 -> bits diretos (E2)
    sm.define_bit("MAR_IN", DirectEeprom::E2, 0);
    sm.define_bit("IR_IN",  DirectEeprom::E2, 1);
    sm.define_bit("PC_INC", DirectEeprom::E2, 2);
    sm.define_bit("HLT",    DirectEeprom::E2, 3);

    // EEPROM3 -> bits diretos (E3)
    sm.define_bit("A_IN",   DirectEeprom::E3, 0);
    sm.define_bit("B_IN",   DirectEeprom::E3, 1);
    sm.define_bit("ALU_EN", DirectEeprom::E3, 2);
    sm.define_bit("OUT_EN", DirectEeprom::E3, 3);

    MicrocodeImage img;
    img.set_default_word(MicroWord{ .d1_line=0, .d2_line=0, .e2_bits=0, .e3_bits=0 });

    // Template global: step 0 e 1
    img.add_rule(
        Step(0), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
            .select(sm, "MI_LOAD_A")
            .set(sm, "MAR_IN")
            .set(sm, "PC_INC")
    );

    img.add_rule(
        Step(1), AnyOpcode(), AnyFlag(), AnyFlag(),
        MicroAction{}
            .select(sm, "MI_LOAD_B")
            .set(sm, "IR_IN")
    );

    // opcode 0x1A sequência a partir do step 2
    img.opcode(0x1A)
       .from_step(2)
       .seq({
            MicroAction{}.select(sm, "MI_ALU_OP").set(sm, "ALU_EN"),
            MicroAction{}.select(sm, "MI_WB").set(sm, "A_IN").clr(sm, "B_IN"),
            MicroAction{}.set(sm, "OUT_EN")
       });

    // HLT opcode 0x1F step 2..15
    img.add_rule(
        StepRange(2, 15), Op(0x1F), AnyFlag(), AnyFlag(),
        MicroAction{}.set(sm, "HLT")
    );

    img.build(true); // replica don't-care bits
    img.write("eeprom1.bin", "eeprom2.bin", "eeprom3.bin");
    return 0;
}
