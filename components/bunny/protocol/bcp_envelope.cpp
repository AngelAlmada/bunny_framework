#include "bcp_envelope.h"
#include "../config/config.h"

#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>

namespace bunny {

uint64_t BcpEnvelope::uptime_ms()
{
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

uint64_t BcpEnvelope::timestamp_ms()
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

const char* BcpEnvelope::device_id()
{
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->device.id) {
        return cfg->device.id;
    }
    return "unknown";
}

const char* BcpEnvelope::firmware_version()
{
    const bunny_config_t* cfg = bunny_config_get();
    if (cfg && cfg->device.version) {
        return cfg->device.version;
    }
    return "unknown";
}

std::string BcpEnvelope::make_message_id(const char* short_type)
{
    char out[32];
    uint32_t low_ms = (uint32_t)(uptime_ms() & 0xFFFFU);
    uint32_t rnd = (uint32_t)(esp_random() & 0xFFFFU);
    snprintf(out, sizeof(out), "%s_%04x_%04x", short_type, (unsigned)low_ms, (unsigned)rnd);
    return std::string(out);
}

char* BcpEnvelope::serialize_doc(const JsonDocument& doc)
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

JsonObject BcpEnvelope::build_envelope(JsonDocument& out,
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

    out["timestamp"] = timestamp_ms();
    return out["payload"].to<JsonObject>();
}

char* BcpEnvelope::build_error_message(const char* correlation_id,
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

char* BcpEnvelope::build_response_success(const char* correlation_id,
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
    if (data_writer) {
        data_writer(data);
    }
    payload["error"] = nullptr;
    return serialize_doc(out);
}

char* BcpEnvelope::build_response_error(const char* correlation_id,
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

char* BcpEnvelope::build_event(const char* event_name, const char* data_json)
{
    if (!event_name || !event_name[0]) {
        return nullptr;
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

    return serialize_doc(out);
}

} // namespace bunny
