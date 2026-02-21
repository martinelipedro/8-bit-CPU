// microcode.hpp  (3 EEPROMs: 1x "two decoders" + 2x direct 8-bit)
// - EEPROM1: drives two 4->16 decoders (D1 and D2) via two nibbles
// - EEPROM2: direct 8-bit outputs
// - EEPROM3: direct 8-bit outputs
//
// Address space: 14-bit (16384). Layout default:
//   STEP  : bits 0..3 (4 bits)
//   OPCODE: bits 4..8 (5 bits)
//   ZF    : bit 9
//   CF    : bit 10
//   DC    : bits 11..13 replicated by default

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <optional>
#include <initializer_list>

namespace ucode {

// =========================
// Raw write helper
// =========================
inline void write_raw_binary(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Failed to open output: " + path);
    f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!f) throw std::runtime_error("Failed to write output: " + path);
}

// =========================
// Address packing
// =========================
struct AddressLayout {
    uint8_t step_lsb   = 0;  // width 4
    uint8_t opcode_lsb = 4;  // width 5
    uint8_t zf_bit     = 9;
    uint8_t cf_bit     = 10;

    // remaining bits are don't-care by default
    std::vector<uint8_t> dontcare_bits = {11, 12, 13};

    uint16_t pack(uint8_t step, uint8_t opcode, bool zf, bool cf, uint8_t dontcare_value = 0) const {
        if (step > 15) throw std::runtime_error("step out of range (0..15)");
        if (opcode > 31) throw std::runtime_error("opcode out of range (0..31)");

        uint16_t a = 0;
        a |= (uint16_t(step)   & 0x0Fu) << step_lsb;
        a |= (uint16_t(opcode) & 0x1Fu) << opcode_lsb;
        a |= (zf ? 1u : 0u) << zf_bit;
        a |= (cf ? 1u : 0u) << cf_bit;

        for (size_t i = 0; i < dontcare_bits.size(); ++i) {
            const uint8_t bitpos = dontcare_bits[i];
            const uint8_t bit = (dontcare_value >> i) & 1u;
            a |= uint16_t(bit) << bitpos;
        }
        return a;
    }

    uint32_t dontcare_combinations() const {
        if (dontcare_bits.size() >= 31) throw std::runtime_error("too many dontcare bits");
        return 1u << dontcare_bits.size();
    }
};

// =========================
// Selector (wildcards / ranges)
// =========================
struct Range {
    uint32_t lo = 0;
    uint32_t hi = 0;
    bool contains(uint32_t v) const { return v >= lo && v <= hi; }
};

class Selector {
public:
    static Selector Any(uint32_t maxv) {
        Selector s(maxv);
        s.any_ = true;
        return s;
    }
    static Selector One(uint32_t maxv, uint32_t v) {
        Selector s(maxv);
        s.any_ = false;
        s.ranges_.push_back({v, v});
        return s;
    }
    static Selector RangeInc(uint32_t maxv, uint32_t lo, uint32_t hi) {
        Selector s(maxv);
        s.any_ = false;
        if (lo > hi) std::swap(lo, hi);
        s.ranges_.push_back({lo, hi});
        return s;
    }

    Selector() = default;

    bool matches(uint32_t v) const {
        if (v > max_) return false;
        if (any_) return true;
        for (auto &r : ranges_) if (r.contains(v)) return true;
        return false;
    }

    Selector& add_range(uint32_t lo, uint32_t hi) {
        if (lo > hi) std::swap(lo, hi);
        ranges_.push_back({lo, hi});
        any_ = false;
        return *this;
    }

private:
    explicit Selector(uint32_t maxv) : max_(maxv) {}
    uint32_t max_ = 0;
    bool any_ = false;
    std::vector<Range> ranges_;
};

// Convenience
inline Selector AnyStep()     { return Selector::Any(15); }
inline Selector AnyOpcode()   { return Selector::Any(31); }
inline Selector AnyFlag()     { return Selector::Any(1); }

inline Selector Step(uint32_t v)                 { return Selector::One(15, v); }
inline Selector StepRange(uint32_t a,uint32_t b) { return Selector::RangeInc(15, a, b); }
inline Selector Op(uint32_t v)                   { return Selector::One(31, v); }
inline Selector OpRange(uint32_t a,uint32_t b)   { return Selector::RangeInc(31, a, b); }
inline Selector Z(uint32_t v)                    { return Selector::One(1, v); }
inline Selector C(uint32_t v)                    { return Selector::One(1, v); }

// =========================
// Signal mapping (now supports 2 direct EEPROMs)
// =========================
enum class DecoderSel { D1, D2 };
enum class DirectEeprom { E2, E3 };

struct DecodedLine {
    DecoderSel decoder;
    uint8_t line; // 0..15
};

struct DirectBit {
    DirectEeprom which;
    uint8_t bit; // 0..7
};

class SignalMap {
public:
    // Direct bit: choose which direct EEPROM (E2 or E3)
    void define_bit(const std::string& name, DirectEeprom which, uint8_t bit) {
        if (bit > 7) throw std::runtime_error("bit out of range (0..7): " + name);
        if (bits_.count(name) || dec_.count(name)) throw std::runtime_error("signal redefined: " + name);
        bits_[name] = DirectBit{which, bit};
    }

    // Decoder line (EEPROM1)
    void define_decoder_line(const std::string& name, DecoderSel which, uint8_t line) {
        if (line > 15) throw std::runtime_error("decoder line out of range (0..15): " + name);
        if (bits_.count(name) || dec_.count(name)) throw std::runtime_error("signal redefined: " + name);
        dec_[name] = DecodedLine{which, line};
    }

    bool is_bit(const std::string& name) const { return bits_.count(name) != 0; }
    bool is_decoded(const std::string& name) const { return dec_.count(name) != 0; }

    DirectBit bit_info(const std::string& name) const {
        auto it = bits_.find(name);
        if (it == bits_.end()) throw std::runtime_error("unknown BIT signal: " + name);
        return it->second;
    }

    DecodedLine decoded_line(const std::string& name) const {
        auto it = dec_.find(name);
        if (it == dec_.end()) throw std::runtime_error("unknown DEC signal: " + name);
        return it->second;
    }

private:
    std::unordered_map<std::string, DirectBit> bits_;
    std::unordered_map<std::string, DecodedLine> dec_;
};

// =========================
// Microinstruction output model (3 bytes)
// =========================
struct MicroWord {
    // EEPROM1: two nibbles -> two decoders
    uint8_t d1_line = 0;
    uint8_t d2_line = 0;

    // EEPROM2 and EEPROM3: direct outputs
    uint8_t e2_bits = 0;
    uint8_t e3_bits = 0;

    uint8_t eeprom1_byte() const { return uint8_t((d2_line << 4) | (d1_line & 0x0F)); }
    uint8_t eeprom2_byte() const { return e2_bits; }
    uint8_t eeprom3_byte() const { return e3_bits; }
};

// =========================
// Action: partial override on a MicroWord
// =========================
class MicroAction {
public:
    // Choose decoder line (EEPROM1)
    MicroAction& select_decoder(DecoderSel which, uint8_t line) {
        if (line > 15) throw std::runtime_error("decoder line out of range");
        if (which == DecoderSel::D1) { use_d1_ = true; d1_ = line; }
        else                         { use_d2_ = true; d2_ = line; }
        return *this;
    }

    // Select named DEC signal
    MicroAction& select(const SignalMap& sm, const std::string& decoded_sig) {
        if (!sm.is_decoded(decoded_sig)) throw std::runtime_error("signal is not a DEC: " + decoded_sig);
        auto dl = sm.decoded_line(decoded_sig);
        return select_decoder(dl.decoder, dl.line);
    }

    // Set/clear named BIT signal (EEPROM2 or EEPROM3)
    MicroAction& set(const SignalMap& sm, const std::string& sig) {
        if (!sm.is_bit(sig)) throw std::runtime_error("signal is not a BIT: " + sig);
        auto bi = sm.bit_info(sig);
        if (bi.which == DirectEeprom::E2) set2_mask_ |= uint8_t(1u << bi.bit);
        else                              set3_mask_ |= uint8_t(1u << bi.bit);
        return *this;
    }

    MicroAction& clr(const SignalMap& sm, const std::string& sig) {
        if (!sm.is_bit(sig)) throw std::runtime_error("signal is not a BIT: " + sig);
        auto bi = sm.bit_info(sig);
        if (bi.which == DirectEeprom::E2) clr2_mask_ |= uint8_t(1u << bi.bit);
        else                              clr3_mask_ |= uint8_t(1u << bi.bit);
        return *this;
    }

    // Raw masks (optional)
    MicroAction& set_bits_e2(uint8_t mask) { set2_mask_ |= mask; return *this; }
    MicroAction& clr_bits_e2(uint8_t mask) { clr2_mask_ |= mask; return *this; }
    MicroAction& set_bits_e3(uint8_t mask) { set3_mask_ |= mask; return *this; }
    MicroAction& clr_bits_e3(uint8_t mask) { clr3_mask_ |= mask; return *this; }

    MicroWord apply(MicroWord base) const {
        if (use_d1_) base.d1_line = d1_;
        if (use_d2_) base.d2_line = d2_;

        // E2: clear then set
        base.e2_bits &= uint8_t(~clr2_mask_);
        base.e2_bits |= set2_mask_;

        // E3: clear then set
        base.e3_bits &= uint8_t(~clr3_mask_);
        base.e3_bits |= set3_mask_;

        return base;
    }

private:
    bool use_d1_ = false, use_d2_ = false;
    uint8_t d1_ = 0, d2_ = 0;

    uint8_t set2_mask_ = 0, clr2_mask_ = 0;
    uint8_t set3_mask_ = 0, clr3_mask_ = 0;
};

// =========================
// Rule
// =========================
struct Rule {
    Selector step = AnyStep();
    Selector op   = AnyOpcode();
    Selector zf   = AnyFlag();
    Selector cf   = AnyFlag();
    MicroAction action;
};

// =========================
// Sequencing helper (opcode builder)
// =========================
struct SeqFlags {
    std::optional<uint8_t> zf; // 0 or 1; unset => Any
    std::optional<uint8_t> cf; // 0 or 1; unset => Any
};

inline Selector SelFromOptFlag(const std::optional<uint8_t>& f) {
    if (!f.has_value()) return AnyFlag();
    if (*f > 1) throw std::runtime_error("flag must be 0 or 1");
    return Selector::One(1, *f);
}

// =========================
// Microcode image builder (3 EEPROM images)
// =========================
class MicrocodeImage {
public:
    class OpcodeBuilder {
    public:
        OpcodeBuilder(MicrocodeImage& img, uint32_t opcode) : img_(img), opcode_(opcode) {
            if (opcode_ > 31) throw std::runtime_error("opcode out of range (0..31)");
        }

        OpcodeBuilder& z(uint8_t v) { if (v>1) throw std::runtime_error("z must be 0/1"); flags_.zf = v; return *this; }
        OpcodeBuilder& c(uint8_t v) { if (v>1) throw std::runtime_error("c must be 0/1"); flags_.cf = v; return *this; }
        OpcodeBuilder& flags_any()  { flags_.zf.reset(); flags_.cf.reset(); return *this; }

        OpcodeBuilder& from_step(uint32_t s) { start_step_ = s; return *this; }

        void seq(std::initializer_list<MicroAction> actions) {
            require_start();
            img_.add_sequence(*start_step_, opcode_, flags_, std::vector<MicroAction>(actions));
        }
        void seq(const std::vector<MicroAction>& actions) {
            require_start();
            img_.add_sequence(*start_step_, opcode_, flags_, actions);
        }

    private:
        void require_start() const {
            if (!start_step_.has_value()) throw std::runtime_error("OpcodeBuilder: call from_step() first");
        }

        MicrocodeImage& img_;
        uint32_t opcode_;
        SeqFlags flags_;
        std::optional<uint32_t> start_step_;
    };

public:
    explicit MicrocodeImage(AddressLayout layout = {})
        : layout_(std::move(layout))
    {
        constexpr size_t SIZE = 1u << 14;
        e1_.assign(SIZE, 0x00);
        e2_.assign(SIZE, 0x00);
        e3_.assign(SIZE, 0x00);
        default_word_ = MicroWord{};
    }

    void set_default_word(const MicroWord& w) { default_word_ = w; }

    void add_rule(const Rule& r) { rules_.push_back(r); }

    void add_rule(Selector step, Selector op, Selector zf, Selector cf, const MicroAction& action) {
        Rule r;
        r.step = step; r.op = op; r.zf = zf; r.cf = cf; r.action = action;
        rules_.push_back(r);
    }

    void add_sequence(uint32_t start_step, uint32_t opcode, SeqFlags flags,
                      const std::vector<MicroAction>& actions)
    {
        if (start_step > 15) throw std::runtime_error("start_step out of range (0..15)");
        if (opcode > 31) throw std::runtime_error("opcode out of range (0..31)");
        if (start_step + actions.size() > 16) throw std::runtime_error("sequence exceeds step range 0..15");

        Selector zsel = SelFromOptFlag(flags.zf);
        Selector csel = SelFromOptFlag(flags.cf);

        for (size_t i = 0; i < actions.size(); ++i) {
            add_rule(
                Step(uint32_t(start_step + i)),
                Op(opcode),
                zsel,
                csel,
                actions[i]
            );
        }
    }

    OpcodeBuilder opcode(uint32_t op) { return OpcodeBuilder(*this, op); }

    void build(bool replicate_dontcare = true) {
        const uint8_t def1 = default_word_.eeprom1_byte();
        const uint8_t def2 = default_word_.eeprom2_byte();
        const uint8_t def3 = default_word_.eeprom3_byte();
        std::fill(e1_.begin(), e1_.end(), def1);
        std::fill(e2_.begin(), e2_.end(), def2);
        std::fill(e3_.begin(), e3_.end(), def3);

        const uint32_t dcN = replicate_dontcare ? layout_.dontcare_combinations() : 1u;

        for (const auto& r : rules_) {
            for (uint32_t step = 0; step <= 15; ++step) if (r.step.matches(step)) {
                for (uint32_t op = 0; op <= 31; ++op) if (r.op.matches(op)) {
                    for (uint32_t z = 0; z <= 1; ++z) if (r.zf.matches(z)) {
                        for (uint32_t c = 0; c <= 1; ++c) if (r.cf.matches(c)) {
                            for (uint32_t dc = 0; dc < dcN; ++dc) {
                                const uint16_t addr = layout_.pack(
                                    (uint8_t)step, (uint8_t)op, z!=0, c!=0, (uint8_t)dc
                                );

                                MicroWord cur;
                                cur.d1_line = uint8_t(e1_[addr] & 0x0F);
                                cur.d2_line = uint8_t((e1_[addr] >> 4) & 0x0F);
                                cur.e2_bits = e2_[addr];
                                cur.e3_bits = e3_[addr];

                                MicroWord out = r.action.apply(cur);

                                e1_[addr] = out.eeprom1_byte();
                                e2_[addr] = out.eeprom2_byte();
                                e3_[addr] = out.eeprom3_byte();
                            }
                        }
                    }
                }
            }
        }
    }

    const std::vector<uint8_t>& eeprom1() const { return e1_; }
    const std::vector<uint8_t>& eeprom2() const { return e2_; }
    const std::vector<uint8_t>& eeprom3() const { return e3_; }

    void write(const std::string& eeprom1_path,
               const std::string& eeprom2_path,
               const std::string& eeprom3_path) const
    {
        write_raw_binary(eeprom1_path, e1_);
        write_raw_binary(eeprom2_path, e2_);
        write_raw_binary(eeprom3_path, e3_);
    }

private:
    AddressLayout layout_;
    MicroWord default_word_;
    std::vector<Rule> rules_;
    std::vector<uint8_t> e1_, e2_, e3_;
};

} // namespace ucode
