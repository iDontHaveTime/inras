#include <inras/Assembler/Assembler.h>
#include <inras/Defs/Defs.h>
#include <inras/Defs/Register.h>

#include <cstring>

#include "inras/Defs/Addressing.h"
#include "inras/Instruction/Instruction.h"

namespace as {

std::string_view Assembler::getErrcStr(Errc errc) noexcept {
    switch(errc) {
        case Errc::Success:
            return "Success";
        case Errc::InstNotSupportedInMode:
            return "InstNotSupportedInMode";
        case Errc::RegNotSupportedInMode:
            return "RegNotSupportedInMode";
        case Errc::MismatchedOperandSizes:
            return "MismatchedOperandSizes";
        case Errc::AddressingSizeNotAllowedInMode:
            return "AddressingSizeNotAllowedInMode";
        case Errc::InvalidAddressSize:
            return "InvalidAddressSize";
        case Errc::MixingREXAndHighBit:
            return "MixingREXAndHighBit";
        case Errc::REXPrefixInNon64BitMode:
            return "REXPrefixInNon64BitMode";
        case Errc::RegisterNotAllowed:
            return "RegisterNotAllowed";
        case Errc::PrefixNotAddedNotEnoughSpace:
            return "PrefixNotAddedNotEnoughSpace";
    }
}

static inline bool checkModeCompat(Mode cur, byte srcmode) {
    return srcmode & byte(cur);
}

#define NEW_FIXED_INSTRUCTION(NAME, MODE, ENC)                                 \
    Assembler::Errc Assembler::encode##NAME(Inst& inst) const {                \
        if(!checkModeCompat(mode_, MODE)) return Errc::InstNotSupportedInMode; \
        constexpr static byte enc[] = {ENC};                                   \
        inst.setEncoding(enc, sizeof(enc));                                    \
        return Errc();                                                         \
    }

#include <inras/Macros/InstMacros.inc>

#undef NEW_FIXED_INSTRUCTION

Assembler::Errc Assembler::addPrefix(Inst& inst, byte p) {
    unsigned size = inst.getEncodingSize();
    if(size == Inst::INSTRUCTION_SIZE) {
        return Errc::PrefixNotAddedNotEnoughSpace;
    }
    byte* enc = inst.getEncoding();

    std::memmove(enc + 1, enc, size++);
    *enc = p;

    inst.setEncodingSize(size);
    return Errc();
}

static inline bool instSizeCheck(Mode mode, unsigned instSize) {
    switch(instSize) {
        case 8:
            return true;
        case 16:
            return true;
        case 32:
            return byte(mode) >= byte(Mode::i386);
        case 64:
            return byte(mode) >= byte(Mode::x86_64);
        default:
            return false;
    }
}

static inline Assembler::Errc check2OperandInst(Addressing addr, regs reg,
                                                Mode mode, bool& needsRex) {
    if(!supportedReg(reg, mode)) return Assembler::Errc::RegNotSupportedInMode;
    unsigned sizeOfInst = getBits(reg);
    needsRex = false;

    if(sizeOfInst == 64) {
        needsRex = true;
    }

    if(needsRexBitExt(reg)) {
        needsRex = true;
    }

    if(addr.isMemory()) {
        if(addr.RexBaseOrRM()) {
            needsRex = true;
        }
        else if(addr.needSIB() && addr.RexIndex()) {
            needsRex = true;
        }
    }
    else {
        if(sizeOfInst != addr.getRMSize())
            return Assembler::Errc::MismatchedOperandSizes;

        if(addr.RexBaseOrRM()) {
            needsRex = true;
        }
    }

    if(needsRex) {
        if(sizeOfInst == 8) {
            if(isHighBits(reg)) return Assembler::Errc::MixingREXAndHighBit;
            if(highBitEncoding(addr.getRM()) && !addr.RexBaseOrRM())
                return Assembler::Errc::MixingREXAndHighBit;
        }
        if(mode != Mode::x86_64)
            return Assembler::Errc::REXPrefixInNon64BitMode;
    }

    return Assembler::Errc();
}

static inline Assembler::Errc addASOPrefixIfNeeded(byte*& enc,
                                                   unsigned addrSize,
                                                   Mode mode) {
    switch(addrSize) {
        case 16:
            if(mode == Mode::x86_64) {
                return Assembler::Errc::AddressingSizeNotAllowedInMode;
            }
            else if(mode == Mode::i386) {
                *enc++ = prefix::ASO;
            }
            break;
        case 32:
            if(mode == Mode::x86_64) {
                *enc++ = prefix::ASO;
            }
            else if(mode == Mode::m16) {
                return Assembler::Errc::AddressingSizeNotAllowedInMode;
            }
            break;
        case 64:
            if(mode != Mode::x86_64)
                return Assembler::Errc::AddressingSizeNotAllowedInMode;
            break;
        default:
            return Assembler::Errc::InvalidAddressSize;
    }
    return Assembler::Errc();
}

static inline void addREXPrefix(byte*& enc, Addressing addr, regs reg,
                                unsigned instSize) {
    *enc++ = prefix::REX(instSize == 64, needsRexBitExt(reg),
                         (addr.needSIB() ? addr.RexIndex() : false),
                         addr.RexBaseOrRM());
}

static inline void addSIBAndDisp(Inst& inst, byte*& enc, Addressing addr) {
    if(addr.needSIB()) {
        *enc++ = addr.getSIB();
    }

    if(unsigned b = addr.getDispNBytes()) {
        inst.setDispIndex(enc - inst.getEncoding());
        inst.setDispSize(b);
        int32_t disp = inst.getDispSize();
        std::memcpy(enc, &disp, b);
        enc += b;
    }
}

Assembler::Errc Assembler::encodeGeneric2OpRMR(Inst& inst, Addressing dest,
                                               regs src, unsigned addrSize,
                                               byte opNorm, byte op8bit) const {
    bool needsRex;
    if(auto err = check2OperandInst(dest, src, mode_, needsRex);
       err != Errc()) {
        return err;
    }
    byte* enc = inst.getEncoding();

    unsigned instSize = getBits(src);
    if(!instSizeCheck(mode_, instSize)) {
        return Errc::InstNotSupportedInMode;
    }

    if(instSize == 8) {
        opNorm = op8bit;
    }

    if(byte(mode_) > byte(Mode::m16)) {
        if(instSize == 16) {
            *enc++ = prefix::OSO;
        }
    }

    if(auto err = addASOPrefixIfNeeded(enc, addrSize, mode_); err != Errc()) {
        return err;
    }

    if(needsRex) {
        addREXPrefix(enc, dest, src, instSize);
    }

    *enc++ = opNorm;

    byte modrm = dest.getModRM();
    encode::setReg(modrm, getEncoding(src));

    *enc++ = modrm;

    addSIBAndDisp(inst, enc, dest);

    inst.setEncodingSize(enc - inst.getEncoding());
    return Errc();
}

Assembler::Errc Assembler::encodeGeneric2OpRRM(Inst& inst, regs dest,
                                               Addressing src,
                                               unsigned addrSize, byte opNorm,
                                               byte op8bit) const {
    bool needsRex;
    if(auto err = check2OperandInst(src, dest, mode_, needsRex);
       err != Errc()) {
        return err;
    }
    byte* enc = inst.getEncoding();

    unsigned instSize = getBits(dest);
    if(!instSizeCheck(mode_, instSize)) {
        return Errc::InstNotSupportedInMode;
    }

    if(instSize == 8) {
        opNorm = op8bit;
    }

    if(byte(mode_) > byte(Mode::m16)) {
        if(instSize == 16) {
            *enc++ = prefix::OSO;
        }
    }

    if(auto err = addASOPrefixIfNeeded(enc, addrSize, mode_); err != Errc()) {
        return err;
    }

    if(needsRex) {
        addREXPrefix(enc, src, dest, instSize);
    }

    *enc++ = opNorm;

    byte modrm = src.getModRM();
    encode::setReg(modrm, getEncoding(dest));

    *enc++ = modrm;

    addSIBAndDisp(inst, enc, src);

    inst.setEncodingSize(enc - inst.getEncoding());
    return Errc();
}

Assembler::Errc Assembler::encodeGenericAddrImm(Inst& inst, Addressing dest,
                                                int32_t imm, unsigned addrSize,
                                                bool aso, byte opNorm,
                                                byte op8bit) const {
    byte* enc = inst.getEncoding();

    bool needsRex = false;
    unsigned immSize = 0;

    if(dest.isMemory()) {
        if(!instSizeCheck(mode_, addrSize)) {
            return Errc::InstNotSupportedInMode;
        }
        switch(addrSize) {
            case 8:
                opNorm = op8bit;
                immSize = 1;
                break;
            case 16:
                if(mode_ != Mode::m16) {
                    *enc++ = prefix::OSO;
                }
                immSize = 2;
                break;
            case 64:
                needsRex = true;
                [[fallthrough]];
            case 32:
                immSize = 4;
                break;
            default:
                return Errc::InvalidAddressSize;
        }
        if(dest.RexBaseOrRM()) {
            needsRex = true;
        }
        else if(dest.needSIB() && dest.RexIndex()) {
            needsRex = true;
        }
    }
    else {
        unsigned size = dest.getRMSize();
        if(!instSizeCheck(mode_, addrSize)) {
            return Errc::InstNotSupportedInMode;
        }

        switch(size) {
            case 8:
                opNorm = op8bit;
                immSize = 1;
                break;
            case 16:
                if(mode_ != Mode::m16) {
                    *enc++ = prefix::OSO;
                }
                immSize = 2;
                break;
            case 64:
                needsRex = true;
                [[fallthrough]];
            case 32:
                immSize = 4;
                break;
            default:
                return Errc::InvalidAddressSize;
        }

        if(dest.RexBaseOrRM()) {
            needsRex = true;
        }
        else if(size == 8 && highBitEncoding(dest.getRM())) {
            if(needsRex) return Errc::MixingREXAndHighBit;
        }
        addrSize = size;
    }

    if(aso) {
        *enc++ = prefix::ASO;
    }

    if(needsRex) {
        *enc++ =
            prefix::REX(addrSize, 0, (dest.needSIB() ? dest.RexIndex() : false),
                        dest.RexBaseOrRM());
    }

    *enc++ = opNorm;

    byte modrm = dest.getModRM();
    encode::setReg(modrm, 0);
    *enc++ = modrm;

    addSIBAndDisp(inst, enc, dest);

    std::memcpy(enc, &imm, immSize);
    enc += immSize;

    inst.setEncodingSize(enc - inst.getEncoding());
    return Errc();
}

Assembler::Errc Assembler::encodeMov(Inst& inst, regs dest, int64_t imm) const {
    unsigned size = getBits(dest);
    if(!instSizeCheck(mode_, size)) {
        return Errc::InstNotSupportedInMode;
    }
    byte* enc = inst.getEncoding();
    bool needsRex = needsRex8Bit(dest);
    unsigned immSize = 0;
    byte opcode = 0xB8;

    if(!needsRex) {
        // not high 8 bits
        if(needsRexBitExt(dest)) return Errc::RegisterNotAllowed;
    }

    switch(size) {
        case 8:
            immSize = 1;
            opcode = 0xB0;
            break;
        case 16:
            immSize = 2;
            if(mode_ != Mode::m16) {
                *enc++ = prefix::OSO;
            }
            break;
        case 32:
            immSize = 4;
            break;
        case 64:
            needsRex = true;
            immSize = 8;
            break;
        default:
            return Errc::InvalidAddressSize;
    }

    if(needsRex) {
        *enc++ = prefix::REX(size == 64, false, false, false);
    }

    opcode |= getEncoding(dest);
    *enc++ = opcode;

    std::memcpy(enc, &imm, immSize);
    enc += immSize;

    inst.setEncodingSize(enc - inst.getEncoding());
    return Errc();
}

} // namespace as