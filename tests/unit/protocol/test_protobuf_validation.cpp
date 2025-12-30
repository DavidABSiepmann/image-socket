#include <gtest/gtest.h>
#include "protobuf_helpers.h"
#include "common_test_helpers.h"

namespace {

using testing::fixtures::ProtobufHelpers;
using testing::fixtures::TestHelpers;

/**
 * Protobuf Message Validation Tests
 * 
 * Verify that:
 * - Required fields can have default values (proto3 behavior)
 * - Value ranges are preserved
 * - String fields accept empty and non-empty values
 * - Timestamp fields work correctly
 * 
 * Note: Proto3 does not enforce "required" fields at serialization time.
 * All fields have default values, and missing fields become defaults
 * when deserialized. These tests verify proto3 semantics.
 */
class TestProtobufValidation : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }
};

// Test: Message with default values
TEST_F(TestProtobufValidation, DefaultValuesAfterConstruction) {
    imagesocket::control::ControlMessage msg;
    
    EXPECT_EQ(msg.type(), imagesocket::control::UNKNOWN);
    EXPECT_EQ(msg.client_id(), 0);
    EXPECT_EQ(msg.fps(), 0);
    EXPECT_EQ(msg.quality(), 0);
    EXPECT_TRUE(msg.reason().empty());
    EXPECT_EQ(msg.timestamp_ms(), 0);
    EXPECT_TRUE(msg.alias().empty());
}

// Test: Type field accepts all enum values
TEST_F(TestProtobufValidation, AllTypeEnumsValid) {
    imagesocket::control::CommandType types[] = {
        imagesocket::control::UNKNOWN,
        imagesocket::control::PAUSE,
        imagesocket::control::RESUME,
        imagesocket::control::ID,
        imagesocket::control::REQUEST_RESUME,
        imagesocket::control::SET_FPS,
        imagesocket::control::SET_QUALITY,
        imagesocket::control::SUBSCRIBE,
        imagesocket::control::UNSUBSCRIBE,
        imagesocket::control::REQUEST_ALIAS,
        imagesocket::control::ALIAS
    };
    
    for (auto type : types) {
        imagesocket::control::ControlMessage msg;
        msg.set_type(type);
        EXPECT_EQ(msg.type(), type);
    }
}

// Test: Client ID accepts 0 and positive values
TEST_F(TestProtobufValidation, ClientIdAcceptsValidRange) {
    imagesocket::control::ControlMessage msg;
    
    msg.set_client_id(0);
    EXPECT_EQ(msg.client_id(), 0);
    
    msg.set_client_id(1);
    EXPECT_EQ(msg.client_id(), 1);
    
    msg.set_client_id(2147483647);  // INT32_MAX
    EXPECT_EQ(msg.client_id(), 2147483647);
}

// Test: FPS field accepts typical frame rate values
TEST_F(TestProtobufValidation, FpsAcceptsTypicalValues) {
    int32_t fps_values[] = {1, 24, 25, 30, 60, 120, 240};
    
    for (auto fps : fps_values) {
        imagesocket::control::ControlMessage msg;
        msg.set_fps(fps);
        EXPECT_EQ(msg.fps(), fps);
    }
}

// Test: FPS field accepts zero and negative values (no validation in proto)
TEST_F(TestProtobufValidation, FpsAcceptsEdgeValues) {
    imagesocket::control::ControlMessage msg;
    
    msg.set_fps(0);
    EXPECT_EQ(msg.fps(), 0);
    
    msg.set_fps(-1);
    EXPECT_EQ(msg.fps(), -1);
    
    msg.set_fps(65535);
    EXPECT_EQ(msg.fps(), 65535);
}

// Test: Quality field accepts 0-100 range
TEST_F(TestProtobufValidation, QualityAcceptsValidRange) {
    int32_t quality_values[] = {0, 1, 50, 75, 90, 99, 100};
    
    for (auto quality : quality_values) {
        imagesocket::control::ControlMessage msg;
        msg.set_quality(quality);
        EXPECT_EQ(msg.quality(), quality);
    }
}

// Test: Quality field accepts values outside typical range (no validation)
TEST_F(TestProtobufValidation, QualityAcceptsAnyIntValue) {
    imagesocket::control::ControlMessage msg;
    
    msg.set_quality(-100);
    EXPECT_EQ(msg.quality(), -100);
    
    msg.set_quality(1000);
    EXPECT_EQ(msg.quality(), 1000);
}

// Test: Alias string field accepts empty string
TEST_F(TestProtobufValidation, AliasAcceptsEmptyString) {
    imagesocket::control::ControlMessage msg;
    msg.set_alias("");
    
    EXPECT_TRUE(msg.alias().empty());
    EXPECT_EQ(msg.alias().length(), 0);
}

// Test: Alias string field accepts various strings
TEST_F(TestProtobufValidation, AliasAcceptsVariousStrings) {
    std::string test_strings[] = {
        "device-1",
        "Client With Spaces",
        "123456",
        "client_with_underscores",
        "UPPERCASE",
        "mixed-Case_123"
    };
    
    for (const auto& alias : test_strings) {
        imagesocket::control::ControlMessage msg;
        msg.set_alias(alias);
        EXPECT_EQ(msg.alias(), alias);
    }
}

// Test: Reason string field accepts empty and non-empty values
TEST_F(TestProtobufValidation, ReasonStringHandling) {
    imagesocket::control::ControlMessage msg;
    
    // Empty
    msg.set_reason("");
    EXPECT_TRUE(msg.reason().empty());
    
    // Non-empty
    msg.set_reason("Connection timeout");
    EXPECT_EQ(msg.reason(), "Connection timeout");
    
    // With special characters
    msg.set_reason("Error: invalid\nvalue");
    EXPECT_EQ(msg.reason(), "Error: invalid\nvalue");
}

// Test: Timestamp field accepts 0 and large values
TEST_F(TestProtobufValidation, TimestampAcceptsValidRange) {
    imagesocket::control::ControlMessage msg;
    
    msg.set_timestamp_ms(0);
    EXPECT_EQ(msg.timestamp_ms(), 0);
    
    msg.set_timestamp_ms(1000000000);
    EXPECT_EQ(msg.timestamp_ms(), 1000000000);
    
    msg.set_timestamp_ms(9223372036854775807LL);  // INT64_MAX
    EXPECT_EQ(msg.timestamp_ms(), 9223372036854775807LL);
}

// Test: Multiple fields can be set independently
TEST_F(TestProtobufValidation, IndependentFieldSetting) {
    imagesocket::control::ControlMessage msg;
    
    msg.set_type(imagesocket::control::SET_FPS);
    msg.set_client_id(100);
    msg.set_fps(30);
    
    // Changing one field doesn't affect others
    msg.set_quality(75);
    EXPECT_EQ(msg.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(msg.client_id(), 100);
    EXPECT_EQ(msg.fps(), 30);
    EXPECT_EQ(msg.quality(), 75);
    
    msg.set_alias("device");
    EXPECT_EQ(msg.alias(), "device");
    EXPECT_EQ(msg.quality(), 75);  // Still 75
}

// Test: Fields can be cleared and reset
TEST_F(TestProtobufValidation, FieldsClearedAfterClear) {
    imagesocket::control::ControlMessage msg;
    msg.set_client_id(99);
    msg.set_fps(60);
    msg.set_alias("test");
    
    msg.Clear();
    
    EXPECT_EQ(msg.client_id(), 0);
    EXPECT_EQ(msg.fps(), 0);
    EXPECT_TRUE(msg.alias().empty());
    EXPECT_EQ(msg.type(), imagesocket::control::UNKNOWN);
}

// Test: UTF-8 strings are accepted (no validation in proto)
TEST_F(TestProtobufValidation, Utf8StringsAccepted) {
    imagesocket::control::ControlMessage msg;
    
    // UTF-8 encoded string with non-ASCII characters
    msg.set_alias("café");  // Contains é (U+00E9)
    EXPECT_EQ(msg.alias(), "café");
    
    // More complex UTF-8
    msg.set_reason("日本語");  // Japanese characters
    EXPECT_EQ(msg.reason(), "日本語");
}

// Test: Very long strings are accepted
TEST_F(TestProtobufValidation, VeryLongStringsAccepted) {
    std::string long_string(10000, 'A');
    
    imagesocket::control::ControlMessage msg;
    msg.set_alias(long_string);
    
    EXPECT_EQ(msg.alias().length(), 10000);
    EXPECT_EQ(msg.alias(), long_string);
}

// Test: Roundtrip preserves all field values exactly
TEST_F(TestProtobufValidation, RoundtripPreservesExactValues) {
    auto original = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SET_QUALITY,
        12345,
        99,
        88,
        "test-device-with-long-name",
        "Reason with special chars: @#$%");
    
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_TRUE(ProtobufHelpers::MessagesEqual(original, deserialized));
}

}  // namespace
