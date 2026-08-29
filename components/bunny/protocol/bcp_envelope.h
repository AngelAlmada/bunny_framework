#pragma once

#include "ArduinoJson.h"
#include <cstdint>
#include <functional>
#include <string>

namespace bunny {

class BcpEnvelope {
public:
    static uint64_t uptime_ms();
    static uint64_t timestamp_ms();
    static const char* device_id();
    static const char* firmware_version();
    static std::string make_message_id(const char* short_type);

    static char* serialize_doc(const JsonDocument& doc);

    static JsonObject build_envelope(JsonDocument& out,
                                     const char* type,
                                     const char* message_id,
                                     const char* correlation_id,
                                     JsonVariantConst instance_id,
                                     JsonVariantConst step_id);

    static char* build_error_message(const char* correlation_id,
                                     JsonVariantConst instance_id,
                                     JsonVariantConst step_id,
                                     const char* code,
                                     const char* category,
                                     const char* message,
                                     bool retryable);

    static char* build_response_success(const char* correlation_id,
                                        JsonVariantConst instance_id,
                                        JsonVariantConst step_id,
                                        std::function<void(JsonObject)> data_writer);

    static char* build_response_error(const char* correlation_id,
                                      JsonVariantConst instance_id,
                                      JsonVariantConst step_id,
                                      const char* code,
                                      const char* message,
                                      bool retryable);

    static char* build_event(const char* event_name, const char* data_json);
};

} // namespace bunny
