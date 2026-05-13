/// @file tests/TestAsm.cpp
/// @brief The main file for testing the assembler.
#include <iostream>
#include <string_view>
#include <utility>

#define NEW_TEST(Name) extern bool test##Name(void);
#include "../Tests/Tests.inc"
#undef NEW_TEST

int main() {
    bool success = true;
    using test_t = std::pair<std::string_view, bool (*)(void)>;
#define NEW_TEST(Name) {#Name, test##Name},
    constexpr static test_t tests[] = {
// INCLUDES START HERE
#include "../Tests/Tests.inc"
        // INCLUDES END HERE
    };
#undef NEW_TEST

    for(const auto& test : tests) {
        bool res = test.second();
        std::cout << test.first << ": " << (res ? "success\n" : "failure\n");
    }

    return !success;
}