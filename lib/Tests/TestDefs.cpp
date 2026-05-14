#include <inras/Assembler/Assembler.h>
#include <inras/Defs/Addressing.h>
#include <inras/Defs/Defs.h>
#include <inras/Defs/Register.h>
#include <inras/Instruction/Instruction.h>

#include <iostream>

#include "../Tests/Tests.inc"

as::Assembler assembler;

#define SUCCESS 1
#define IGNORE 0
#define FAILURE -1

#define DEFINE_FIXED_TEST(NAME, ...)                                           \
    DEFINE_TEST(NAME) {                                                        \
        as::Inst inst;                                                         \
        auto err = assembler.encode##NAME(inst);                               \
        constexpr static as::byte expected[] = {__VA_ARGS__};                  \
        if(err != decltype(err)()) {                                           \
            std::cout << #NAME ": assembler encountered an error: "            \
                      << as::Assembler::getErrcStr(err) << '\n';               \
            if(err == as::AsmErrc::InstNotSupportedInMode) return IGNORE;      \
            return FAILURE;                                                    \
        }                                                                      \
        if(inst.getEncodingSize() != sizeof(expected)) {                       \
            std::cout << #NAME ": encoding did not match the expected size\n"; \
            return FAILURE;                                                    \
        }                                                                      \
        if(std::memcmp(inst.getEncoding(), expected, sizeof(expected))) {      \
            std::cout << #NAME                                                 \
                ": encoding did not match the expected encoding\n";            \
            std::cout << "instead got this encoding: " << inst << '\n';        \
            return FAILURE;                                                    \
        }                                                                      \
        std::cout << #NAME ": final encoding: " << inst << '\n';               \
        return SUCCESS;                                                        \
    }

#define NEW_FIXED_INSTRUCTION(IDENT, MODE, ...) \
    DEFINE_FIXED_TEST(IDENT, __VA_ARGS__)
#include <inras/Macros/InstMacros.inc>
#undef NEW_FIXED_INSTRUCTION

#define ERR_STR(x)                                                       \
    if(x != decltype(x)()) {                                             \
        std::cout << "Assembler error: " << as::Assembler::getErrcStr(x) \
                  << '\n';                                               \
        if(x == as::AsmErrc::InstNotSupportedInMode) return IGNORE;      \
        return FAILURE;                                                  \
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

    return SUCCESS;
}