#include "dispatcher.h"

#include "../capabilities/command_capability.h"
#include "../capabilities/sensor_capability.h"
#include "../capabilities/state_capability.h"
#include "../protocol/bcp_envelope.h"
#include "../protocol/type_converter.h"
#include "../registry/registry.h"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace bunny {

char* Dispatcher::dispatch_command(const JsonDocument& in,
                                   const char* inbound_message_id,
                                   JsonVariantConst instance_id,
                                   JsonVariantConst step_id)
{
    JsonObjectConst payload = in["payload"].as<JsonObjectConst>();
    const char* method = payload["method"];
    if (!method || !method[0]) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "INVALID_PARAMS", "validation",
                                                "COMMAND requires payload.method", false);
    }

    JsonVariantConst params_variant = payload["params"];
    if (params_variant.isUnbound() || !params_variant.is<JsonObjectConst>()) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "INVALID_PARAMS", "validation",
                                                "COMMAND requires payload.params object", false);
    }

    bool await_response = true;
    if (!payload["awaitResponse"].isUnbound()) {
        await_response = payload["awaitResponse"].as<bool>();
    }

    bunny::ICapability* cap = bunny::Registry::instance().find(method, bunny::CapabilityKind::COMMAND);
    if (!cap) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "UNKNOWN_METHOD", "validation",
                                                "Command method is not registered in this device", false);
    }

    auto* command = static_cast<bunny::CommandCapability*>(cap);
    JsonObjectConst params_obj = params_variant.as<JsonObjectConst>();

    const bunny::Metadata& meta = command->metadata();
    for (size_t i = 0; i < meta.param_count; ++i) {
        const bunny::ParamDef& def = meta.params[i];
        JsonVariantConst pv = params_obj[def.name];

        if (def.required && (pv.isUnbound() || pv.isNull())) {
            return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                    "INVALID_PARAMS", "validation",
                                                    "Missing required command parameter", false);
        }

        if (!pv.isUnbound() && !pv.isNull() && !TypeConverter::validate_param_type(pv, def.type)) {
            return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                    "INVALID_PARAMS", "validation",
                                                    "Command parameter type is incompatible", false);
        }
    }

    bunny::Params runtime_params;
    std::vector<std::string> key_storage;
    std::vector<std::string> value_storage;
    TypeConverter::json_to_params(params_obj, runtime_params, key_storage, value_storage);

    command->execute(runtime_params);

    if (!await_response) {
        return nullptr;
    }

    return BcpEnvelope::build_response_success(inbound_message_id,
                                               instance_id,
                                               step_id,
                                               [&](JsonObject data) {
                                                   JsonObject applied = data["applied"].to<JsonObject>();
                                                   applied.set(params_obj);
                                               });
}

char* Dispatcher::dispatch_sensor_request(const JsonDocument& in,
                                          const char* inbound_message_id,
                                          JsonVariantConst instance_id,
                                          JsonVariantConst step_id,
                                          const char* method)
{
    (void)in;
    bunny::ICapability* cap = bunny::Registry::instance().find(method, bunny::CapabilityKind::SENSOR);
    if (!cap) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "UNKNOWN_METHOD", "validation",
                                                "Sensor method is not registered in this device", false);
    }

    auto* sensor = static_cast<bunny::SensorCapability*>(cap);
    double value = sensor->read();

    if (!std::isfinite(value)) {
        return BcpEnvelope::build_response_error(inbound_message_id,
                                                 instance_id,
                                                 step_id,
                                                 "SENSOR_READ_FAIL",
                                                 "Sensor value is invalid or out of range",
                                                 true);
    }

    return BcpEnvelope::build_response_success(inbound_message_id,
                                               instance_id,
                                               step_id,
                                               [&](JsonObject data) {
                                                   data["sensor"] = method;
                                                   data["value"] = value;
                                               });
}

char* Dispatcher::dispatch_state_request(const JsonDocument& in,
                                         const char* inbound_message_id,
                                         JsonVariantConst instance_id,
                                         JsonVariantConst step_id,
                                         const char* method)
{
    (void)in;
    const size_t prefix_len = strlen(STATE_METHOD_PREFIX);
    const char* state_name = method + prefix_len;

    if (!state_name || !state_name[0]) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "INVALID_PARAMS", "validation",
                                                "State request method must be getState:<name>", false);
    }

    bunny::ICapability* cap = bunny::Registry::instance().find(state_name, bunny::CapabilityKind::STATE);
    if (!cap) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "UNKNOWN_STATE", "validation",
                                                "Requested state is not registered in this device", false);
    }

    auto* state = static_cast<bunny::StateCapability*>(cap);
    const char* raw = state->get();

    JsonDocument probe;
    std::string msg_id = BcpEnvelope::make_message_id("res");
    JsonObject payload = BcpEnvelope::build_envelope(probe,
                                                     "RESPONSE",
                                                     msg_id.c_str(),
                                                     inbound_message_id,
                                                     instance_id,
                                                     step_id);
    payload["status"] = "success";
    JsonObject data = payload["data"].to<JsonObject>();
    data["state"] = state_name;

    if (!TypeConverter::write_typed_state_value(data["value"], state->state_type(), raw)) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "INVALID_PARAMS", "validation",
                                                "State getter returned a value incompatible with state type", false);
    }

    payload["error"] = nullptr;
    return BcpEnvelope::serialize_doc(probe);
}

char* Dispatcher::dispatch_request(const JsonDocument& in,
                                   const char* inbound_message_id,
                                   JsonVariantConst instance_id,
                                   JsonVariantConst step_id)
{
    JsonObjectConst payload = in["payload"].as<JsonObjectConst>();
    const char* method = payload["method"];
    if (!method || !method[0]) {
        return BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                "INVALID_PARAMS", "validation",
                                                "REQUEST requires payload.method", false);
    }

    if (strncmp(method, STATE_METHOD_PREFIX, strlen(STATE_METHOD_PREFIX)) == 0) {
        return dispatch_state_request(in, inbound_message_id, instance_id, step_id, method);
    }

    return dispatch_sensor_request(in, inbound_message_id, instance_id, step_id, method);
}

} // namespace bunny
