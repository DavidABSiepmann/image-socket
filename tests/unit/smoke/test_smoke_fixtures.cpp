#include <gtest/gtest.h>
#include "protobuf_helpers.h"
#include "common_test_helpers.h"
#include "mock_socket.h"

namespace {

using testing::fixtures::ProtobufHelpers;
using testing::fixtures::TestHelpers;
using testing::fixtures::FakeSocket;
using testing::fixtures::MockSocketImpl;

/**
 * Smoke test to verify that all fixtures are properly linked and usable
 */
class TestFixturesUsage : public ::testing::Test {
protected:
    void SetUp() override {
        // Fixtures are available and can be instantiated
    }
};

// Test: Verify ProtobufHelpers factory methods work
TEST_F(TestFixturesUsage, ProtobufHelpersFactoryMethods) {
    auto msg = ProtobufHelpers::CreateSetFpsMessage(30);
    EXPECT_EQ(msg.type(), imagesocket::control::SET_FPS);
    EXPECT_EQ(msg.fps(), 30);
}

// Test: Verify ProtobufHelpers serialization works
TEST_F(TestFixturesUsage, ProtobufHelpersSerialization) {
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::PAUSE);
    auto serialized = ProtobufHelpers::SerializeMessage(msg);
    
    EXPECT_GT(serialized.size(), 0);
    
    imagesocket::control::ControlMessage deserialized;
    EXPECT_TRUE(ProtobufHelpers::DeserializeMessage(
        serialized.data(), serialized.size(), deserialized));
    EXPECT_EQ(deserialized.type(), imagesocket::control::PAUSE);
}

// Test: Verify TestHelpers byte order conversion
TEST_F(TestFixturesUsage, TestHelpersByteOrder) {
    uint32_t value = 0x12345678;
    uint32_t network = TestHelpers::HostToNetworkByteOrder(value);
    uint32_t back = TestHelpers::NetworkToHostByteOrder(network);
    
    EXPECT_EQ(back, value);
}

// Test: Verify TestHelpers buffer utilities
TEST_F(TestFixturesUsage, TestHelpersBufferUtilities) {
    auto buffer = TestHelpers::CreateFilledBuffer(10, 0xFF);
    
    EXPECT_EQ(buffer.size(), 10);
    EXPECT_TRUE(TestHelpers::BufferAllBytesEqual(buffer.data(), buffer.size(), 0xFF));
}

// Test: Verify FakeSocket implementation
TEST_F(TestFixturesUsage, FakeSocketImplementation) {
    FakeSocket socket;
    
    EXPECT_FALSE(socket.IsConnected());
    EXPECT_TRUE(socket.Connect("127.0.0.1", 5000));
    EXPECT_TRUE(socket.IsConnected());
    
    uint8_t data[] = {0x01, 0x02, 0x03};
    EXPECT_TRUE(socket.Send(data, 3));
    
    EXPECT_EQ(socket.GetConnectedAddress(), "127.0.0.1");
    EXPECT_EQ(socket.GetConnectedPort(), 5000);
}

// Test: Verify MockSocketImpl can be used with GoogleMock
TEST_F(TestFixturesUsage, MockSocketImplWithGoogleMock) {
    using ::testing::Return;
    
    MockSocketImpl mock;
    EXPECT_CALL(mock, IsConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    
    EXPECT_FALSE(mock.IsConnected());
    EXPECT_TRUE(mock.IsConnected());
}

// Test: Verify length-prefixed frame creation
TEST_F(TestFixturesUsage, LengthPrefixedFrames) {
    auto msg = ProtobufHelpers::CreateControlMessage(imagesocket::control::ID);
    auto frame = ProtobufHelpers::CreateLengthPrefixedFrame(msg);
    
    EXPECT_GT(frame.size(), 4);  // At least length prefix
    
    uint32_t length = 0;
    std::vector<uint8_t> payload;
    EXPECT_TRUE(ProtobufHelpers::ParseLengthPrefixedFrame(
        frame.data(), frame.size(), length, payload));
    
    EXPECT_GT(length, 0);
    EXPECT_GT(payload.size(), 0);
}

}  // namespace
