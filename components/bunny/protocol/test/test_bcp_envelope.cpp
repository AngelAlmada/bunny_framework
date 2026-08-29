#include "test_framework.h"
#include "../bcp_envelope.h"
#include "ArduinoJson.h"
#include <cstdlib>

TEST_CASE(test_bcp_envelope_error_message)
{
    JsonVariantConst null_v;
    char* err_json = bunny::BcpEnvelope::build_error_message("corr_123", null_v, null_v,
                                                             "INVALID_PARAMS", "validation",
                                                             "Parameter missing", false);
    ASSERT_TRUE(err_json != nullptr);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, err_json);
    ASSERT_FALSE(err);

    ASSERT_STR_EQ(doc["type"].as<const char*>(), "ERROR");
    ASSERT_STR_EQ(doc["correlationId"].as<const char*>(), "corr_123");
    ASSERT_STR_EQ(doc["payload"]["code"].as<const char*>(), "INVALID_PARAMS");
    ASSERT_STR_EQ(doc["payload"]["category"].as<const char*>(), "validation");
    ASSERT_STR_EQ(doc["payload"]["message"].as<const char*>(), "Parameter missing");
    ASSERT_FALSE(doc["payload"]["retryable"].as<bool>());

    free(err_json);
}

TEST_CASE(test_bcp_envelope_response_success)
{
    JsonVariantConst null_v;
    char* res_json = bunny::BcpEnvelope::build_response_success("corr_456", null_v, null_v,
                                                               [&](JsonObject data) {
                                                                   data["sensor"] = "temperature";
                                                                   data["value"] = 24.5;
                                                               });
    ASSERT_TRUE(res_json != nullptr);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, res_json);
    ASSERT_FALSE(err);

    ASSERT_STR_EQ(doc["type"].as<const char*>(), "RESPONSE");
    ASSERT_STR_EQ(doc["correlationId"].as<const char*>(), "corr_456");
    ASSERT_STR_EQ(doc["payload"]["status"].as<const char*>(), "success");
    ASSERT_STR_EQ(doc["payload"]["data"]["sensor"].as<const char*>(), "temperature");
    ASSERT_EQ(doc["payload"]["data"]["value"].as<double>(), 24.5);

    free(res_json);
}

TEST_CASE(test_bcp_envelope_event)
{
    char* evt_json = bunny::BcpEnvelope::build_event("motion_detected", "{\"zone\":\"entrance\",\"count\":1}");
    ASSERT_TRUE(evt_json != nullptr);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, evt_json);
    ASSERT_FALSE(err);

    ASSERT_STR_EQ(doc["type"].as<const char*>(), "EVENT");
    ASSERT_STR_EQ(doc["payload"]["event"].as<const char*>(), "motion_detected");
    ASSERT_STR_EQ(doc["payload"]["data"]["zone"].as<const char*>(), "entrance");
    ASSERT_EQ(doc["payload"]["data"]["count"].as<int>(), 1);
    ASSERT_TRUE(doc["payload"]["meta"].is<JsonObject>());

    free(evt_json);
}

void run_bcp_envelope_tests()
{
    RUN_TEST(test_bcp_envelope_error_message);
    RUN_TEST(test_bcp_envelope_response_success);
    RUN_TEST(test_bcp_envelope_event);
}
