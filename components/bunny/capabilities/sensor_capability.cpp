#include "sensor_capability.h"
#include "../utils/json_builder.h"
#include <cstdio>

namespace bunny {

SensorCapability::SensorCapability(const char* name, const Metadata& meta, SensorReadFn read_fn)
    : _name(name), _meta(meta), _read_fn(std::move(read_fn)) {}

double SensorCapability::read() const {
    if (_read_fn) return _read_fn();
    return 0.0;
}

size_t SensorCapability::serialize(char* buf, size_t len) const {
    size_t pos = 0;
    char tmp[64];

    json::append(buf, len, pos, "{");
    snprintf(tmp, sizeof(tmp), "\"name\":\"%s\",\"kind\":\"sensor\",", _name ? _name : "");
    json::append(buf, len, pos, tmp);
    json::serialize_metadata(buf + pos, len - pos, _meta);
    pos = __builtin_strlen(buf);
    json::append(buf, len, pos, "}");
    return pos;
}

} // namespace bunny
