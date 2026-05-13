#ifndef INRAS_DEFS_REGISTER_H
#define INRAS_DEFS_REGISTER_H

/// @file Defs/Register.h
/// @brief Defines the registers available.

#include <inras/Defs/Defs.h>

namespace as {

enum class regs : unsigned {
#define NEW_REGISTER(IDENT, ...) IDENT,
#include <inras/Macros/RegMacros.inc>
#undef NEW_REGISTER
};

constexpr byte getEncoding(regs reg) {
    switch(reg) {
        case regs::al:
        case regs::ah:
        case regs::ax:
        case regs::eax:
        case regs::rax:
        case regs::r8b:
        case regs::r8w:
        case regs::r8d:
        case regs::r8:
            return 0b000;
        case regs::cl:
        case regs::ch:
        case regs::cx:
        case regs::ecx:
        case regs::rcx:
        case regs::r9b:
        case regs::r9w:
        case regs::r9d:
        case regs::r9:
            return 0b001;
        case regs::dl:
        case regs::dh:
        case regs::dx:
        case regs::edx:
        case regs::rdx:
        case regs::r10b:
        case regs::r10w:
        case regs::r10d:
        case regs::r10:
            return 0b010;
        case regs::bl:
        case regs::bh:
        case regs::bx:
        case regs::ebx:
        case regs::rbx:
        case regs::r11b:
        case regs::r11w:
        case regs::r11d:
        case regs::r11:
            return 0b011;
        case regs::spl:
        case regs::sp:
        case regs::esp:
        case regs::rsp:
        case regs::r12b:
        case regs::r12w:
        case regs::r12d:
        case regs::r12:
            return 0b100;
        case regs::bpl:
        case regs::bp:
        case regs::ebp:
        case regs::rbp:
        case regs::r13b:
        case regs::r13w:
        case regs::r13d:
        case regs::r13:
            return 0b101;
        case regs::sil:
        case regs::si:
        case regs::esi:
        case regs::rsi:
        case regs::r14b:
        case regs::r14w:
        case regs::r14d:
        case regs::r14:
            return 0b110;
        case regs::dil:
        case regs::di:
        case regs::edi:
        case regs::rdi:
        case regs::r15b:
        case regs::r15w:
        case regs::r15d:
        case regs::r15:
            return 0b111;
    }
}

/// @brief Returns true if this register represents high 8 bits of a 16bit
/// register.
constexpr static bool isHighBits(regs reg) {
    switch(reg) {
        case regs::ah:
        case regs::bh:
        case regs::ch:
        case regs::dh:
            return true;
        default:
            return false;
    }
}

constexpr unsigned getBits(regs reg) {
    switch(reg) {
#define NEW_REGISTER(IDENT, BITS, ...) \
    case regs::IDENT:                  \
        return BITS;
#include <inras/Macros/RegMacros.inc>
#undef NEW_REGISTER
    }
}

/// @brief Checks whether or not the register is supported in that mode.
constexpr static bool supportedReg(regs reg, Mode mode) {
    switch(reg) {
#define NEW_REGISTER(IDENT, BITS, MODE, ...) \
    case regs::IDENT:                        \
        return (MODE) & byte(mode);
#include <inras/Macros/RegMacros.inc>
#undef NEW_REGISTER
    }
}

constexpr static bool needsRex8Bit(regs reg) {
    switch(reg) {
        case regs::sil:
        case regs::dil:
        case regs::spl:
        case regs::bpl:
            return true;
        default:
            return false;
    }
}

constexpr static bool needsRexBitExt(regs reg) {
    switch(reg) {
        case regs::r8b:
        case regs::r8w:
        case regs::r8d:
        case regs::r8:
        case regs::r9b:
        case regs::r9w:
        case regs::r9d:
        case regs::r9:
        case regs::r10b:
        case regs::r10w:
        case regs::r10d:
        case regs::r10:
        case regs::r11b:
        case regs::r11w:
        case regs::r11d:
        case regs::r11:
        case regs::r12b:
        case regs::r12w:
        case regs::r12d:
        case regs::r12:
        case regs::r13b:
        case regs::r13w:
        case regs::r13d:
        case regs::r13:
        case regs::r14b:
        case regs::r14w:
        case regs::r14d:
        case regs::r14:
        case regs::r15b:
        case regs::r15w:
        case regs::r15d:
        case regs::r15:
            return true;
        default:
            return needsRex8Bit(reg);
    }
}

constexpr static bool highBitEncoding(byte enc) {
    switch(enc) {
        case getEncoding(regs::ah):
        case getEncoding(regs::bh):
        case getEncoding(regs::ch):
        case getEncoding(regs::dh):
            return true;
        default:
            return false;
    }
}

} // namespace as

#endif // INRAS_DEFS_REGISTER_H
