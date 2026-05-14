#ifndef NEW_FIXED_INSTRUCTION
#define NEW_FIXED_INSTRUCTION(...)
#endif

#define b16 byte(Mode::m16)
#define b32 byte(Mode::i386)
#define b64 byte(Mode::AMD64)

#define FIXED_ENCODING(...) __VA_ARGS__

/// syscall
NEW_FIXED_INSTRUCTION(Syscall, b64, FIXED_ENCODING(0x0F, 0x05))
/// ret
NEW_FIXED_INSTRUCTION(NearRet, b64 | b32 | b16, FIXED_ENCODING(0xC3))
/// lret
NEW_FIXED_INSTRUCTION(FarRet, b64 | b32 | b16, FIXED_ENCODING(0xCB))
/// aaa
NEW_FIXED_INSTRUCTION(ASCIIAdjustAfterAddition, b32 | b16, FIXED_ENCODING(0x37))
/// aad
NEW_FIXED_INSTRUCTION(ASCIIAdjustAXBeforeDivision, b32 | b16, FIXED_ENCODING(0xD5, 0x0A))
/// aam
NEW_FIXED_INSTRUCTION(ASCIIAdjustAXAfterMultiply, b32 | b16, FIXED_ENCODING(0xD4, 0x0A))
/// aas
NEW_FIXED_INSTRUCTION(ASCIIAdjustALAfterSubtraction, b32 | b16, FIXED_ENCODING(0x3F))



#undef b16
#undef b32
#undef b64