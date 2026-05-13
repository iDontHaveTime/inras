#ifndef INRAS_DEFS_ADDRESSING_H
#define INRAS_DEFS_ADDRESSING_H

/// @file Defs/Addressing.h
/// @brief Provides the memory operand class.

#include <inras/Defs/Defs.h>
#include <inras/Defs/Register.h>

#include <cstdint>

namespace as {

/// @brief Represents a class to abstract x86 r/m operand.
/// @note Assumes 32 or 64bit mode.
class Addressing {
    int32_t disp_ = 0;
    bit useSIB_ : 1 = 0b0;
    byte modEnc_ : 2 = 0b11;
    byte rmEnc_ : 3 = 0b000;
    byte scaleEnc_ : 2 = 0b00;
    byte indexEnc_ : 3 = 0b000;
    byte baseEnc_ : 3 = 0b000;
    bit indexRex_ : 1 = 0b0;
    bit rmOrBaseRex_ : 1 = 0b0;

    constexpr Addressing() noexcept = default;
    constexpr Addressing(int32_t disp, bit useSIB, byte modEnc, byte rmEnc,
                         byte scaleEnc, byte indexEnc, byte baseEnc,
                         bit indexRex, bit rmOrBaseRex) noexcept :
        disp_(disp),
        useSIB_(useSIB),
        modEnc_(modEnc),
        rmEnc_(rmEnc),
        scaleEnc_(scaleEnc),
        indexEnc_(indexEnc),
        baseEnc_(baseEnc),
        indexRex_(indexRex),
        rmOrBaseRex_(rmOrBaseRex) {}

    constexpr Addressing(byte modEnc, byte rmEnc, bit rmRex) noexcept :
        modEnc_(modEnc), rmEnc_(rmEnc), rmOrBaseRex_(rmRex) {}

    constexpr Addressing(int32_t disp, byte modEnc, byte rmEnc,
                         bit rmRex) noexcept :
        disp_(disp), modEnc_(modEnc), rmEnc_(rmEnc), rmOrBaseRex_(rmRex) {}

public:
    constexpr Addressing(const Addressing&) noexcept = default;
    constexpr Addressing(Addressing&&) noexcept = default;

    constexpr Addressing& operator=(const Addressing&) noexcept = default;
    constexpr Addressing& operator=(Addressing&&) noexcept = default;

    constexpr ~Addressing() noexcept = default;

    /// @brief Creates a new SIB byte addressing, with the base set as rbp.
    constexpr static Addressing NoBaseSIB(int32_t disp, byte scale,
                                          regs index = regs::rsp) {
        return Addressing(disp, true, 0b00, getEncoding(regs::rsp), scale,
                          getEncoding(index), getEncoding(regs::rbp),
                          needsRexBitExt(index), false);
    }

    /// @brief 8disp(%base + maybe index*scale).
    /// @param index When RSP it uses 8disp(%base) instead of the normal scale
    /// and index based.
    constexpr static Addressing Disp8SIB(int8_t disp, byte scale, regs base,
                                         regs index = regs::rsp) {
        return Addressing(disp, true, 0b01, getEncoding(regs::rsp), scale,
                          getEncoding(index), getEncoding(base),
                          needsRexBitExt(index), needsRexBitExt(base));
    }

    /// @brief Creates a new SIB byte addressing.
    constexpr static Addressing BaseSIB(byte scale, regs base,
                                        regs index = regs::rsp) {
        byte enc = getEncoding(base);
        if(enc == getEncoding(regs::rbp)) {
            return Disp8SIB(0, scale, regs::rbp, index);
        }
        return Addressing(0, true, 0b00, getEncoding(regs::rsp), scale,
                          getEncoding(index), enc, needsRexBitExt(index),
                          needsRexBitExt(base));
    }

    /// @brief 32disp(%base + maybe index*scale).
    /// @param index When RSP it uses 32disp(%base) instead of the normal scale
    /// and index based.
    constexpr static Addressing Disp32SIB(int32_t disp, byte scale, regs base,
                                          regs index = regs::rsp) {
        return Addressing(disp, true, 0b10, getEncoding(regs::rsp), scale,
                          getEncoding(index), getEncoding(base),
                          needsRexBitExt(index), needsRexBitExt(base));
    }

    /// @brief %reg.
    ///
    /// For example: %rax, %eax, %ax, %al.
    constexpr static Addressing DirectReg(regs reg) {
        byte scaleForSize = 0b00;
        switch(getBits(reg)) {
            case 16:
                scaleForSize = 0b01;
                break;
            case 32:
                scaleForSize = 0b10;
                break;
            case 64:
                scaleForSize = 0b11;
                break;
        }
        return Addressing(0, false, 0b11, getEncoding(reg), scaleForSize, 0, 0,
                          0, needsRexBitExt(reg));
    }

    /// @brief 8disp(%reg).
    ///
    /// For example: 16(%rax), 8(%rbx), -1(%rcx).
    /// Please make sure reg isn't %rsp or %r12 family.
    constexpr static Addressing Disp8Reg(int8_t disp, regs reg) {
        byte enc = getEncoding(reg);
        if(enc == getEncoding(regs::rsp)) {
            return Disp8SIB(disp, encode::SIB_1, regs::rsp);
        }
        else return Addressing(disp, 0b01, enc, needsRexBitExt(reg));
    }

    /// @brief 32disp(%reg).
    ///
    /// For example: 123(%rax), 8(%eax), -1(%ax).
    /// Please make sure reg isn't %rsp or %r12 family.
    constexpr static Addressing Disp32Reg(int8_t disp, regs reg) {
        byte enc = getEncoding(reg);
        if(enc == getEncoding(regs::rsp)) {
            return Disp32SIB(disp, encode::SIB_1, regs::rsp);
        }
        else return Addressing(disp, 0b10, enc);
    }

    /// @brief (%reg).
    ///
    /// For example: (%ebx), (%rax).
    constexpr static Addressing MemReg(regs reg) {
        byte enc = getEncoding(reg);
        if(enc == getEncoding(regs::rbp)) {
            return Disp8Reg(0, regs::rbp);
        }
        else if(enc == getEncoding(regs::rsp)) {
            return BaseSIB(encode::SIB_1, regs::rsp);
        }
        else return Addressing(0b00, enc, needsRexBitExt(reg));
    }

    /// @brief Just an disp32 encoding for 32bit mode.
    constexpr static Addressing Bit32ModeGlobalAddressign(int32_t addr) {
        return Addressing(addr, 0b00, getEncoding(regs::rbp));
    }

    /// @brief disp32(%rip).
    constexpr static Addressing RipRelative(int32_t disp) {
        return Bit32ModeGlobalAddressign(disp);
    }

    /// @brief Returns the ModR/M byte that matches the addressing.
    byte getModRM() const noexcept {
        return encode::ModRM(modEnc_, 0, rmEnc_);
    }

    /// @brief Returns the SIB byte if needed.
    byte getSIB() const noexcept {
        return encode::SIB(scaleEnc_, indexEnc_, baseEnc_);
    }

    /// @brief Returns whether or not the SIB byte is needed.
    bool needSIB() const noexcept {
        return useSIB_;
    }

    bool RexIndex() const noexcept {
        return indexRex_;
    }

    bool RexBaseOrRM() const noexcept {
        return rmOrBaseRex_;
    }

    /// @brief Returns the displacement.
    int32_t getDisp() const noexcept {
        return disp_;
    }

    /// @brief Only works reliably when !isMemory() is true.
    unsigned getRMSize() const noexcept {
#if defined(__has_builtin) && __has_builtin(__builtin_unreachable)
#define UNREACHABLE __builtin_unreachable()
#else
#define UNREACHABLE return 0
#endif
        switch(scaleEnc_) {
            case 0b00:
                return 8;
            case 0b01:
                return 16;
            case 0b10:
                return 32;
            case 0b11:
                return 64;
            default:
                UNREACHABLE;
        }
#undef UNREACHABLE
    }

    bool isMemory() const noexcept {
        return modEnc_ != 0b11;
    }

    /// @brief Returns how many bytes is the displacement.
    unsigned getDispNBytes() const noexcept {
        if(modEnc_ == 0b01) {
            return 1;
        }
        else if(modEnc_ == 0b10) {
            return 4;
        }
        else if(rmEnc_ == getEncoding(regs::rbp) && modEnc_ == 0b00) {
            return 4;
        }
        return 0;
    }

    byte getRM() const noexcept {
        return rmEnc_;
    }
};

} // namespace as

#endif // INRAS_DEFS_ADDRESSING_H
