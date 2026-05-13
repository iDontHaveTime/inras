#ifndef INRAS_DEFS_DEFS_H
#define INRAS_DEFS_DEFS_H

/// @file Defs/Defs.h
/// @brief Contains x86 definitions.

#include <cstdint>

namespace as {

/// @brief On x86 byte is always 8 bits.
using byte = uint8_t;
using bit =
    bool; ///< Doesn't represent a bit, rather mostly for syntax purposes

/// @brief Contains various x86 prefixes.
/// @note Source: https://wiki.osdev.org/X86-64_Instruction_Encoding
namespace prefix {

constexpr static byte LOCK = 0xF0; ///< Lock prefix.

constexpr static byte REPNE = 0xF2;  ///< REPNE prefix.
constexpr static byte REPNZ = REPNE; ///< REPNZ prefix.

constexpr static byte REP = 0xF3; ///< REP prefix.
constexpr static byte REPE = REP; ///< REPE prefix.
constexpr static byte REPZ = REP; ///< REPZ prefix.

constexpr static byte CS = 0x2E; ///< CS segment override.
constexpr static byte SS = 0x36; ///< SS segment override.
constexpr static byte DS = 0x3E; ///< DS segment override.
constexpr static byte ES = 0x26; ///< ES segment override.
constexpr static byte FS = 0x64; ///< FS segment override.
constexpr static byte GS = 0x65; ///< GS segment override.

constexpr static byte BNT = 0x2E; ///< Branch not taken.
constexpr static byte BT = 0x3E;  ///< Branch taken.

constexpr static byte OSO = 0x66; ///< Operand size override prefix.

constexpr static byte ASO = 0x67; ///< Address size override prefix.

constexpr static byte EMPTY_REX = 0x40;

/// @brief Creates a new REX prefix.
/// @param W True when 64bit operand size is used.
/// @param R True when extending ModRM's reg field.
/// @param X True when extending SIB's index field.
/// @param B True when extending ModRM's RM field or SIB's base field.
constexpr static byte REX(bit W, bit R, bit X, bit B) {
    return EMPTY_REX | (uint8_t(W) << 3) | (uint8_t(R) << 2) |
           (uint8_t(X) << 1) | B;
}

/// @brief Used when a register combination was not REX-able.
constexpr static byte INVALID_REX = ~0;

}; // namespace prefix

/// @brief Contains various encoding functions.
/// @note Source: https://wiki.osdev.org/X86-64_Instruction_Encoding
///
/// Technically for SIB encoding one could use _BitInt, but relying on an
/// extension isn't ideal, also it adds overhead that is unnecessary if you
/// trust the input is valid.
namespace encode {

/// SIB scale field factor 1.
constexpr static byte SIB_1 = 0b00;
/// SIB scale field factor 2.
constexpr static byte SIB_2 = 0b01;
/// SIB scale field factor 4.
constexpr static byte SIB_4 = 0b10;
/// SIB scale field factor 8.
constexpr static byte SIB_8 = 0b11;

/// @brief Creates a new SIB byte.
/// @param scale Scaling factor of the index field.
/// @param index Index register to use.
/// @param base Base register to use.
constexpr static byte SIB(byte scale, byte index, byte base) {
    return scale << 6 | index << 3 | base;
}

/// @brief Sets the SIB's scale part.
/// @param sib The byte to set.
/// @param scale Expects a `0bXX` format not `0bXX000000`.
constexpr static void setScale(byte& sib, byte scale) {
    (sib &= ~0xC0) |= (scale << 6);
}

/// @brief Sets the SIB's index part.
/// @param sib The byte to set.
/// @param index Expects a `0bXXX` format not `0b00XXX000`.
constexpr static void setIndex(byte& sib, byte index) {
    (sib &= ~0x38) |= (index << 3);
}

/// @brief Sets the SIB's base part.
/// @param sib The byte to set.
/// @param base Expects a valid base encoding.
constexpr static void setBase(byte& sib, byte base) {
    (sib &= ~0x07) |= base;
}

/// @brief Creates a new ModR/M byte.
/// @param mod Mod field.
/// @param reg Reg field.
/// @param rm R/M field.
///
/// More in depth on the `mod` and `r/m` fields:
/// The numbers before the colon are the values of the mod field.
/// ```
/// 0b00: (%rm)
/// 0b01: 8bit(%rm)
/// 0b10: 32bit(%rm)
/// 0b11: %rm
/// ```
/// Although there is an exception, when `r/m` is 0b100 it uses the SIB byte
/// instead.
/// In x86 (32bit) another exception is that when `r/m` is 0b101 on `mod` 0b00
/// it uses 32bit displacement and for `mod` 0b01 and on it uses the same rules
/// as stated above for the frame register (%rbp). The behavior above of 32bit
/// displacement on `r/m` being 0b101 and `mod` being 0b00 is utilized by
/// assemblers to allow direct addressing like:
/// ```
/// movl $0x07690748, 0xB8000
/// ```
/// In AMD64 the exception also occurs in the same place, being `r/m` 0b101 and
/// `mod` 0b00, although the behavior is changed. Instead of it being absolute
/// addressing, it becomes %rip relative one. So on 64bit mode the assembler
/// should use the SIB byte with no base for "global addressing".
/// But global addressing should be avoided in AMD64 so that use is discouraged.
constexpr static byte ModRM(byte mod, byte reg, byte rm) {
    return mod << 6 | reg << 3 | rm;
}

/// @brief Sets the r/m field of ModR/M.
/// @param modrm The byte to set it to.
/// @param rm Assuming a valid 3bit rm field.
constexpr static void setRM(byte& modrm, byte rm) {
    (modrm &= ~0x07) |= rm;
}

/// @brief Sets the reg field of ModR/M.
/// @param modrm The byte to set it to.
/// @param reg Assuming a valid 3bit reg field.
/// @note Reg is expected to be in `0b00000XXX` format not `0b00XXX000`.
constexpr static void setReg(byte& modrm, byte reg) {
    (modrm &= ~0x38) |= (reg << 3);
}

/// @brief Sets the mod field of ModR/M.
/// @param modrm The byte to set it to.
/// @param mod Assuming a valid 2bit mod field.
/// @note Same as setReg where format is `0b000000XX` not `0bXX000000`.
constexpr static void setMod(byte& modrm, byte mod) {
    (modrm &= ~0xC0) |= (mod << 6);
}

/// @brief Same as the others just sets both mod and r/m.
/// @note See functions like setMod() or setRM() for more info.
constexpr static void setModAndRM(byte& modrm, byte mod, byte rm) {
    (modrm &= ~0xC7) |= ((mod << 6) | rm);
}

} // namespace encode

/// @brief Represents an assembler mode.
enum class Mode : byte {
    m16 = 0x1,     ///< 16bit mode.
    i386 = 0x2,    ///< 32bit mode.
    x86 = i386,    ///< Alias to i386.
    x86_64 = 0x4,  ///< 64bit mode.
    AMD64 = x86_64 ///< Alias to x86_64.
};

} // namespace as

#endif // INRAS_DEFS_DEFS_H
