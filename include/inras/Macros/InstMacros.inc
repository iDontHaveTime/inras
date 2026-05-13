#ifndef NEW_FIXED_INSTRUCTION
#define NEW_FIXED_INSTRUCTION(...)
#endif

#define b16 byte(Mode::m16)
#define b32 byte(Mode::i386)
#define b64 byte(Mode::AMD64)

#define FIXED_ENCODING(...) __VA_ARGS__

NEW_FIXED_INSTRUCTION(Syscall, b64, FIXED_ENCODING(0x0F, 0x05))
NEW_FIXED_INSTRUCTION(NearRet, b64 | b32 | b16, FIXED_ENCODING(0xC3))
NEW_FIXED_INSTRUCTION(FarRet, b64 | b32 | b16, FIXED_ENCODING(0xCB))

#undef b16
#undef b32
#undef b64