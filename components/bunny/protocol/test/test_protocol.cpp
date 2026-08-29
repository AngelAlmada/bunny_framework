#include "test_framework.h"
#include "../protocol.h"
#include "../../registry/registry.h"
#include "../../capabilities/command_capability.h"
#include "../../capabilities/sensor_capability.h"
#include "../../capabilities/state_capability.h"
#include "ArduinoJson.h"
#include <cstdlib>

static bool s_fan_state = false;
static std::string s_last_state_val = "OFF";

static void setup_mock_capabilities()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // Command: setFanState
    bunny::Metadata cmd_meta;
    cmd_meta.description = "Set fan state";
    cmd_meta.add_param("state", bunny::Type::STRING, "Target state (ON/OFF)", true);
    auto* cmd = new bunny::CommandCapability("setFanState", cmd_meta, [](const bunny::Params& p) {
        const char* s = p.get_string("state");
        s_fan_state = (strcmp(s, "ON") == 0);
        s_last_state_val = s;
    });
    bunny::Registry::instance().register_capability(cmd);

    // Sensor: temperature
    bunny::Metadata sensor_meta;
    sensor_meta.description = "Read temperature";
    sensor_meta.returns_type = bunny::Type::NUMBER;
    auto* sensor = new bunny::SensorCapability("temperature", sensor_meta, []() -> double {
        return 25.5;
    });
    bunny::Registry::instance().register_capability(sensor);

    // State: fanState
    bunny::Metadata state_meta;
    state_meta.description = "Fan state";
    auto* state = new bunny::StateCapability("fanState", state_meta, bunny::Type::STRING,
        []() -> const char* { return s_last_state_val.c_str(); },
        [](const char* val) { s_last_state_val = val; }
    );
    bunny::Registry::instance().register_capability(state);
}

TEST_CASE(test_protocol_handle_valid_command)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    const char* in_json = "{\"type\":\"COMMAND\",\"messageId\":\"cmd_101\",\"payload\":{\"method\":\"setFanState\",\"params\":{\"state\":\"ON\"}}}";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "RESPONSE");
    ASSERT_STR_EQ(doc["correlationId"].as<const char*>(), "cmd_101");
    ASSERT_STR_EQ(doc["payload"]["status"].as<const char*>(), "success");
    ASSERT_STR_EQ(doc["payload"]["data"]["applied"]["state"].as<const char*>(), "ON");
    ASSERT_TRUE(s_fan_state);

    bunny_protocol_free_message(res);
}

TEST_CASE(test_protocol_handle_duplicate_message)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    const char* in_json = "{\"type\":\"COMMAND\",\"messageId\":\"dup_001\",\"payload\":{\"method\":\"setFanState\",\"params\":{\"state\":\"ON\"}}}";
    char* res1 = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res1 != nullptr);
    bunny_protocol_free_message(res1);

    // Second execution with same messageId
    char* res2 = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res2 != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res2);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "ERROR");
    ASSERT_STR_EQ(doc["payload"]["code"].as<const char*>(), "DUPLICATE_MSG");

    bunny_protocol_free_message(res2);
}

TEST_CASE(test_protocol_handle_missing_required_param)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    const char* in_json = "{\"type\":\"COMMAND\",\"messageId\":\"cmd_miss\",\"payload\":{\"method\":\"setFanState\",\"params\":{}}}";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "ERROR");
    ASSERT_STR_EQ(doc["payload"]["code"].as<const char*>(), "INVALID_PARAMS");

    bunny_protocol_free_message(res);
}

TEST_CASE(test_protocol_handle_unknown_method)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    const char* in_json = "{\"type\":\"COMMAND\",\"messageId\":\"cmd_unk\",\"payload\":{\"method\":\"nonExistentCommand\",\"params\":{}}}";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "ERROR");
    ASSERT_STR_EQ(doc["payload"]["code"].as<const char*>(), "UNKNOWN_METHOD");

    bunny_protocol_free_message(res);
}

TEST_CASE(test_protocol_handle_sensor_request)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    const char* in_json = "{\"type\":\"REQUEST\",\"messageId\":\"req_temp\",\"payload\":{\"method\":\"temperature\"}}";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "RESPONSE");
    ASSERT_STR_EQ(doc["payload"]["status"].as<const char*>(), "success");
    ASSERT_STR_EQ(doc["payload"]["data"]["sensor"].as<const char*>(), "temperature");
    ASSERT_EQ(doc["payload"]["data"]["value"].as<double>(), 25.5);

    bunny_protocol_free_message(res);
}

TEST_CASE(test_protocol_handle_state_request)
{
    bunny_protocol_init();
    setup_mock_capabilities();

    s_last_state_val = "ACTIVE";

    const char* in_json = "{\"type\":\"REQUEST\",\"messageId\":\"req_state\",\"payload\":{\"method\":\"getState:fanState\"}}";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "RESPONSE");
    ASSERT_STR_EQ(doc["payload"]["status"].as<const char*>(), "success");
    ASSERT_STR_EQ(doc["payload"]["data"]["state"].as<const char*>(), "fanState");
    ASSERT_STR_EQ(doc["payload"]["data"]["value"].as<const char*>(), "ACTIVE");

    bunny_protocol_free_message(res);
}

TEST_CASE(test_protocol_handle_malformed_json)
{
    bunny_protocol_init();

    const char* in_json = "this is not a valid json envelope";
    char* res = bunny_protocol_handle_incoming(in_json);
    ASSERT_TRUE(res != nullptr);

    JsonDocument doc;
    deserializeJson(doc, res);
    ASSERT_STR_EQ(doc["type"].as<const char*>(), "ERROR");
    ASSERT_STR_EQ(doc["payload"]["code"].as<const char*>(), "PARSE_ERROR");

    bunny_protocol_free_message(res);
}

void run_protocol_tests()
{
    RUN_TEST(test_protocol_handle_valid_command);
    RUN_TEST(test_protocol_handle_duplicate_message);
    RUN_TEST(test_protocol_handle_missing_required_param);
    RUN_TEST(test_protocol_handle_unknown_method);
    RUN_TEST(test_protocol_handle_sensor_request);
    RUN_TEST(test_protocol_handle_state_request);
    RUN_TEST(test_protocol_handle_malformed_json);
}
