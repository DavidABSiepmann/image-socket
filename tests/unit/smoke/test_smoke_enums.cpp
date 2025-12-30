#include <gtest/gtest.h>
#include "control.pb.h"

namespace {

class TestProtobufEnums : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed - just testing enum availability
    }
};

// Test: Verify all CommandType enum values are defined and accessible
TEST_F(TestProtobufEnums, CommandTypeEnumsDefined) {
    // Test that all enum values are defined and have expected values
    EXPECT_EQ(imagesocket::control::UNKNOWN, 0);
    EXPECT_EQ(imagesocket::control::PAUSE, 1);
    EXPECT_EQ(imagesocket::control::RESUME, 2);
    EXPECT_EQ(imagesocket::control::ID, 3);
    EXPECT_EQ(imagesocket::control::REQUEST_RESUME, 4);
    EXPECT_EQ(imagesocket::control::SET_FPS, 5);
    EXPECT_EQ(imagesocket::control::SET_QUALITY, 6);
    EXPECT_EQ(imagesocket::control::SUBSCRIBE, 7);
    EXPECT_EQ(imagesocket::control::UNSUBSCRIBE, 8);
    EXPECT_EQ(imagesocket::control::REQUEST_ALIAS, 9);
    EXPECT_EQ(imagesocket::control::ALIAS, 10);
}

// Test: Verify CommandType enum values can be set in ControlMessage
TEST_F(TestProtobufEnums, CommandTypeCanBeSet) {
    imagesocket::control::ControlMessage msg;
    
    // Test setting various command types
    msg.set_type(imagesocket::control::PAUSE);
    EXPECT_EQ(msg.type(), imagesocket::control::PAUSE);
    
    msg.set_type(imagesocket::control::SET_FPS);
    EXPECT_EQ(msg.type(), imagesocket::control::SET_FPS);
    
    msg.set_type(imagesocket::control::ALIAS);
    EXPECT_EQ(msg.type(), imagesocket::control::ALIAS);
}

// Test: Verify default enum value is UNKNOWN
TEST_F(TestProtobufEnums, DefaultCommandTypeIsUnknown) {
    imagesocket::control::ControlMessage msg;
    
    // Default value should be UNKNOWN (0)
    EXPECT_EQ(msg.type(), imagesocket::control::UNKNOWN);
}

// Test: Verify enum names are available in CommandType_Name
TEST_F(TestProtobufEnums, CommandTypeNamesAvailable) {
    // Verify that the _Name function exists and works
    // (available in protobuf for enum reflection)
    using imagesocket::control::CommandType_Name;
    
    EXPECT_EQ(CommandType_Name(imagesocket::control::UNKNOWN), "UNKNOWN");
    EXPECT_EQ(CommandType_Name(imagesocket::control::SET_FPS), "SET_FPS");
    EXPECT_EQ(CommandType_Name(imagesocket::control::ALIAS), "ALIAS");
}

}  // namespace
