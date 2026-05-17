#include "event_capability.h"
#include "../utils/json_builder.h"
#include "../protocol/protocol.h"
#include <cstdio>

namespace bunny {

EventCapability::EventCapability(const char* name, const Metadata& meta, EventEmitFn on_emit)
    : _name(name), _meta(meta), _on_emit(std::move(on_emit)) {}

void EventCapability::emit() const {
    /* Transport layer (network module) handles sending to motor de procesos.
       The optional local hook runs for hardware side-effects only. */
    if (_on_emit) _on_emit();
    (void)bunny_protocol_emit_event(_name, "{}");
}

size_t EventCapability::serialize(char* buf, size_t len) const {
    size_t pos = 0;
    char tmp[64];

    json::append(buf, len, pos, "{");
    snprintf(tmp, sizeof(tmp), "\"name\":\"%s\",\"kind\":\"event\",", _name ? _name : "");
    json::append(buf, len, pos, tmp);
    json::serialize_metadata(buf + pos, len - pos, _meta);
    pos = __builtin_strlen(buf);
    json::append(buf, len, pos, "}");
    return pos;
}

} // namespace bunny
