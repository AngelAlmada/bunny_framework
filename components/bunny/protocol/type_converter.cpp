#include "type_converter.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bunny {

bool TypeConverter::validate_param_type(const JsonVariantConst& value, bunny::Type expected)
{
    switch (expected) {
        case bunny::Type::STRING:  return value.is<const char*>();
        case bunny::Type::NUMBER:  return value.is<int>() || value.is<long>() || value.is<float>() || value.is<double>();
        case bunny::Type::BOOLEAN: return value.is<bool>();
        case bunny::Type::OBJECT:  return value.is<JsonObjectConst>();
        case bunny::Type::ARRAY:   return value.is<JsonArrayConst>();
        case bunny::Type::VOID:    return true;
        default:                   return false;
    }
}

std::string TypeConverter::variant_as_string(const JsonVariantConst& v)
{
    if (v.is<const char*>()) {
        return std::string(v.as<const char*>());
    }
    if (v.is<bool>()) {
        return v.as<bool>() ? "true" : "false";
    }
    if (v.is<int>()) {
        return std::to_string(v.as<int>());
    }
    if (v.is<long>()) {
        return std::to_string(v.as<long>());
    }
    if (v.is<float>()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.6f", v.as<float>());
        return std::string(buf);
    }
    if (v.is<double>()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.6f", v.as<double>());
        return std::string(buf);
    }

    std::string out;
    serializeJson(v, out);
    return out;
}

bool TypeConverter::write_typed_state_value(JsonVariant value_slot, bunny::Type state_type, const char* raw)
{
    const char* safe = raw ? raw : "";

    if (state_type == bunny::Type::STRING) {
        value_slot.set(safe);
        return true;
    }

    if (state_type == bunny::Type::NUMBER) {
        char* end = nullptr;
        double number = strtod(safe, &end);
        if (end == safe || (end && *end != '\0')) {
            return false;
        }
        value_slot.set(number);
        return true;
    }

    if (state_type == bunny::Type::BOOLEAN) {
        if (strcmp(safe, "true") == 0 || strcmp(safe, "1") == 0) {
            value_slot.set(true);
            return true;
        }
        if (strcmp(safe, "false") == 0 || strcmp(safe, "0") == 0) {
            value_slot.set(false);
            return true;
        }
        return false;
    }

    if (state_type == bunny::Type::OBJECT || state_type == bunny::Type::ARRAY) {
        JsonDocument tmp;
        DeserializationError e = deserializeJson(tmp, safe);
        if (e) {
            return false;
        }
        value_slot.set(tmp.as<JsonVariantConst>());
        return true;
    }

    value_slot.set(safe);
    return true;
}

void TypeConverter::json_to_params(JsonObjectConst params_obj,
                                   bunny::Params& out_params,
                                   std::vector<std::string>& key_storage,
                                   std::vector<std::string>& val_storage)
{
    key_storage.clear();
    val_storage.clear();
    key_storage.reserve(bunny::Params::MAX_ENTRIES);
    val_storage.reserve(bunny::Params::MAX_ENTRIES);

    size_t written = 0;
    for (JsonPairConst kv : params_obj) {
        if (written >= bunny::Params::MAX_ENTRIES) {
            break;
        }
        key_storage.emplace_back(kv.key().c_str());
        val_storage.emplace_back(variant_as_string(kv.value()));
        out_params.set(key_storage.back().c_str(), val_storage.back().c_str());
        ++written;
    }
}

} // namespace bunny
