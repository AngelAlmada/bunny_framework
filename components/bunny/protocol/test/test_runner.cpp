#include "test_framework.h"
#include <iostream>

void run_dedup_tests();
void run_type_converter_tests();
void run_bcp_envelope_tests();
void run_protocol_tests();

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "🐇 Bunny Framework - Protocol Test Suite" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    std::cout << "--- 1. DedupCache Unit Tests ---" << std::endl;
    run_dedup_tests();
    std::cout << std::endl;

    std::cout << "--- 2. TypeConverter Unit Tests ---" << std::endl;
    run_type_converter_tests();
    std::cout << std::endl;

    std::cout << "--- 3. BcpEnvelope Unit Tests ---" << std::endl;
    run_bcp_envelope_tests();
    std::cout << std::endl;

    std::cout << "--- 4. Protocol Dispatcher End-to-End Tests ---" << std::endl;
    run_protocol_tests();
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << bunny_test::g_tests_passed << " passed, "
              << bunny_test::g_tests_failed << " failed." << std::endl;
    std::cout << "========================================" << std::endl;

    return (bunny_test::g_tests_failed == 0) ? 0 : 1;
}
