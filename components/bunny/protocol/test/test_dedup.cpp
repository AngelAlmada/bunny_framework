#include "test_framework.h"
#include "../dedup_cache.h"

TEST_CASE(test_dedup_initial_state)
{
    bunny::DedupCache cache;
    ASSERT_FALSE(cache.is_duplicate("msg_1"));
    ASSERT_FALSE(cache.is_duplicate(nullptr));
    ASSERT_FALSE(cache.is_duplicate(""));
}

TEST_CASE(test_dedup_remember_and_detect)
{
    bunny::DedupCache cache;
    cache.remember("msg_100");
    ASSERT_TRUE(cache.is_duplicate("msg_100"));
    ASSERT_FALSE(cache.is_duplicate("msg_101"));

    cache.remember("msg_101");
    ASSERT_TRUE(cache.is_duplicate("msg_100"));
    ASSERT_TRUE(cache.is_duplicate("msg_101"));
}

TEST_CASE(test_dedup_circular_buffer_overflow)
{
    bunny::DedupCache cache;
    // Fill beyond capacity (16)
    for (int i = 0; i < 20; ++i) {
        std::string id = "msg_" + std::to_string(i);
        cache.remember(id.c_str());
    }

    // The most recent ones should definitely exist
    ASSERT_TRUE(cache.is_duplicate("msg_19"));
    ASSERT_TRUE(cache.is_duplicate("msg_18"));
    ASSERT_TRUE(cache.is_duplicate("msg_10"));

    // The oldest should have been overwritten (0-3)
    ASSERT_FALSE(cache.is_duplicate("msg_0"));
    ASSERT_FALSE(cache.is_duplicate("msg_1"));
}

TEST_CASE(test_dedup_clear)
{
    bunny::DedupCache cache;
    cache.remember("msg_a");
    cache.remember("msg_b");
    ASSERT_TRUE(cache.is_duplicate("msg_a"));

    cache.clear();
    ASSERT_FALSE(cache.is_duplicate("msg_a"));
    ASSERT_FALSE(cache.is_duplicate("msg_b"));
}

void run_dedup_tests()
{
    RUN_TEST(test_dedup_initial_state);
    RUN_TEST(test_dedup_remember_and_detect);
    RUN_TEST(test_dedup_circular_buffer_overflow);
    RUN_TEST(test_dedup_clear);
}
