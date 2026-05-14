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

// Should this be a test case?
//
// constexpr static as::regs TestRegs[4][16] = {
//     {as::regs::al, as::regs::bl, as::regs::cl, as::regs::dl, as::regs::sil,
//      as::regs::dil, as::regs::spl, as::regs::bpl, as::regs::r8b,
//      as::regs::r9b, as::regs::r10b, as::regs::r11b, as::regs::r12b,
//      as::regs::r13b, as::regs::r14b, as::regs::r15b},
//     {as::regs::ax, as::regs::bx, as::regs::cx, as::regs::dx, as::regs::si,
//      as::regs::di, as::regs::sp, as::regs::bp, as::regs::r8w, as::regs::r9w,
//      as::regs::r10w, as::regs::r11w, as::regs::r12w, as::regs::r13w,
//      as::regs::r14w, as::regs::r15w},
//     {as::regs::eax, as::regs::ebx, as::regs::ecx, as::regs::edx,
//     as::regs::esi,
//      as::regs::edi, as::regs::esp, as::regs::ebp, as::regs::r8d,
//      as::regs::r9d, as::regs::r10d, as::regs::r11d, as::regs::r12d,
//      as::regs::r13d, as::regs::r14d, as::regs::r15d},
//     {as::regs::rax, as::regs::rbx, as::regs::rcx, as::regs::rdx,
//     as::regs::rsi,
//      as::regs::rdi, as::regs::rsp, as::regs::rbp, as::regs::r8, as::regs::r9,
//      as::regs::r10, as::regs::r11, as::regs::r12, as::regs::r13,
//      as::regs::r14, as::regs::r15}};

// int testExample() {
//     as::Inst inst;

//     for(unsigned size = 0; size < 4; size++) {
//         const as::regs* regArray = TestRegs[size];
//         unsigned addrSize;
//         switch(size) {
//             case 0:
//                 addrSize = 8;
//                 break;
//             case 1:
//                 addrSize = 16;
//                 break;
//             case 2:
//                 addrSize = 32;
//                 break;
//             case 3:
//                 addrSize = 64;
//                 break;
//         }

//         for(unsigned i = 0; i < 16; i++) {
//             for(unsigned j = 0; j < 16; j++) {
//                 auto err = assembler.encodeAdd(
//                     inst, regArray[i],
//                     as::Addressing::DirectReg(regArray[j]), addrSize);
//                 std::cout << "dest: " << as::getRegAsStr(regArray[i])
//                           << ", src: " << as::getRegAsStr(regArray[j]);
//                 if(err != as::AsmErrc()) {
//                     std::cout << ", error: " <<
//                     as::Assembler::getErrcStr(err)
//                               << '\n';
//                     continue;
//                 }
//                 std::cout << ", enc: " << inst << '\n';
//             }
//         }
//     }

//     return SUCCESS;
// }

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

DEFINE_TEST(AddC) {
    std::cout << "testing normal addc...\n";
    as::Inst addc;

    auto err = assembler.encodeAddC(
        addc, as::Addressing::DirectReg(as::regs::eax), as::regs::esp, 32);
    ERR_STR(err)

    std::cout << "addc %esp, %eax: " << addc << '\n';

    std::cout << "testing ax exception...\n";

    err = assembler.encodeAddC(addc, as::Addressing::DirectReg(as::regs::rax),
                               42, 64);
    ERR_STR(err)

    std::cout << "addc $42, %rax: " << addc << '\n';

    std::cout << "testing no exception imm...\n";

    err = assembler.encodeAddC(addc, as::Addressing::DirectReg(as::regs::rbx),
                               42, 64);
    ERR_STR(err)

    std::cout << "addc $42, %rbx: " << addc << '\n';

    return SUCCESS;
}

#if defined(__linux__) && defined(__x86_64__)
#include <sys/mman.h>
#endif

[[noreturn]]
DEFINE_TEST(MMapExec) {
#if !defined(__linux__) || !defined(__x86_64__)
    std::cout << "MMapExec: platform unsupported\n";
    std::exit(0);
#else
    as::byte* addr = (as::byte*)mmap(nullptr, 0x1000, PROT_WRITE | PROT_READ,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    /// movl $60, %eax
    /// movl $42, %edi
    /// syscall
    as::Inst mov, movedi, syscall;
    assembler.encodeMov(mov, as::regs::eax, 60);
    assembler.encodeMov(movedi, as::regs::edi, 42);
    assembler.encodeSyscall(syscall);

    syscall.writeInto(movedi.writeInto(mov.writeInto(addr)));

    std::cout << mov << ' ' << movedi << ' ' << syscall << std::endl; // flush

    mprotect(addr, 0x1000, PROT_EXEC | PROT_READ);

    ((void (*)())addr)();
    __builtin_unreachable();
#endif
}