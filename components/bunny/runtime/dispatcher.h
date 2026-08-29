#pragma once

#include "ArduinoJson.h"

namespace bunny {

class Dispatcher {
public:
    static constexpr const char* STATE_METHOD_PREFIX = "getState:";

    static char* dispatch_command(const JsonDocument& in,
                                  const char* inbound_message_id,
                                  JsonVariantConst instance_id,
                                  JsonVariantConst step_id);

    static char* dispatch_request(const JsonDocument& in,
                                  const char* inbound_message_id,
                                  JsonVariantConst instance_id,
                                  JsonVariantConst step_id);

    static char* dispatch_sensor_request(const JsonDocument& in,
                                         const char* inbound_message_id,
                                         JsonVariantConst instance_id,
                                         JsonVariantConst step_id,
                                         const char* method);

    static char* dispatch_state_request(const JsonDocument& in,
                                        const char* inbound_message_id,
                                        JsonVariantConst instance_id,
                                        JsonVariantConst step_id,
                                        const char* method);
};

} // namespace bunny
