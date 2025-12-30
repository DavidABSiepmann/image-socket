#include <gtest/gtest.h>
#include "protobuf_helpers.h"
#include "common_test_helpers.h"

namespace {

using testing::fixtures::ProtobufHelpers;
using testing::fixtures::TestHelpers;

/**
 * Message Type Discrimination Tests
 * 
 * Verify that:
 * - Message types are correctly preserved during serialization
 * - Different types can be discriminated after deserialization
 * - Type fields are correctly parsed from bytes
 * - Unknown types are handled gracefully
 */
class TestMessageTypeDiscrimination : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }
};

// Test: Type field is preserved through roundtrip
TEST_F(TestMessageTypeDiscrimination, TypePreservedRoundtrip) {
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
        auto original = ProtobufHelpers::CreateControlMessage(type);
        auto serialized = ProtobufHelpers::SerializeMessage(original);
        
        imagesocket::control::ControlMessage deserialized;
        ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
            serialized.data(), serialized.size(), deserialized));
        
        EXPECT_EQ(deserialized.type(), type) 
            << "Type mismatch for enum value " << type;
    }
}

// Test: Different types produce different serialized forms
TEST_F(TestMessageTypeDiscrimination, DifferentTypesDifferentBytes) {
    auto msg_pause = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    auto msg_resume = ProtobufHelpers::CreateControlMessage(imagesocket::control::RESUME);
    auto msg_id = ProtobufHelpers::CreateControlMessage(imagesocket::control::ID);
    
    auto bytes_pause = ProtobufHelpers::SerializeMessage(msg_pause);
    auto bytes_resume = ProtobufHelpers::SerializeMessage(msg_resume);
    auto bytes_id = ProtobufHelpers::SerializeMessage(msg_id);
    
    // All three should be different
    EXPECT_NE(bytes_pause, bytes_resume);
    EXPECT_NE(bytes_pause, bytes_id);
    EXPECT_NE(bytes_resume, bytes_id);
}

// Test: Type can be extracted from serialized bytes
TEST_F(TestMessageTypeDiscrimination, TypeExtractableFromBytes) {
    auto original = ProtobufHelpers::CreateControlMessage(imagesocket::control::SET_FPS);
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    // Deserialize to verify type is in the bytes
    imagesocket::control::ControlMessage extracted;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), extracted));
    
    EXPECT_EQ(extracted.type(), imagesocket::control::SET_FPS);
}

// Test: Type discrimination with payload
TEST_F(TestMessageTypeDiscrimination, TypeDiscriminationWithPayload) {
    // Same payload fields but different types
    auto msg_fps = ProtobufHelpers::CreateSetFpsMessage(30, 100);
    auto msg_quality = ProtobufHelpers::CreateSetQualityMessage(30, 100);  // Same value, different field
    
    auto bytes_fps = ProtobufHelpers::SerializeMessage(msg_fps);
    auto bytes_quality = ProtobufHelpers::SerializeMessage(msg_quality);
    
    imagesocket::control::ControlMessage msg1, msg2;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_fps.data(), bytes_fps.size(), msg1));
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_quality.data(), bytes_quality.size(), msg2));
    
    EXPECT_EQ(msg1.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(msg2.type(), imagesocket::control::SET_QUALITY);
    EXPECT_NE(msg1.type(), msg2.type());
}

// Test: Type is correct even with other fields changed
TEST_F(TestMessageTypeDiscrimination, TypeIndependentOfOtherFields) {
    auto msg1 = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::ALIAS, 1, 0, 0, "device1", "");
    auto msg2 = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::ALIAS, 999, 60, 100, "device2", "different");
    
    auto bytes1 = ProtobufHelpers::SerializeMessage(msg1);
    auto bytes2 = ProtobufHelpers::SerializeMessage(msg2);
    
    imagesocket::control::ControlMessage deserialized1, deserialized2;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes1.data(), bytes1.size(), deserialized1));
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes2.data(), bytes2.size(), deserialized2));
    
    // Same type despite different other fields
    EXPECT_EQ(deserialized1.type(), imagesocket::control::ALIAS);
    EXPECT_EQ(deserialized2.type(), imagesocket::control::ALIAS);
    EXPECT_EQ(deserialized1.type(), deserialized2.type());
}

// Test: PAUSE and RESUME are clearly distinct
TEST_F(TestMessageTypeDiscrimination, PauseVsResumeClear) {
    auto msg_pause = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    auto msg_resume = ProtobufHelpers::CreateControlMessage(imagesocket::control::RESUME);
    
    auto bytes_pause = ProtobufHelpers::SerializeMessage(msg_pause);
    auto bytes_resume = ProtobufHelpers::SerializeMessage(msg_resume);
    
    imagesocket::control::ControlMessage pause_parsed, resume_parsed;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_pause.data(), bytes_pause.size(), pause_parsed));
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_resume.data(), bytes_resume.size(), resume_parsed));
    
    EXPECT_EQ(pause_parsed.type(), imagesocket::control::PAUSE);
    EXPECT_EQ(resume_parsed.type(), imagesocket::control::RESUME);
    EXPECT_NE(pause_parsed.type(), resume_parsed.type());
}

// Test: SET_FPS vs SET_QUALITY discrimination
TEST_F(TestMessageTypeDiscrimination, SetFpsVsSetQualityDiscrimination) {
    auto msg_fps = ProtobufHelpers::CreateSetFpsMessage(60, 1);
    auto msg_quality = ProtobufHelpers::CreateSetQualityMessage(90, 1);
    
    auto bytes_fps = ProtobufHelpers::SerializeMessage(msg_fps);
    auto bytes_quality = ProtobufHelpers::SerializeMessage(msg_quality);
    
    imagesocket::control::ControlMessage fps_parsed, quality_parsed;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_fps.data(), bytes_fps.size(), fps_parsed));
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        bytes_quality.data(), bytes_quality.size(), quality_parsed));
    
    EXPECT_EQ(fps_parsed.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(quality_parsed.type(), imagesocket::control::SET_QUALITY);
    
    // Can distinguish which field is set
    EXPECT_EQ(fps_parsed.fps(), 60);
    EXPECT_EQ(fps_parsed.quality(), 0);  // Default
    EXPECT_EQ(quality_parsed.quality(), 90);
    EXPECT_EQ(quality_parsed.fps(), 0);  // Default
}

// Test: Type UNKNOWN is preserved
TEST_F(TestMessageTypeDiscrimination, UnknownTypePreserved) {
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::UNKNOWN);
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    imagesocket::control::ControlMessage deserialized;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    
    EXPECT_EQ(deserialized.type(), imagesocket::control::UNKNOWN);
}

// Test: Type parsing is case-sensitive (enum values)
TEST_F(TestMessageTypeDiscrimination, AllEnumValuesDistinct) {
    // Create a message for each enum value and verify they're all different
    imagesocket::control::CommandType types[] = {
        imagesocket::control::UNKNOWN,      // 0
        imagesocket::control::PAUSE,        // 1
        imagesocket::control::RESUME,       // 2
        imagesocket::control::ID,           // 3
        imagesocket::control::REQUEST_RESUME, // 4
        imagesocket::control::SET_FPS,      // 5
        imagesocket::control::SET_QUALITY,  // 6
        imagesocket::control::SUBSCRIBE,    // 7
        imagesocket::control::UNSUBSCRIBE,  // 8
        imagesocket::control::REQUEST_ALIAS,// 9
        imagesocket::control::ALIAS         // 10
    };
    
    for (size_t i = 0; i < 11; ++i) {
        for (size_t j = i + 1; j < 11; ++j) {
            auto bytes_i = ProtobufHelpers::SerializeMessage(
                ProtobufHelpers::CreateControlMessage(types[i]));
            auto bytes_j = ProtobufHelpers::SerializeMessage(
                ProtobufHelpers::CreateControlMessage(types[j]));
            
            EXPECT_NE(bytes_i, bytes_j) 
                << "Types at index " << i << " and " << j << " produce same bytes";
        }
    }
}

// Test: Type can be determined without deserializing all fields
TEST_F(TestMessageTypeDiscrimination, TypeDeterminableWithoutFullDeserialization) {
    // This test verifies that the type field is accessible after parsing
    auto original = ProtobufHelpers::CreateFullMessage(
        imagesocket::control::SUBSCRIBE,
        9999,
        120,
        75,
        "very-long-device-identifier",
        "very-long-reason-message");
    
    auto serialized = ProtobufHelpers::SerializeMessage(original);
    
    // Parse into a message and check type first
    imagesocket::control::ControlMessage msg;
    ASSERT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), msg));
    
    // Type should be immediately available
    EXPECT_EQ(msg.type(), imagesocket::control::SUBSCRIBE);
}

// Test: Type value matches enum value
TEST_F(TestMessageTypeDiscrimination, TypeValueMatchesEnumValue) {
    // Verify the numeric values of enum constants
    EXPECT_EQ(static_cast<int>(imagesocket::control::UNKNOWN), 0);
    EXPECT_EQ(static_cast<int>(imagesocket::control::PAUSE), 1);
    EXPECT_EQ(static_cast<int>(imagesocket::control::RESUME), 2);
    EXPECT_EQ(static_cast<int>(imagesocket::control::ID), 3);
    EXPECT_EQ(static_cast<int>(imagesocket::control::REQUEST_RESUME), 4);
    EXPECT_EQ(static_cast<int>(imagesocket::control::SET_FPS), 5);
    EXPECT_EQ(static_cast<int>(imagesocket::control::SET_QUALITY), 6);
    EXPECT_EQ(static_cast<int>(imagesocket::control::SUBSCRIBE), 7);
    EXPECT_EQ(static_cast<int>(imagesocket::control::UNSUBSCRIBE), 8);
    EXPECT_EQ(static_cast<int>(imagesocket::control::REQUEST_ALIAS), 9);
    EXPECT_EQ(static_cast<int>(imagesocket::control::ALIAS), 10);
}

}  // namespace
