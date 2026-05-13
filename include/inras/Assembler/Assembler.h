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
        RegisterNotAllowed ///< When a register that is not supported in that
                           ///< instruction was passed in.
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

#define NEW_FIXED_INSTRUCTION(NAME, MODE, ENC) Errc encode##NAME(Inst&) const;

#include <inras/Macros/InstMacros.inc>

#undef NEW_FIXED_INSTRUCTION

    /// @brief Encodes a mov %reg, %r/m
    /// @note addrSize is ignored when the addressing isn't memory.
    Errc encodeMov(Inst& inst, Addressing dest, regs src, unsigned addrSize) const;
    /// @brief Encodes a mov %r/m, %reg
    /// @note addrSize is ignored when the addressing isn't memory.
    Errc encodeMov(Inst& inst, regs dest, Addressing src, unsigned addrSize) const;
    /// @brief Encodes a mov $IMM, %r/m
    /// @note addrSize is ignored when the addressing isn't memory, here its
    /// used to determine immediate size as well.
    Errc encodeMov(Inst& inst, Addressing dest, int32_t imm, unsigned addrSize,
                   bool ASOprefix = false) const;
    Errc encodeMov(Inst& inst, regs dest, int64_t imm) const;
};

} // namespace as

#endif // INRAS_ASSEMBLER_ASSEMBLER_H
