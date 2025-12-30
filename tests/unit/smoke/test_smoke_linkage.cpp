#include <gtest/gtest.h>

// Test library linkage by including a header from the imagesocket library
// This verifies that:
// 1. The header can be found
// 2. The library symbols are available for linking
#include "control.pb.h"

namespace {

class TestLinkage : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed - just testing that includes work
    }
};

// Test: Verify that the control.pb.h header is properly linked
TEST_F(TestLinkage, ProtobufHeaderLinked) {
    // Create a ControlMessage instance - this verifies the protobuf library is linked
    imagesocket::control::ControlMessage msg;
    
    // Verify we can set basic fields
    msg.set_client_id(1);
    msg.set_fps(30);
    
    EXPECT_EQ(msg.client_id(), 1);
    EXPECT_EQ(msg.fps(), 30);
}

// Test: Verify protobuf serialization is available
TEST_F(TestLinkage, ProtobufSerializationAvailable) {
    imagesocket::control::ControlMessage msg;
    msg.set_client_id(42);
    msg.set_type(imagesocket::control::SET_FPS);
    msg.set_fps(60);
    
    // Verify we can serialize
    std::string serialized;
    EXPECT_TRUE(msg.SerializeToString(&serialized));
    EXPECT_GT(serialized.length(), 0);
    
    // Verify we can deserialize
    imagesocket::control::ControlMessage parsed;
    EXPECT_TRUE(parsed.ParseFromString(serialized));
    EXPECT_EQ(parsed.client_id(), 42);
    EXPECT_EQ(parsed.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(parsed.fps(), 60);
}

}  // namespace
