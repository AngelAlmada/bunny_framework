#include "test_framework.h"
#include "../type_converter.h"
#include "ArduinoJson.h"

TEST_CASE(test_type_converter_validate_types)
{
    JsonDocument doc;
    doc["str"] = "hello";
    doc["num_int"] = 42;
    doc["num_float"] = 3.14159;
    doc["bool_val"] = true;
    doc["obj"]["k"] = "v";
    doc["arr"].add(1);

    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["str"], bunny::Type::STRING));
    ASSERT_FALSE(bunny::TypeConverter::validate_param_type(doc["str"], bunny::Type::NUMBER));

    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["num_int"], bunny::Type::NUMBER));
    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["num_float"], bunny::Type::NUMBER));
    ASSERT_FALSE(bunny::TypeConverter::validate_param_type(doc["num_int"], bunny::Type::STRING));

    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["bool_val"], bunny::Type::BOOLEAN));
    ASSERT_FALSE(bunny::TypeConverter::validate_param_type(doc["bool_val"], bunny::Type::STRING));

    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["obj"], bunny::Type::OBJECT));
    ASSERT_TRUE(bunny::TypeConverter::validate_param_type(doc["arr"], bunny::Type::ARRAY));
}

TEST_CASE(test_type_converter_json_to_params)
{
    JsonDocument doc;
    doc["state"] = "ON";
    doc["speed"] = 1500;
    doc["enabled"] = true;

    bunny::Params params;
    std::vector<std::string> keys;
    std::vector<std::string> vals;

    bunny::TypeConverter::json_to_params(doc.as<JsonObjectConst>(), params, keys, vals);

    ASSERT_EQ(params.size(), 3);
    ASSERT_TRUE(params.has("state"));
    ASSERT_STR_EQ(params.get_string("state"), "ON");
    ASSERT_EQ(params.get_number("speed"), 1500.0);
    ASSERT_TRUE(params.get_bool("enabled"));
}

TEST_CASE(test_type_converter_write_typed_state)
{
    JsonDocument doc;

    // String state
    ASSERT_TRUE(bunny::TypeConverter::write_typed_state_value(doc["s"], bunny::Type::STRING, "RUNNING"));
    ASSERT_STR_EQ(doc["s"].as<const char*>(), "RUNNING");

    // Number state
    ASSERT_TRUE(bunny::TypeConverter::write_typed_state_value(doc["n"], bunny::Type::NUMBER, "25.75"));
    ASSERT_EQ(doc["n"].as<double>(), 25.75);

    // Number state invalid string -> should return false
    ASSERT_FALSE(bunny::TypeConverter::write_typed_state_value(doc["n_inv"], bunny::Type::NUMBER, "not_a_num"));

    // Boolean state
    ASSERT_TRUE(bunny::TypeConverter::write_typed_state_value(doc["b1"], bunny::Type::BOOLEAN, "true"));
    ASSERT_TRUE(doc["b1"].as<bool>());
    ASSERT_TRUE(bunny::TypeConverter::write_typed_state_value(doc["b2"], bunny::Type::BOOLEAN, "false"));
    ASSERT_FALSE(doc["b2"].as<bool>());
}

void run_type_converter_tests()
{
    RUN_TEST(test_type_converter_validate_types);
    RUN_TEST(test_type_converter_json_to_params);
    RUN_TEST(test_type_converter_write_typed_state);
}
