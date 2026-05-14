#ifndef NEW_TEST
#define NEW_TEST(...)
#endif

#define NEW_FIXED_INSTRUCTION(NAME, ...) NEW_TEST(NAME)
#include <inras/Macros/InstMacros.inc>
#undef NEW_FIXED_INSTRUCTION

NEW_TEST(Mov)
NEW_TEST(AddC)

#define DEFINE_TEST(NAME) int test##NAME()