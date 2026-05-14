#ifndef INRAS_ASSEMBLER_ASSEMBLER_H
#define INRAS_ASSEMBLER_ASSEMBLER_H

/// @file Assembler/Assembler.h
/// @brief The main class for assembling instructions.

#include <inras/Defs/Addressing.h>
#include <inras/Defs/Defs.h>
#include <inras/Defs/Register.h>
#include <inras/Instruction/Instruction.h>

#include <cassert>
#include <string_view>

namespace as {

/// @brief Provides functions to assemble instructions.
class Assembler {
    Mode mode_; ///< Current set mode.

public:
    enum class Errc {
        Success = 0x0,

        InstNotSupportedInMode, ///< Means the instruction is not supported in
                                ///< the current mode.
        RegNotSupportedInMode,  ///< Register is not supported in the current
                                ///< mode.
        MismatchedOperandSizes, ///< Operands don't match in size.
        AddressingSizeNotAllowedInMode, ///< The provided addressing size is
                                        ///< impossible in this mode.
        InvalidAddressSize,             ///< When addrSize is not 64, 32, or 16.
        MixingREXAndHighBit, ///< Happens when mixing REX prefix and high bit
                             ///< registers.
        REXPrefixInNon64BitMode, ///< When the REX prefix was used in non-64bit
                                 ///< mode.
        RegisterNotAllowed, ///< When a register that is not supported in that
                            ///< instruction was passed in.
        PrefixNotAddedNotEnoughSpace ///< Cannot add a prefix since the
                                     ///< instruction is too long for it.
    };

    static std::string_view getErrcStr(Errc errc) noexcept;

    /// @brief Creates an assembler class.
    Assembler(Mode mode = Mode::x86_64) noexcept : mode_(mode) {}

    /// @brief Returns the current mode of the assembler.
    Mode getMode() const noexcept {
        return mode_;
    }

    /// @brief Changes the mode of the assembler.
    void setMode(Mode mode) noexcept {
        mode_ = mode;
    }

    unsigned getDefaultAddrSize() const noexcept {
        switch(mode_) {
            case Mode::m16:
                return 16;
            case Mode::i386:
                return 32;
            case Mode::x86_64:
                return 64;
        }
    }

    /// @brief Inserts a prefix to an **already encoded** instruction.
    /// @note If you reincode the instruction you must call this again and add
    /// the prefixes.
    Errc addPrefix(Inst& inst, byte p);

    /// @brief Adds the lock prefix.
    Errc addLOCK(Inst& inst) {
        return addPrefix(inst, prefix::LOCK);
    }

    /// @brief Adds the REPNE prefix.
    Errc addREPNE(Inst& inst) {
        return addPrefix(inst, prefix::REPNE);
    }

    /// @brief Alias for addREPNE.
    Errc addREPNZ(Inst& inst) {
        return addREPNE(inst);
    }

    /// @brief Adds the REP prefix.
    Errc addREP(Inst& inst) {
        return addPrefix(inst, prefix::REP);
    }

    /// @brief Alias for addREP.
    Errc addREPE(Inst& inst) {
        return addREP(inst);
    }

    /// @brief Alias for addREP.
    Errc addREPZ(Inst& inst) {
        return addREP(inst);
    }

    /// @brief Adds the CS segment override prefix.
    Errc addCSSeg(Inst& inst) {
        return addPrefix(inst, prefix::CS);
    }

    /// @brief Adds the SS segment override prefix.
    Errc addSSSeg(Inst& inst) {
        return addPrefix(inst, prefix::SS);
    }

    /// @brief Adds the DS segment override prefix.
    Errc addDSSeg(Inst& inst) {
        return addPrefix(inst, prefix::DS);
    }

    /// @brief Adds the ES segment override prefix.
    Errc addESSeg(Inst& inst) {
        return addPrefix(inst, prefix::ES);
    }

    /// @brief Adds the FS segment override prefix.
    Errc addFSSeg(Inst& inst) {
        return addPrefix(inst, prefix::FS);
    }

    /// @brief Adds the GS segment override prefix.
    Errc addGSSeg(Inst& inst) {
        return addPrefix(inst, prefix::GS);
    }

#define NEW_FIXED_INSTRUCTION(NAME, MODE, ENC) Errc encode##NAME(Inst&) const;

#include <inras/Macros/InstMacros.inc>

#undef NEW_FIXED_INSTRUCTION

private:
    Errc encodeGeneric2OpRMR(Inst& inst, Addressing dest, regs src,
                             unsigned addrSize, byte opNorm, byte op8bit) const;
    Errc encodeGeneric2OpRRM(Inst& inst, regs dest, Addressing src,
                             unsigned addrSize, byte opNorm, byte op8bit) const;
    Errc encodeGenericAddrImm(Inst& inst, Addressing dest, int32_t imm,
                              unsigned addrSize, bool ASOprefix, byte opNorm,
                              byte op8bit) const;

public:
    /// @brief Encodes a mov %reg, %r/m
    /// @note addrSize is ignored when the addressing isn't memory.
    Errc encodeMov(Inst& inst, Addressing dest, regs src,
                   unsigned addrSize) const {
        return encodeGeneric2OpRMR(inst, dest, src, addrSize, 0x89, 0x88);
    }
    /// @brief Encodes a mov %r/m, %reg
    /// @note addrSize is ignored when the addressing isn't memory.
    Errc encodeMov(Inst& inst, regs dest, Addressing src,
                   unsigned addrSize) const {
        return encodeGeneric2OpRRM(inst, dest, src, addrSize, 0x8B, 0x8A);
    }
    /// @brief Encodes a mov $IMM, %r/m
    /// @note addrSize is ignored when the addressing isn't memory, here its
    /// used to determine immediate size as well.
    Errc encodeMov(Inst& inst, Addressing dest, int32_t imm, unsigned addrSize,
                   bool ASOprefix = false) const {
        return encodeGenericAddrImm(inst, dest, imm, addrSize, ASOprefix, 0xC7,
                                    0xC6);
    }
    Errc encodeMov(Inst& inst, regs dest, int64_t imm) const;
};

using AsmErrc = Assembler::Errc;

} // namespace as

#endif // INRAS_ASSEMBLER_ASSEMBLER_H
