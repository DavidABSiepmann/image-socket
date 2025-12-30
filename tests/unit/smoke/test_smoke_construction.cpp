#include <gtest/gtest.h>
#include "control.pb.h"

namespace {

class TestConstruction : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }
};

// Test: Verify ControlMessage can be constructed
TEST_F(TestConstruction, ControlMessageConstruction) {
    // Should be able to construct without errors
    imagesocket::control::ControlMessage msg;
    
    // Verify it's created with default values
    EXPECT_EQ(msg.type(), imagesocket::control::UNKNOWN);
    EXPECT_EQ(msg.client_id(), 0);
    EXPECT_EQ(msg.fps(), 0);
    EXPECT_EQ(msg.quality(), 0);
    EXPECT_TRUE(msg.reason().empty());
    EXPECT_EQ(msg.timestamp_ms(), 0);
    EXPECT_TRUE(msg.alias().empty());
}

// Test: Verify ControlMessage fields can be populated
TEST_F(TestConstruction, ControlMessageFieldPopulation) {
    imagesocket::control::ControlMessage msg;
    
    // Populate all fields
    msg.set_type(imagesocket::control::SET_FPS);
    msg.set_client_id(123);
    msg.set_fps(30);
    msg.set_quality(85);
    msg.set_reason("test reason");
    msg.set_timestamp_ms(1234567890);
    msg.set_alias("client-alias");
    
    // Verify all fields are set correctly
    EXPECT_EQ(msg.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(msg.client_id(), 123);
    EXPECT_EQ(msg.fps(), 30);
    EXPECT_EQ(msg.quality(), 85);
    EXPECT_EQ(msg.reason(), "test reason");
    EXPECT_EQ(msg.timestamp_ms(), 1234567890);
    EXPECT_EQ(msg.alias(), "client-alias");
}

// Test: Verify ControlMessage copy semantics work
TEST_F(TestConstruction, ControlMessageCopySemantics) {
    imagesocket::control::ControlMessage msg1;
    msg1.set_client_id(456);
    msg1.set_fps(60);
    msg1.set_alias("original");
    
    // Copy construct
    imagesocket::control::ControlMessage msg2(msg1);
    
    EXPECT_EQ(msg2.client_id(), 456);
    EXPECT_EQ(msg2.fps(), 60);
    EXPECT_EQ(msg2.alias(), "original");
    
    // Assignment
    imagesocket::control::ControlMessage msg3;
    msg3 = msg1;
    
    EXPECT_EQ(msg3.client_id(), 456);
    EXPECT_EQ(msg3.fps(), 60);
    EXPECT_EQ(msg3.alias(), "original");
}

// Test: Verify ControlMessage can be cleared
TEST_F(TestConstruction, ControlMessageClear) {
    imagesocket::control::ControlMessage msg;
    msg.set_client_id(789);
    msg.set_fps(45);
    msg.set_alias("test");
    
    // Clear should reset to default values
    msg.Clear();
    
    EXPECT_EQ(msg.client_id(), 0);
    EXPECT_EQ(msg.fps(), 0);
    EXPECT_TRUE(msg.alias().empty());
    EXPECT_EQ(msg.type(), imagesocket::control::UNKNOWN);
}

}  // namespace
