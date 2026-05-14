#ifndef INRAS_INSTRUCTION_INSTRUCTION_H
#define INRAS_INSTRUCTION_INSTRUCTION_H

/// @file Instruction/Instruction.h
/// @brief This file contains an instruction class representing an x86
/// instruction.

#include <inras/Defs/Defs.h>

#include <cstring>
#include <ostream>

namespace as {

/// @brief Represents an x86 instruction.
///
/// x86 Instructions are at most 15 bytes in length.
class Inst {
public:
    constexpr static unsigned INSTRUCTION_SIZE = 16;

private:
    /// @brief Made for extra info about the instruction.
    ///
    /// An example of that "extra info" is index and size of the displacement
    /// for example jmp, call, or anything really with displacement.
    /// Higher 8 bits for the size and lower for index.
    uint16_t asmExtra_{};
    /// @brief Extra field that is not affected by the assembler and can be
    /// reused for anything.
    uint16_t userExtra_{};
    uint16_t encodingSize_;
    byte encoding_[INSTRUCTION_SIZE];

public:
    /// @brief Empty instruction constructor.
    Inst() noexcept = default;

    /// @brief The normal instruction constructor.
    /// @param encoding Encoding of the instruction.
    /// @param encodingSize Size of the encoding.
    Inst(const byte* encoding, unsigned encodingSize) noexcept :
        encodingSize_(encodingSize) {
        std::memcpy(encoding_, encoding, encodingSize);
    }

    /// @brief Constructor with the extra field.
    /// @param encoding Encoding of the instruction.
    /// @param encodingSize Size of the encoding.
    /// @param extra The extra field, can be used for various properties.
    Inst(const byte* encoding, unsigned encodingSize, unsigned asmExtra,
         unsigned userExtra) noexcept :
        asmExtra_(asmExtra),
        userExtra_(userExtra),
        encodingSize_(encodingSize) {
        std::memcpy(encoding_, encoding, encodingSize);
    }

    /// @brief An alternative to the constructor.
    void setEncoding(const byte* encoding, unsigned encodingSize) noexcept {
        encodingSize_ = encodingSize;
        std::memcpy(encoding_, encoding, encodingSize);
    }

    /// @brief Sets the encoding and the extra field.
    void setEncoding(const byte* encoding, unsigned encodingSize,
                     unsigned asmExtra, unsigned userExtra) noexcept {
        setEncoding(encoding, encodingSize);
        asmExtra_ = asmExtra;
        userExtra_ = userExtra;
    }

    /// @brief Gets the encoding of this instruction.
    const byte* getEncoding() const noexcept {
        return encoding_;
    }

    byte* getEncoding() noexcept {
        return encoding_;
    }

    void setEncodingSize(unsigned size) noexcept {
        encodingSize_ = size;
    }

    /// @brief Gets the size of the encoding.
    unsigned getEncodingSize() const noexcept {
        return encodingSize_;
    }

    /// @brief Gets the assembler metadata field.
    unsigned getAsmExtra() const noexcept {
        return asmExtra_;
    }

    /// @brief Only reliable when known this has a displacement.
    unsigned getDispIndex() const noexcept {
        return asmExtra_ & 0xFF;
    }

    void setDispIndex(uint8_t idx) noexcept {
        (asmExtra_ &= 0x00FF) |= idx;
    }

    void setDispSize(uint8_t size) noexcept {
        (asmExtra_ &= 0xFF00) |= (uint16_t(size) << 8);
    }

    /// @brief Only reliable when known this has a displacement.
    unsigned getDispSize() const noexcept {
        return asmExtra_ >> 8;
    }

    uint8_t* getDisp8() noexcept {
        return &encoding_[(asmExtra_ & 0xFF)];
    }

    uint32_t* getDisp32() noexcept {
        return (uint32_t*)(&encoding_[(asmExtra_ & 0xFF)]);
    }

    /// @brief Gets the user metadata field.
    unsigned getUserExtra() const noexcept {
        return userExtra_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Inst& inst) {
        const byte* data = inst.getEncoding();
        const unsigned size = inst.getEncodingSize();

        char out[(INSTRUCTION_SIZE * 2) + INSTRUCTION_SIZE]; // Hex + spaces.
        char* ptr = out;

        for(unsigned i = 0; i < size; i++) {
            if(i) {
                *ptr++ = ' ';
            }
            byte v = data[i];

            byte hi = v >> 4;
            byte lo = v & 0x0F;

            *ptr++ = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
            *ptr++ = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
        }

        return os.write(out, ptr - out);
    }

    /// @brief Writes the encoding into the buffer.
    /// @param dest The buffer to write to.
    /// @return dest + getEncodingSize().
    byte* writeInto(byte* dest) const noexcept {
        std::memcpy(dest, encoding_, encodingSize_);
        return dest + encodingSize_;
    }
};

} // namespace as

#endif // INRAS_INSTRUCTION_INSTRUCTION_H
