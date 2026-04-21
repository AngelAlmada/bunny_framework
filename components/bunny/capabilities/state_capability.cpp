#include "state_capability.h"
#include "../utils/json_builder.h"
#include <cstdio>

namespace bunny {

StateCapability::StateCapability(const char* name, const Metadata& meta,
                                 Type state_type,
                                 StateGetFn get_fn, StateSetFn set_fn)
    : _name(name), _meta(meta), _state_type(state_type),
      _get_fn(std::move(get_fn)), _set_fn(std::move(set_fn)) {}

const char* StateCapability::get() const {
    if (_get_fn) return _get_fn();
    return nullptr;
}

void StateCapability::set(const char* val) const {
    if (_set_fn) _set_fn(val);
}

size_t StateCapability::serialize(char* buf, size_t len) const {
    size_t pos = 0;
    char tmp[128];

    json::append(buf, len, pos, "{");
    snprintf(tmp, sizeof(tmp),
             "\"name\":\"%s\",\"kind\":\"state\",\"stateType\":\"%s\",",
             _name ? _name : "", type_name(_state_type));
    json::append(buf, len, pos, tmp);
    json::serialize_metadata(buf + pos, len - pos, _meta);
    pos = __builtin_strlen(buf);
    json::append(buf, len, pos, "}");
    return pos;
}

} // namespace bunny
