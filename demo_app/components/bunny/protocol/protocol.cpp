#include "protocol.h"

#include "../config/config.h"
#include "../network/network.h"
#include "../registry/registry.h"
#include "../capabilities/command_capability.h"
#include "../capabilities/sensor_capability.h"
#include "../capabilities/state_capability.h"

#include "ArduinoJson.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include <sys/time.h>

namespace {

static const char* TAG = "bunny_protocol";
static constexpr size_t DUP_BUFFER_SIZE = 16;
static constexpr const char* STATE_METHOD_PREFIX = "getState:";

static char s_seen_message_ids[DUP_BUFFER_SIZE][64] = {{0}};
static size_t s_seen_count = 0;
static size_t s_seen_next = 0;

static uint64_t uptime_ms()
{
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

static uint64_t protocol_timestamp_ms()
{
    struct timeval tv = {0, 0};
    if (gettimeofday(&tv, nullptr) == 0) {
        const uint64_t epoch_ms = ((uint64_t)tv.tv_sec * 1000ULL) + ((uint64_t)tv.tv_usec / 1000ULL);
        if (epoch_ms > 1700000000000ULL) {
            return epoch_ms;
        }
    }
    return uptime_ms();
}

static const char* device_id()
{
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->device.id) {
        return cfg->device.id;
    }
    return "unknown";
}

static const char* firmware_version()
{
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->device.version) {
        return cfg->device.version;
    }
    return "unknown";
}

static std::string make_message_id(const char* short_type)
{
    char out[32];
    uint32_t low_ms = (uint32_t)(uptime_ms() & 0xFFFFU);
    uint32_t rnd = (uint32_t)(esp_random() & 0xFFFFU);
    snprintf(out, sizeof(out), "%s_%04x_%04x", short_type, (unsigned)low_ms, (unsigned)rnd);
    return std::string(out);
}

static bool is_duplicate_message_id(const char* message_id)
{
    if (!message_id || !message_id[0]) {
        return false;
    }

    for (size_t i = 0; i < s_seen_count; ++i) {
        if (strcmp(s_seen_message_ids[i], message_id) == 0) {
            return true;
        }
    }
    return false;
}

static void remember_message_id(const char* message_id)
{
    if (!message_id || !message_id[0]) {
        return;
    }

    strncpy(s_seen_message_ids[s_seen_next], message_id, sizeof(s_seen_message_ids[s_seen_next]) - 1);
    s_seen_message_ids[s_seen_next][sizeof(s_seen_message_ids[s_seen_next]) - 1] = '\0';

    s_seen_next = (s_seen_next + 1U) % DUP_BUFFER_SIZE;
    if (s_seen_count < DUP_BUFFER_SIZE) {
        ++s_seen_count;
    }
}

static char* serialize_doc(const JsonDocument& doc)
{
    size_t len = measureJson(doc);
    char* out = (char*)malloc(len + 1U);
    if (!out) {
        return nullptr;
    }

    size_t written = serializeJson(doc, out, len + 1U);
    out[written] = '\0';
    return out;
}

static JsonObject build_envelope(JsonDocument& out,
                                 const char* type,
                                 const char* message_id,
                                 const char* correlation_id,
                                 JsonVariantConst instance_id,
                                 JsonVariantConst step_id)
{
    out.clear();
    out["type"] = type;
    out["messageId"] = message_id;

    if (correlation_id) {
        out["correlationId"] = correlation_id;
    } else {
        out["correlationId"] = nullptr;
    }

    out["deviceId"] = device_id();

    if (instance_id.isNull() || instance_id.isUnbound()) {
        out["instanceId"] = nullptr;
    } else {
        out["instanceId"].set(instance_id);
    }

    if (step_id.isNull() || step_id.isUnbound()) {
        out["stepId"] = nullptr;
    } else {
        out["stepId"].set(step_id);
    }

    out["timestamp"] = protocol_timestamp_ms();
    return out["payload"].to<JsonObject>();
}

static char* build_error_message(const char* correlation_id,
                                 JsonVariantConst instance_id,
                                 JsonVariantConst step_id,
                                 const char* code,
                                 const char* category,
                                 const char* message,
                                 bool retryable)
{
    JsonDocument out;
    std::string msg_id = make_message_id("err");
    JsonObject payload = build_envelope(out,
                                        "ERROR",
                                        msg_id.c_str(),
                                        correlation_id,
                                        instance_id,
                                        step_id);
    payload["code"] = code;
    payload["category"] = category;
    payload["message"] = message;
    payload["retryable"] = retryable;
    return serialize_doc(out);
}

static char* build_response_success(const char* correlation_id,
                                    JsonVariantConst instance_id,
                                    JsonVariantConst step_id,
                                    std::function<void(JsonObject)> data_writer)
{
    JsonDocument out;
    std::string msg_id = make_message_id("res");
    JsonObject payload = build_envelope(out,
                                        "RESPONSE",
                                        msg_id.c_str(),
                                        correlation_id,
                                        instance_id,
                                        step_id);
    payload["status"] = "success";
    JsonObject data = payload["data"].to<JsonObject>();
    data_writer(data);
    payload["error"] = nullptr;
    return serialize_doc(out);
}

static char* build_response_error(const char* correlation_id,
                                  JsonVariantConst instance_id,
                                  JsonVariantConst step_id,
                                  const char* code,
                                  const char* message,
                                  bool retryable)
{
    JsonDocument out;
    std::string msg_id = make_message_id("res");
    JsonObject payload = build_envelope(out,
                                        "RESPONSE",
                                        msg_id.c_str(),
                                        correlation_id,
                                        instance_id,
                                        step_id);
    payload["status"] = "error";
    payload["data"] = nullptr;

    JsonObject err = payload["error"].to<JsonObject>();
    err["code"] = code;
    err["message"] = message;
    err["retryable"] = retryable;
    return serialize_doc(out);
}

static bool validate_param_type(const JsonVariantConst& value, bunny::Type expected)
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

static std::string variant_as_string(const JsonVariantConst& v)
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

static bool write_typed_state_value(JsonVariant value_slot, bunny::Type state_type, const char* raw)
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

static char* handle_command(const JsonDocument& in,
                            const char* inbound_message_id,
                            JsonVariantConst instance_id,
                            JsonVariantConst step_id)
{
    JsonObjectConst payload = in["payload"].as<JsonObjectConst>();
    const char* method = payload["method"];
    if (!method || !method[0]) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "INVALID_PARAMS", "validation",
                                   "COMMAND requires payload.method", false);
    }

    JsonVariantConst params_variant = payload["params"];
    if (params_variant.isUnbound() || !params_variant.is<JsonObjectConst>()) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "INVALID_PARAMS", "validation",
                                   "COMMAND requires payload.params object", false);
    }

    bool await_response = true;
    if (!payload["awaitResponse"].isUnbound()) {
        await_response = payload["awaitResponse"].as<bool>();
    }

    bunny::ICapability* cap = bunny::Registry::instance().find(method, bunny::CapabilityKind::COMMAND);
    if (!cap) {
        return build_error_message(inbound_message_id, instance_id, step_id,
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
            return build_error_message(inbound_message_id, instance_id, step_id,
                                       "INVALID_PARAMS", "validation",
                                       "Missing required command parameter", false);
        }

        if (!pv.isUnbound() && !pv.isNull() && !validate_param_type(pv, def.type)) {
            return build_error_message(inbound_message_id, instance_id, step_id,
                                       "INVALID_PARAMS", "validation",
                                       "Command parameter type is incompatible", false);
        }
    }

    bunny::Params runtime_params;
    std::vector<std::string> key_storage;
    std::vector<std::string> value_storage;
    key_storage.reserve(bunny::Params::MAX_ENTRIES);
    value_storage.reserve(bunny::Params::MAX_ENTRIES);

    size_t written = 0;
    for (JsonPairConst kv : params_obj) {
        if (written >= bunny::Params::MAX_ENTRIES) {
            break;
        }
        key_storage.emplace_back(kv.key().c_str());
        value_storage.emplace_back(variant_as_string(kv.value()));
        runtime_params.set(key_storage.back().c_str(), value_storage.back().c_str());
        ++written;
    }

    command->execute(runtime_params);

    if (!await_response) {
        return nullptr;
    }

    return build_response_success(inbound_message_id,
                                  instance_id,
                                  step_id,
                                  [&](JsonObject data) {
                                      JsonObject applied = data["applied"].to<JsonObject>();
                                      applied.set(params_obj);
                                  });
}

static char* handle_sensor_request(const JsonDocument& in,
                                   const char* inbound_message_id,
                                   JsonVariantConst instance_id,
                                   JsonVariantConst step_id,
                                   const char* method)
{
    bunny::ICapability* cap = bunny::Registry::instance().find(method, bunny::CapabilityKind::SENSOR);
    if (!cap) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "UNKNOWN_METHOD", "validation",
                                   "Sensor method is not registered in this device", false);
    }

    auto* sensor = static_cast<bunny::SensorCapability*>(cap);
    double value = 0.0;

    value = sensor->read();

    if (!std::isfinite(value)) {
        return build_response_error(inbound_message_id,
                                    instance_id,
                                    step_id,
                                    "SENSOR_READ_FAIL",
                                    "Sensor value is invalid or out of range",
                                    true);
    }

    return build_response_success(inbound_message_id,
                                  instance_id,
                                  step_id,
                                  [&](JsonObject data) {
                                      data["sensor"] = method;
                                      data["value"] = value;
                                  });
}

static char* handle_state_request(const JsonDocument& in,
                                  const char* inbound_message_id,
                                  JsonVariantConst instance_id,
                                  JsonVariantConst step_id,
                                  const char* method)
{
    const size_t prefix_len = strlen(STATE_METHOD_PREFIX);
    const char* state_name = method + prefix_len;

    if (!state_name || !state_name[0]) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "INVALID_PARAMS", "validation",
                                   "State request method must be getState:<name>", false);
    }

    bunny::ICapability* cap = bunny::Registry::instance().find(state_name, bunny::CapabilityKind::STATE);
    if (!cap) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "UNKNOWN_STATE", "validation",
                                   "Requested state is not registered in this device", false);
    }

    auto* state = static_cast<bunny::StateCapability*>(cap);
    const char* raw = state->get();

    JsonDocument probe;
    std::string msg_id = make_message_id("res");
    JsonObject payload = build_envelope(probe,
                                        "RESPONSE",
                                        msg_id.c_str(),
                                        inbound_message_id,
                                        instance_id,
                                        step_id);
    payload["status"] = "success";
    JsonObject data = payload["data"].to<JsonObject>();
    data["state"] = state_name;

    if (!write_typed_state_value(data["value"], state->state_type(), raw)) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "INVALID_PARAMS", "validation",
                                   "State getter returned a value incompatible with state type", false);
    }

    payload["error"] = nullptr;
    return serialize_doc(probe);
}

static char* handle_request(const JsonDocument& in,
                            const char* inbound_message_id,
                            JsonVariantConst instance_id,
                            JsonVariantConst step_id)
{
    JsonObjectConst payload = in["payload"].as<JsonObjectConst>();
    const char* method = payload["method"];
    if (!method || !method[0]) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "INVALID_PARAMS", "validation",
                                   "REQUEST requires payload.method", false);
    }

    if (strncmp(method, STATE_METHOD_PREFIX, strlen(STATE_METHOD_PREFIX)) == 0) {
        return handle_state_request(in, inbound_message_id, instance_id, step_id, method);
    }

    return handle_sensor_request(in, inbound_message_id, instance_id, step_id, method);
}

} // namespace

extern "C" {

void bunny_protocol_init(void)
{
    memset(s_seen_message_ids, 0, sizeof(s_seen_message_ids));
    s_seen_count = 0;
    s_seen_next = 0;
}

char* bunny_protocol_handle_incoming(const char* incoming_json)
{
    if (!incoming_json) {
        JsonVariantConst null_variant;
        return build_error_message(nullptr, null_variant, null_variant,
                                   "PARSE_ERROR", "protocol",
                                   "Incoming payload is null", false);
    }

    JsonDocument in;
    DeserializationError parse_error = deserializeJson(in, incoming_json);
    if (parse_error) {
        JsonVariantConst null_variant;
        return build_error_message(nullptr, null_variant, null_variant,
                                   "PARSE_ERROR", "protocol",
                                   "Incoming payload is not valid JSON", false);
    }

    JsonVariantConst type_v = in["type"];
    JsonVariantConst message_id_v = in["messageId"];
    JsonVariantConst payload_v = in["payload"];
    JsonVariantConst instance_id = in["instanceId"];
    JsonVariantConst step_id = in["stepId"];

    const char* inbound_message_id = message_id_v.is<const char*>() ? message_id_v.as<const char*>() : nullptr;

    if (!type_v.is<const char*>() || !inbound_message_id || !payload_v.is<JsonObjectConst>()) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "PARSE_ERROR", "protocol",
                                   "Envelope requires type, messageId and payload object", false);
    }

    if (is_duplicate_message_id(inbound_message_id)) {
        return build_error_message(inbound_message_id, instance_id, step_id,
                                   "DUPLICATE_MSG", "protocol",
                                   "MessageId already processed", false);
    }

    remember_message_id(inbound_message_id);

    const char* type = type_v.as<const char*>();
    if (strcmp(type, "COMMAND") == 0) {
        return handle_command(in, inbound_message_id, instance_id, step_id);
    }

    if (strcmp(type, "REQUEST") == 0) {
        return handle_request(in, inbound_message_id, instance_id, step_id);
    }

    return build_error_message(inbound_message_id, instance_id, step_id,
                               "PARSE_ERROR", "protocol",
                               "Unsupported message type for inbound dispatcher", false);
}

void bunny_protocol_free_message(char* message_json)
{
    if (message_json) {
        free(message_json);
    }
}

bool bunny_protocol_emit_event(const char* event_name, const char* data_json)
{
    if (!event_name || !event_name[0]) {
        return false;
    }

    JsonDocument out;
    JsonVariantConst null_variant;
    std::string msg_id = make_message_id("evt");
    JsonObject payload = build_envelope(out,
                                        "EVENT",
                                        msg_id.c_str(),
                                        nullptr,
                                        null_variant,
                                        null_variant);

    payload["event"] = event_name;

    JsonObject data = payload["data"].to<JsonObject>();
    if (data_json && data_json[0]) {
        JsonDocument in_data;
        DeserializationError err = deserializeJson(in_data, data_json);
        if (!err && in_data.is<JsonObject>()) {
            data.set(in_data.as<JsonObjectConst>());
        }
    }

    JsonObject meta = payload["meta"].to<JsonObject>();
    meta["firmwareVersion"] = firmware_version();
    meta["freeHeap"] = esp_get_free_heap_size();
    meta["uptime"] = uptime_ms();

    char* json = serialize_doc(out);
    if (!json) {
        return false;
    }

    bool sent = bunny_network_send_text(json);
    free(json);

    if (!sent) {
        ESP_LOGW(TAG, "EVENT not sent: websocket session is not active");
    }
    return sent;
}

} // extern "C"
