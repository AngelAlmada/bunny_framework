#pragma once

#include <iostream>
#include <string>
#include <cassert>

namespace bunny_test {

inline int g_tests_passed = 0;
inline int g_tests_failed = 0;

#define TEST_CASE(name) void name()

#define ASSERT_TRUE(condition) \
    do { \
        if (condition) { \
            ++bunny_test::g_tests_passed; \
        } else { \
            ++bunny_test::g_tests_failed; \
            std::cerr << "  ❌ FAIL: " #condition " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) == (b)) { \
            ++bunny_test::g_tests_passed; \
        } else { \
            ++bunny_test::g_tests_failed; \
            std::cerr << "  ❌ FAIL: " #a " == " #b " (Actual: " << (a) << " vs " << (b) << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)

#define ASSERT_STR_EQ(a, b) \
    do { \
        const char* _a = (a); \
        const char* _b = (b); \
        if (_a && _b && std::string(_a) == std::string(_b)) { \
            ++bunny_test::g_tests_passed; \
        } else { \
            ++bunny_test::g_tests_failed; \
            std::cerr << "  ❌ FAIL: '" << (_a ? _a : "NULL") << "' == '" << (_b ? _b : "NULL") << "' at " << __FILE__ << ":" << __LINE__ << std::endl; \
        } \
    } while (0)

#define RUN_TEST(fn) \
    do { \
        std::cout << "[RUN] " #fn << "..." << std::endl; \
        int before = bunny_test::g_tests_failed; \
        fn(); \
        if (bunny_test::g_tests_failed == before) { \
            std::cout << "  ✅ PASS: " #fn << std::endl; \
        } \
    } while (0)

} // namespace bunny_test
