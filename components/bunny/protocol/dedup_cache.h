#pragma once

#include <cstddef>
#include <cstdint>

namespace bunny {

class DedupCache {
public:
    static constexpr size_t DEFAULT_CAPACITY = 16;
    static constexpr size_t MAX_ID_LEN = 64;

    DedupCache();

    bool is_duplicate(const char* message_id) const;
    void remember(const char* message_id);
    void clear();

private:
    char _seen_ids[DEFAULT_CAPACITY][MAX_ID_LEN];
    size_t _count;
    size_t _next;
};

} // namespace bunny
