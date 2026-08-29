#pragma once

#include "ArduinoJson.h"
#include "../types/bunny_types.h"
#include "../capabilities/command_capability.h"
#include <string>
#include <vector>

namespace bunny {

class TypeConverter {
public:
    static bool validate_param_type(const JsonVariantConst& value, bunny::Type expected);
    static std::string variant_as_string(const JsonVariantConst& v);
    static bool write_typed_state_value(JsonVariant value_slot, bunny::Type state_type, const char* raw);
    static void json_to_params(JsonObjectConst params_obj,
                               bunny::Params& out_params,
                               std::vector<std::string>& key_storage,
                               std::vector<std::string>& val_storage);
};

} // namespace bunny
