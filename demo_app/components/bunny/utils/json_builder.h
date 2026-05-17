#pragma once
#include "../metadata/metadata.h"
#include <cstdio>
#include <cstddef>

namespace bunny::json
{

    /**
     * Lightweight JSON builder helpers — no heap, no external deps.
     * All functions return the number of characters written (excl. null terminator).
     * Designed for fixed-size stack/static buffers on ESP32.
     */

    inline size_t append(char *buf, size_t len, size_t &pos, const char *str)
    {
        if (!str)
            str = "";
        size_t written = 0;
        while (*str && pos < len - 1)
        {
            buf[pos++] = *str++;
            ++written;
        }
        buf[pos] = '\0';
        return written;
    }

    inline size_t append_json_escaped(char *buf, size_t len, size_t &pos, const char *str)
    {
        if (!str)
            str = "";

        size_t written = 0;
        while (*str && pos < len - 1)
        {
            unsigned char ch = static_cast<unsigned char>(*str++);
            const char *escape = nullptr;
            char unicode_escape[7] = {};

            switch (ch)
            {
            case '"':
                escape = "\\\"";
                break;
            case '\\':
                escape = "\\\\";
                break;
            case '\b':
                escape = "\\b";
                break;
            case '\f':
                escape = "\\f";
                break;
            case '\n':
                escape = "\\n";
                break;
            case '\r':
                escape = "\\r";
                break;
            case '\t':
                escape = "\\t";
                break;
            default:
                if (ch < 0x20)
                {
                    snprintf(unicode_escape, sizeof(unicode_escape), "\\u%04x", ch);
                    escape = unicode_escape;
                }
                break;
            }

            if (escape)
            {
                written += append(buf, len, pos, escape);
            }
            else
            {
                buf[pos++] = static_cast<char>(ch);
                buf[pos] = '\0';
                ++written;
            }
        }

        return written;
    }

    inline void append_json_string_field(char *buf, size_t len, size_t &pos,
                                         const char *key, const char *value,
                                         bool with_leading_comma = false)
    {
        if (with_leading_comma)
            append(buf, len, pos, ",");
        append(buf, len, pos, "\"");
        append(buf, len, pos, key ? key : "");
        append(buf, len, pos, "\":\"");
        append_json_escaped(buf, len, pos, value ? value : "");
        append(buf, len, pos, "\"");
    }

    inline size_t serialize_metadata(char *buf, size_t len, const Metadata &m)
    {
        size_t pos = 0;

        // description
        if (m.description)
        {
            append_json_string_field(buf, len, pos, "description", m.description);
        }

        // returns
        {
            append_json_string_field(buf, len, pos, "returns", type_name(m.returns_type), true);
        }

        // params
        if (m.param_count > 0)
        {
            append(buf, len, pos, ",\"params\":[");
            for (size_t i = 0; i < m.param_count; ++i)
            {
                if (i > 0)
                    append(buf, len, pos, ",");
                append(buf, len, pos, "{");
                append_json_string_field(buf, len, pos, "name", m.params[i].name ? m.params[i].name : "");
                append_json_string_field(buf, len, pos, "type", type_name(m.params[i].type), true);
                append_json_string_field(buf, len, pos, "description", m.params[i].description ? m.params[i].description : "", true);
                append(buf, len, pos, ",\"required\":");
                append(buf, len, pos, m.params[i].required ? "true" : "false");
                append(buf, len, pos, "}");
            }
            append(buf, len, pos, "]");
        }

        // tags
        if (m.tag_count > 0)
        {
            append(buf, len, pos, ",\"tags\":[");
            for (size_t i = 0; i < m.tag_count; ++i)
            {
                if (i > 0)
                    append(buf, len, pos, ",");
                append(buf, len, pos, "\"");
                append_json_escaped(buf, len, pos, m.tags[i] ? m.tags[i] : "");
                append(buf, len, pos, "\"");
            }
            append(buf, len, pos, "]");
        }

        // affects
        if (m.affects_count > 0)
        {
            append(buf, len, pos, ",\"affects\":[");
            for (size_t i = 0; i < m.affects_count; ++i)
            {
                if (i > 0)
                    append(buf, len, pos, ",");
                append(buf, len, pos, "\"");
                append_json_escaped(buf, len, pos, m.affects[i] ? m.affects[i] : "");
                append(buf, len, pos, "\"");
            }
            append(buf, len, pos, "]");
        }

        // example
        if (m.example)
        {
            append_json_string_field(buf, len, pos, "example", m.example, true);
        }

        return pos;
    }

} // namespace bunny::json
