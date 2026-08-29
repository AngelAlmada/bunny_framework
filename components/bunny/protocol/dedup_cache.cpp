#include "dedup_cache.h"
#include <cstring>

namespace bunny {

DedupCache::DedupCache()
    : _count(0), _next(0)
{
    clear();
}

bool DedupCache::is_duplicate(const char* message_id) const
{
    if (!message_id || !message_id[0]) {
        return false;
    }

    for (size_t i = 0; i < _count; ++i) {
        if (strcmp(_seen_ids[i], message_id) == 0) {
            return true;
        }
    }
    return false;
}

void DedupCache::remember(const char* message_id)
{
    if (!message_id || !message_id[0]) {
        return;
    }

    strncpy(_seen_ids[_next], message_id, MAX_ID_LEN - 1);
    _seen_ids[_next][MAX_ID_LEN - 1] = '\0';

    _next = (_next + 1U) % DEFAULT_CAPACITY;
    if (_count < DEFAULT_CAPACITY) {
        ++_count;
    }
}

void DedupCache::clear()
{
    memset(_seen_ids, 0, sizeof(_seen_ids));
    _count = 0;
    _next = 0;
}

} // namespace bunny
