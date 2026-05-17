#include "command_capability.h"
#include "../utils/json_builder.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace bunny {

// ── Params ────────────────────────────────────────────────────────────────────

void Params::set(const char* key, const char* value) {
    if (_count >= MAX_ENTRIES) return;
    _entries[_count++] = {key, value};
}

const Params::Entry* Params::find(const char* key) const {
    for (size_t i = 0; i < _count; ++i) {
        if (_entries[i].key && key && strcmp(_entries[i].key, key) == 0) {
            return &_entries[i];
        }
    }
    return nullptr;
}

bool Params::has(const char* key) const {
    return find(key) != nullptr;
}

const char* Params::get_string(const char* key) const {
    const Entry* e = find(key);
    return e ? e->value : "";
}

double Params::get_number(const char* key) const {
    const Entry* e = find(key);
    return e && e->value ? atof(e->value) : 0.0;
}

bool Params::get_bool(const char* key) const {
    const Entry* e = find(key);
    if (!e || !e->value) return false;
    return strcmp(e->value, "true") == 0 || strcmp(e->value, "1") == 0;
}

// ── CommandCapability ─────────────────────────────────────────────────────────

CommandCapability::CommandCapability(const char* name, const Metadata& meta, CommandExecuteFn exec_fn)
    : _name(name), _meta(meta), _exec_fn(std::move(exec_fn)) {}

void CommandCapability::execute(const Params& p) const {
    if (_exec_fn) _exec_fn(p);
}

size_t CommandCapability::serialize(char* buf, size_t len) const {
    size_t pos = 0;
    char tmp[64];

    json::append(buf, len, pos, "{");
    snprintf(tmp, sizeof(tmp), "\"name\":\"%s\",\"kind\":\"command\",", _name ? _name : "");
    json::append(buf, len, pos, tmp);
    json::serialize_metadata(buf + pos, len - pos, _meta);
    pos = __builtin_strlen(buf);
    json::append(buf, len, pos, "}");
    return pos;
}

} // namespace bunny
