#pragma once
#include "../metadata/metadata.h"
#include <cstdio>
#include <cstddef>

namespace bunny::json {

/**
 * Lightweight JSON builder helpers — no heap, no external deps.
 * All functions return the number of characters written (excl. null terminator).
 * Designed for fixed-size stack/static buffers on ESP32.
 */

inline size_t append(char* buf, size_t len, size_t& pos, const char* str) {
    if (!str) str = "";
    size_t written = 0;
    while (*str && pos < len - 1) {
        buf[pos++] = *str++;
        ++written;
    }
    buf[pos] = '\0';
    return written;
}

inline size_t serialize_metadata(char* buf, size_t len, const Metadata& m) {
    size_t pos = 0;

    // description
    if (m.description) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "\"description\":\"%s\"", m.description);
        append(buf, len, pos, tmp);
    }

    // returns
    {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), ",\"returns\":\"%s\"", type_name(m.returns_type));
        append(buf, len, pos, tmp);
    }

    // params
    if (m.param_count > 0) {
        append(buf, len, pos, ",\"params\":[");
        for (size_t i = 0; i < m.param_count; ++i) {
            if (i > 0) append(buf, len, pos, ",");
            char tmp[256];
            snprintf(tmp, sizeof(tmp),
                "{\"name\":\"%s\",\"type\":\"%s\",\"description\":\"%s\",\"required\":%s}",
                m.params[i].name        ? m.params[i].name        : "",
                type_name(m.params[i].type),
                m.params[i].description ? m.params[i].description : "",
                m.params[i].required    ? "true" : "false");
            append(buf, len, pos, tmp);
        }
        append(buf, len, pos, "]");
    }

    // tags
    if (m.tag_count > 0) {
        append(buf, len, pos, ",\"tags\":[");
        for (size_t i = 0; i < m.tag_count; ++i) {
            if (i > 0) append(buf, len, pos, ",");
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "\"%s\"", m.tags[i] ? m.tags[i] : "");
            append(buf, len, pos, tmp);
        }
        append(buf, len, pos, "]");
    }

    // affects
    if (m.affects_count > 0) {
        append(buf, len, pos, ",\"affects\":[");
        for (size_t i = 0; i < m.affects_count; ++i) {
            if (i > 0) append(buf, len, pos, ",");
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "\"%s\"", m.affects[i] ? m.affects[i] : "");
            append(buf, len, pos, tmp);
        }
        append(buf, len, pos, "]");
    }

    // example
    if (m.example) {
        char tmp[256];
        snprintf(tmp, sizeof(tmp), ",\"example\":\"%s\"", m.example);
        append(buf, len, pos, tmp);
    }

    return pos;
}

} // namespace bunny::json
