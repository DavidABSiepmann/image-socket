#include <gtest/gtest.h>
#include "protobuf_helpers.h"
#include "common_test_helpers.h"

namespace {

using testing::fixtures::ProtobufHelpers;
using testing::fixtures::TestHelpers;

/**
 * Protobuf Serialization Roundtrip Tests
 * 
 * Verify that:
 * - Messages can be serialized to bytes
 * - Serialized bytes can be deserialized back to equivalent messages
 * - All field values are preserved through the roundtrip
 * - Message equality works after roundtrip
 */
class TestProtobufSerialization : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed - testing pure serialization
    }
};

// Test: Basic serialization with type field only
TEST_F(TestProtobufSerialization, SerializeBasicMessage) {
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    EXPECT_GT(serialized.size(), 0);
    EXPECT_LT(serialized.size(), 1024);  // Should be small for minimal message
}

// Test: Roundtrip serialization with type field
TEST_F(TestProtobufSerialization, RoundtripBasicMessage) {
    auto original = ProtobufHelpers::CreateControlMessage(imagesocket::control::RESUME);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.type(), original.type());
}

// Test: Roundtrip with client_id field
TEST_F(TestProtobufSerialization, RoundtripWithClientId) {
    auto original = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::ID, 42);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.client_id(), 42);
    EXPECT_EQ(deserialized.type(), imagesocket::control::ID);
}

// Test: Roundtrip with FPS field
TEST_F(TestProtobufSerialization, RoundtripWithFps) {
    auto original = ProtobufHelpers::CreateSetFpsMessage(30, 100);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.fps(), 30);
    EXPECT_EQ(deserialized.client_id(), 100);
    EXPECT_EQ(deserialized.type(), imagesocket::control::SET_FPS);
}

// Test: Roundtrip with quality field
TEST_F(TestProtobufSerialization, RoundtripWithQuality) {
    auto original = ProtobufHelpers::CreateSetQualityMessage(85, 200);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.quality(), 85);
    EXPECT_EQ(deserialized.client_id(), 200);
    EXPECT_EQ(deserialized.type(), imagesocket::control::SET_QUALITY);
}

// Test: Roundtrip with alias string field
TEST_F(TestProtobufSerialization, RoundtripWithAlias) {
    auto original = ProtobufHelpers::CreateAliasMessage("my-device", 300);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.alias(), "my-device");
    EXPECT_EQ(deserialized.client_id(), 300);
    EXPECT_EQ(deserialized.type(), imagesocket::control::ALIAS);
}

// Test: Roundtrip with all fields populated
TEST_F(TestProtobufSerialization, RoundtripFullMessage) {
    auto original = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SET_FPS,
        999,
        60,
        90,
        "complete-client",
        "test roundtrip");
    
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    // Verify all fields match
    EXPECT_EQ(deserialized.type(), original.type());
    EXPECT_EQ(deserialized.client_id(), original.client_id());
    EXPECT_EQ(deserialized.fps(), original.fps());
    EXPECT_EQ(deserialized.quality(), original.quality());
    EXPECT_EQ(deserialized.alias(), original.alias());
    EXPECT_EQ(deserialized.reason(), original.reason());
    EXPECT_EQ(deserialized.timestamp_ms(), original.timestamp_ms());
}

// Test: Multiple roundtrips are consistent
TEST_F(TestProtobufSerialization, MultipleRoundtripsConsistent) {
    auto original = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::ALIAS,
        1,
        24,
        75,
        "client-a",
        "first roundtrip");
    
    // First roundtrip
    auto serialized1 = ProtobufHelpers::SerializeMessage(original);
    imagesocket::control::ControlMessage deserialized1;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized1.data(), serialized1.size(), deserialized1));
    
    // Second roundtrip
    auto serialized2 = ProtobufHelpers::SerializeMessage(deserialized1);
    imagesocket::control::ControlMessage deserialized2;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized2.data(), serialized2.size(), deserialized2));
    
    // Both serializations should be identical (deterministic)
    EXPECT_EQ(serialized1, serialized2);
    
    // All deserialized messages should match
    EXPECT_TRUE(ProtobufHelpers::MessagesEqual(original, deserialized1));
    EXPECT_TRUE(ProtobufHelpers::MessagesEqual(deserialized1, deserialized2));
}

// Test: Different message types serialize to different bytes
TEST_F(TestProtobufSerialization, DifferentTypesProduceDifferentSerializations) {
    auto msg_pause = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    auto msg_resume = ProtobufHelpers::CreateControlMessage(imagesocket::control::RESUME);
    
    auto serialized_pause = ProtobufHelpers::SerializeMessage(msg_pause);
    auto serialized_resume = ProtobufHelpers::SerializeMessage(msg_resume);
    
    // Different types should produce different serializations
    EXPECT_NE(serialized_pause, serialized_resume);
}

// Test: Empty alias field is handled correctly
TEST_F(TestProtobufSerialization, EmptyAliasRoundtrip) {
    auto original = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::ALIAS, 50);
    original.set_alias("");  // Explicitly empty
    
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_TRUE(deserialized.alias().empty());
}

// Test: Serialization size is reasonable (< 10KB for normal messages)
TEST_F(TestProtobufSerialization, SerializationSizeReasonable) {
    auto msg = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SET_FPS,
        999999,
        65535,
        100,
        std::string(100, 'A'),
        std::string(100, 'B'));
    
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    EXPECT_GT(serialized.size(), 0);
    EXPECT_LT(serialized.size(), 10000);  // Should still be small
}

}  // namespace
