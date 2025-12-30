#include <gtest/gtest.h>
#include "protobuf_helpers.h"
#include "common_test_helpers.h"

namespace {

using testing::fixtures::ProtobufHelpers;
using testing::fixtures::TestHelpers;

/**
 * Payload Size Limit Tests
 * 
 * Verify that:
 * - Minimal payloads can be created and transmitted
 * - Maximum payloads are within reasonable bounds
 * - Serialization size grows linearly with string fields
 * - Framework handles boundary conditions properly
 * 
 * Note: These tests focus on observable serialization sizes,
 * not on enforced limits (which may not exist in proto3).
 */
class TestPayloadSizeLimits : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }

    // Helper to check if size is reasonable
    static bool IsSizeReasonable(size_t size) {
        // Most control messages should be < 10KB
        return size > 0 && size < 10000;
    }
};

// Test: Minimal payload (only type field)
TEST_F(TestPayloadSizeLimits, MinimalPayload) {
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::UNKNOWN);
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    // Proto3 with default value: UNKNOWN (0) - field may be omitted
    // This test documents the actual behavior
    EXPECT_LT(serialized.size(), 10);  // Should be very small or empty
    EXPECT_TRUE(IsSizeReasonable(serialized.size()) || serialized.size() == 0);
}

// Test: Single byte field (type) is smallest
TEST_F(TestPayloadSizeLimits, TypeFieldOnlySmallest) {
    auto msg_type_only = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::SET_FPS);
    auto msg_with_client = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::SET_FPS, 1);
    
    auto size_type_only = ProtobufHelpers::SerializeMessage(msg_type_only).size();
    auto size_with_client = ProtobufHelpers::SerializeMessage(msg_with_client).size();
    
    // Adding a field should increase or keep size the same
    EXPECT_LE(size_type_only, size_with_client);
}

// Test: Payload grows with each field
TEST_F(TestPayloadSizeLimits, PayloadGrowthWithFields) {
    auto msg1 = ProtobufHelpers::CreateControlMessage(imagesocket::control::SET_FPS);
    auto size1 = ProtobufHelpers::SerializeMessage(msg1).size();
    
    auto msg2 = ProtobufHelpers::CreateSetFpsMessage(30, 1);
    auto size2 = ProtobufHelpers::SerializeMessage(msg2).size();
    
    auto msg3 = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SET_FPS, 1, 30, 85, "device", "reason");
    auto size3 = ProtobufHelpers::SerializeMessage(msg3).size();
    
    // Adding fields increases size (or stays same)
    EXPECT_LE(size1, size2);
    EXPECT_LE(size2, size3);
}

// Test: String field payload size
TEST_F(TestPayloadSizeLimits, StringFieldPayloadGrowth) {
    // Alias field growth
    auto msg_no_alias = ProtobufHelpers::CreateAliasMessage("", 1);
    auto msg_short_alias = ProtobufHelpers::CreateAliasMessage("abc", 1);
    auto msg_long_alias = ProtobufHelpers::CreateAliasMessage(
        "device-with-very-long-name-123456789", 1);
    
    auto size_empty = ProtobufHelpers::SerializeMessage(msg_no_alias).size();
    auto size_short = ProtobufHelpers::SerializeMessage(msg_short_alias).size();
    auto size_long = ProtobufHelpers::SerializeMessage(msg_long_alias).size();
    
    EXPECT_LE(size_empty, size_short);
    EXPECT_LE(size_short, size_long);
}

// Test: Minimal client ID (1 byte value) adds minimal overhead
TEST_F(TestPayloadSizeLimits, ClientIdMinimalOverhead) {
    auto msg_no_id = ProtobufHelpers::CreateControlMessage(imagesocket::control::ID);
    auto msg_small_id = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::ID, 1);
    auto msg_large_id = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::ID, 2147483647);  // INT32_MAX
    
    auto size_no_id = ProtobufHelpers::SerializeMessage(msg_no_id).size();
    auto size_small_id = ProtobufHelpers::SerializeMessage(msg_small_id).size();
    auto size_large_id = ProtobufHelpers::SerializeMessage(msg_large_id).size();
    
    // Zero doesn't include the field, small ID is included, large ID should be same as small
    EXPECT_LT(size_no_id, size_small_id);
    EXPECT_LE(size_small_id, size_large_id);
}

// Test: Maximum string length payload remains reasonable
TEST_F(TestPayloadSizeLimits, MaximalStringPayloadReasonable) {
    std::string max_string(10000, 'X');
    auto msg = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SET_FPS,
        999,
        60,
        100,
        max_string,  // 10KB alias
        max_string); // 10KB reason
    
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    // Should be slightly larger than 20KB (for overhead)
    EXPECT_GT(serialized.size(), 20000);
    EXPECT_LT(serialized.size(), 25000);  // Overhead < 5KB
}

// Test: 1000 messages minimal payload
TEST_F(TestPayloadSizeLimits, ManyMinimalMessages) {
    auto messages = ProtobufHelpers::GenerateValidMessages(1000);
    
    for (const auto& msg : messages) {
        auto serialized = ProtobufHelpers::SerializeMessage(msg);
        EXPECT_TRUE(IsSizeReasonable(serialized.size()));
    }
}

// Test: Payload remains valid even at boundaries
TEST_F(TestPayloadSizeLimits, BoundaryPayloadsValid) {
    // Boundary: INT32_MAX for client_id
    auto msg_max_int = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::ID, 2147483647);
    auto serialized_max = ProtobufHelpers::SerializeMessage(msg_max_int);
    imagesocket::control::ControlMessage deserialized;
    EXPECT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized_max.data(), serialized_max.size(), deserialized));
    EXPECT_EQ(deserialized.client_id(), 2147483647);
    
    // Boundary: Very long string
    std::string very_long(5000, 'Y');
    auto msg_long_string = ProtobufHelpers::CreateAliasMessage(very_long, 1);
    auto serialized_long = ProtobufHelpers::SerializeMessage(msg_long_string);
    imagesocket::control::ControlMessage deserialized_long;
    EXPECT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized_long.data(), serialized_long.size(), deserialized_long));
    EXPECT_EQ(deserialized_long.alias(), very_long);
}

// Test: Field tag and wire format overhead is minimal
TEST_F(TestPayloadSizeLimits, WireFormatOverheadMinimal) {
    // Minimal message with single small field
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    // Protobuf wire format: 1 byte field tag + 1 byte value = 2 bytes minimum
    // But empty message might be 1 byte
    EXPECT_GE(serialized.size(), 1);
    EXPECT_LE(serialized.size(), 3);
}

// Test: Serialization is deterministic (same message = same bytes)
TEST_F(TestPayloadSizeLimits, SerializationDeterministic) {
    // Create messages WITHOUT calling CreateFullMessage (which adds timestamp)
    imagesocket::control::ControlMessage msg1, msg2;
    msg1.set_type(imagesocket::control::SET_FPS);
    msg1.set_client_id(100);
    msg1.set_fps(30);
    msg1.set_quality(75);
    msg1.set_alias("test");
    msg1.set_reason("msg");
    
    msg2.set_type(imagesocket::control::SET_FPS);
    msg2.set_client_id(100);
    msg2.set_fps(30);
    msg2.set_quality(75);
    msg2.set_alias("test");
    msg2.set_reason("msg");
    
    auto bytes1 = ProtobufHelpers::SerializeMessage(msg1);
    auto bytes2 = ProtobufHelpers::SerializeMessage(msg2);
    
    // Protobuf serialization should be deterministic
    EXPECT_EQ(bytes1, bytes2);
}

// Test: Zero-valued fields don't increase payload (proto3 default behavior)
TEST_F(TestPayloadSizeLimits, ZeroFieldsOmitted) {
    auto msg_with_zeros = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::UNKNOWN, 0);
    msg_with_zeros.set_fps(0);
    msg_with_zeros.set_quality(0);
    msg_with_zeros.set_timestamp_ms(0);
    
    auto msg_natural_zeros = ProtobufHelpers::CreateControlMessage(
        imagesocket::control::UNKNOWN);
    
    auto size_explicit_zeros = ProtobufHelpers::SerializeMessage(msg_with_zeros).size();
    auto size_natural_zeros = ProtobufHelpers::SerializeMessage(msg_natural_zeros).size();
    
    // Proto3: unset fields are omitted, so sizes should be the same
    EXPECT_EQ(size_explicit_zeros, size_natural_zeros);
}

}  // namespace
