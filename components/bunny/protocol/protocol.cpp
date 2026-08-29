#include "protocol.h"

#include "bcp_envelope.h"
#include "dedup_cache.h"
#include "../network/network.h"
#include "../runtime/dispatcher.h"

#include "ArduinoJson.h"
#include "esp_log.h"
#include <cstdlib>
#include <cstring>

namespace {

static const char* TAG = "bunny_protocol";
static bunny::DedupCache s_dedup_cache;

} // namespace

extern "C" {

void bunny_protocol_init(void)
{
    s_dedup_cache.clear();
}

char* bunny_protocol_handle_incoming(const char* incoming_json)
{
    if (!incoming_json) {
        JsonVariantConst null_variant;
        return bunny::BcpEnvelope::build_error_message(nullptr, null_variant, null_variant,
                                                       "PARSE_ERROR", "protocol",
                                                       "Incoming payload is null", false);
    }

    JsonDocument in;
    DeserializationError parse_error = deserializeJson(in, incoming_json);
    if (parse_error) {
        JsonVariantConst null_variant;
        return bunny::BcpEnvelope::build_error_message(nullptr, null_variant, null_variant,
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
        return bunny::BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                       "PARSE_ERROR", "protocol",
                                                       "Envelope requires type, messageId and payload object", false);
    }

    if (s_dedup_cache.is_duplicate(inbound_message_id)) {
        return bunny::BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
                                                       "DUPLICATE_MSG", "protocol",
                                                       "MessageId already processed", false);
    }

    s_dedup_cache.remember(inbound_message_id);

    const char* type = type_v.as<const char*>();
    if (strcmp(type, "COMMAND") == 0) {
        return bunny::Dispatcher::dispatch_command(in, inbound_message_id, instance_id, step_id);
    }

    if (strcmp(type, "REQUEST") == 0) {
        return bunny::Dispatcher::dispatch_request(in, inbound_message_id, instance_id, step_id);
    }

    return bunny::BcpEnvelope::build_error_message(inbound_message_id, instance_id, step_id,
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
    char* json = bunny::BcpEnvelope::build_event(event_name, data_json);
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
