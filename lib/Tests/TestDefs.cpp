#include <inras/Assembler/Assembler.h>
#include <inras/Defs/Addressing.h>
#include <inras/Defs/Defs.h>
#include <inras/Defs/Register.h>
#include <inras/Instruction/Instruction.h>

#include <cstring>
#include <iostream>

#include "../Tests/Tests.inc"

as::Assembler assembler;

#define DEFINE_FIXED_TEST(NAME, ENCSIZE, ...)                                  \
    DEFINE_TEST(NAME) {                                                        \
        as::Inst inst;                                                         \
        auto err = assembler.encode##NAME(inst);                               \
        if(err != decltype(err)()) {                                           \
            std::cout << #NAME ": assembler encountered an internal error\n";  \
            return false;                                                      \
        }                                                                      \
        if(inst.getEncodingSize() != ENCSIZE) {                                \
            std::cout << #NAME ": encoding did not match the expected size\n"; \
            return false;                                                      \
        }                                                                      \
        constexpr static as::byte expected[] = {__VA_ARGS__};                  \
        if(std::memcmp(inst.getEncoding(), expected, ENCSIZE)) {               \
            std::cout << #NAME                                                 \
                ": encoding did not match the expected encoding\n";            \
            std::cout << "instead got this encoding: " << inst << '\n';        \
            return false;                                                      \
        }                                                                      \
        std::cout << #NAME ": final encoding: " << inst << '\n';               \
        return true;                                                           \
    }

DEFINE_FIXED_TEST(Syscall, 2, 0x0F, 0x05)
DEFINE_FIXED_TEST(NearRet, 1, 0xC3)
DEFINE_FIXED_TEST(FarRet, 1, 0xCB)

#define ERR_STR(x)                                                           \
    if(x != decltype(x)()) {                                                 \
        std::cout << "Assembler error: " << as::Assembler::getErrcStr(x) << '\n'; \
        return false;                                                        \
    }

DEFINE_TEST(Mov) {
    as::Inst mov;
    auto err = assembler.encodeMov(
        mov, as::Addressing::DirectReg(as::regs::rax), as::regs::rbx, 64);
    ERR_STR(err)

    std::cout << "movq %rbx, %rax: " << mov << '\n';

    err = assembler.encodeMov(
        mov,
        as::Addressing::Disp32SIB(8, as::encode::SIB_4, as::regs::rax,
                                  as::regs::rbx),
        as::regs::r15, 64);
    ERR_STR(err)

    std::cout << "movq 8(%rax, %rbx, 4), %r15: " << mov << '\n';

    err = assembler.encodeMov(
        mov, as::regs::r15,
        as::Addressing::Disp32SIB(8, as::encode::SIB_4, as::regs::rax,
                                  as::regs::rbx),
        64);
    ERR_STR(err)

    std::cout << "movq %r15, 8(%rax, %rbx, 4): " << mov << '\n';

    err = assembler.encodeMov(mov, as::Addressing::DirectReg(as::regs::al), 42,
                              8);
    ERR_STR(err)

    std::cout << "movb $42, %al: " << mov << '\n';

    err = assembler.encodeMov(mov, as::Addressing::DirectReg(as::regs::rax),
                              -42, 64);
    ERR_STR(err)

    std::cout << "movq $-42, %rax: " << mov << '\n';

    err = assembler.encodeMov(mov, as::regs::rbp, 0xF0F0F0F0F0F0F0F0);
    ERR_STR(err)

    std::cout << "movabsq $0xF0F0F0F0F0F0F0F0, %rbp: " << mov << '\n';

    return true;
}