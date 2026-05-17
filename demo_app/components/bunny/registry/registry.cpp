#include "registry.h"
#include "../utils/json_builder.h"
#include <cstring>
#include <cstdio>

namespace bunny {

Registry& Registry::instance() {
    static Registry s_instance;
    return s_instance;
}

bool Registry::register_capability(ICapability* cap) {
    if (!cap || _count >= MAX_CAPABILITIES) return false;
    _caps[_count++] = cap;
    return true;
}

ICapability* Registry::find(const char* name, CapabilityKind kind) const {
    for (size_t i = 0; i < _count; ++i) {
        if (_caps[i]->kind() == kind && strcmp(_caps[i]->name(), name) == 0) {
            return _caps[i];
        }
    }
    return nullptr;
}

size_t Registry::serialize_capabilities(char* buf, size_t len) const {
    size_t pos = 0;

    json::append(buf, len, pos, "{");

    // Iterate per kind to group them
    const CapabilityKind kinds[] = {
        CapabilityKind::SENSOR,
        CapabilityKind::COMMAND,
        CapabilityKind::EVENT,
        CapabilityKind::STATE,
    };
    const char* kind_keys[] = {
        "\"sensors\":",
        "\"commands\":",
        "\"events\":",
        "\"states\":",
    };

    bool first_group = true;
    for (size_t k = 0; k < 4; ++k) {
        bool first_item = true;

        for (size_t i = 0; i < _count; ++i) {
            if (_caps[i]->kind() != kinds[k]) continue;

            if (first_item) {
                if (!first_group) json::append(buf, len, pos, ",");
                json::append(buf, len, pos, kind_keys[k]);
                json::append(buf, len, pos, "[");
                first_group = false;
                first_item  = false;
            } else {
                json::append(buf, len, pos, ",");
            }

            char cap_buf[512] {};
            _caps[i]->serialize(cap_buf, sizeof(cap_buf));
            json::append(buf, len, pos, cap_buf);
        }

        if (!first_item) {
            json::append(buf, len, pos, "]");
        }
    }

    json::append(buf, len, pos, "}");
    return pos;
}

} // namespace bunny
