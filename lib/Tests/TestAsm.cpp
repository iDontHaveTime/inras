/// @file tests/TestAsm.cpp
/// @brief The main file for testing the assembler.
#include <algorithm>
#include <iostream>
#include <string_view>
#include <utility>
#include <vector>

#define NEW_TEST(Name) extern int test##Name(void);
#include "../Tests/Tests.inc"
#undef NEW_TEST

[[noreturn]] extern int testMMapExec(void);

int main(int argc, char** argv) {
    std::vector<std::string_view> argsVec;
    for(int i = 1; i < argc; i++) {
        argsVec.emplace_back(argv[i]);
    }

    bool success = true;
    using test_t = std::pair<std::string_view, int (*)(void)>;
#define NEW_TEST(Name) {#Name, test##Name},
    constexpr static test_t tests[] = {
// INCLUDES START HERE
#include "../Tests/Tests.inc"
        // INCLUDES END HERE
    };
#undef NEW_TEST

    for(const auto& test : tests) {
        int res = test.second();
        if(res == 1) {
            std::cout << test.first << ": success\n";
        }
        else if(res == -1) {
            std::cout << test.first << ": failure\n";
            success = false;
        }
        else {
            std::cout << test.first << ": ignored\n";
        }
    }

    if(auto it = std::find(argsVec.cbegin(), argsVec.cend(),
                           std::string_view("mmap"));
       it != argsVec.cend()) {
        testMMapExec();
    }

    return !success;
}