#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <gmock/gmock.h>
#include <vector>
#include <string>
#include <cstdint>

/**
 * @file mock_socket.h
 * @brief Minimal mock socket interface for testing client logic
 *
 * This fixture provides a mock socket that can be injected into client code
 * to simulate send/receive behavior without actual network operations.
 *
 * Usage:
 *   MockSocket mock_socket;
 *   EXPECT_CALL(mock_socket, Send).WillOnce(Return(true));
 *   // Pass mock_socket to client code
 */

namespace testing {
namespace fixtures {

/**
 * MockSocket: A minimal mock interface for socket operations
 * 
 * Supports basic send/receive operations that can be mocked in tests.
 * This allows testing client logic without actual socket binding.
 */
class MockSocket {
public:
    virtual ~MockSocket() = default;

    /**
     * @brief Send data through the socket
     * @param data Pointer to data buffer
     * @param size Size of data to send
     * @return true if send successful, false otherwise
     */
    virtual bool Send(const uint8_t* data, size_t size) = 0;

    /**
     * @brief Receive data from the socket
     * @param buffer Pointer to receive buffer
     * @param max_size Maximum bytes to receive
     * @return Number of bytes received, or -1 on error
     */
    virtual int Receive(uint8_t* buffer, size_t max_size) = 0;

    /**
     * @brief Check if socket is connected
     * @return true if connected, false otherwise
     */
    virtual bool IsConnected() const = 0;

    /**
     * @brief Connect to a remote address
     * @param address Remote address (e.g., "127.0.0.1")
     * @param port Remote port
     * @return true if connection successful, false otherwise
     */
    virtual bool Connect(const std::string& address, uint16_t port) = 0;

    /**
     * @brief Disconnect from remote address
     */
    virtual void Disconnect() = 0;

    /**
     * @brief Close the socket
     */
    virtual void Close() = 0;
};

/**
 * GoogleMock implementation of MockSocket for use in tests
 * 
 * Example usage:
 *   using ::testing::Return;
 *   using ::testing::AtLeast;
 *
 *   MockSocketImpl mock;
 *   EXPECT_CALL(mock, Send(_, _))
 *       .Times(AtLeast(1))
 *       .WillRepeatedly(Return(true));
 *   
 *   // Pass mock to client code
 *   MyClient client(&mock);
 *   client.SendMessage(...);
 */
class MockSocketImpl : public MockSocket {
public:
    MOCK_METHOD(bool, Send, (const uint8_t* data, size_t size), (override));
    MOCK_METHOD(int, Receive, (uint8_t * buffer, size_t max_size), (override));
    MOCK_METHOD(bool, IsConnected, (), (const, override));
    MOCK_METHOD(bool, Connect, (const std::string& address, uint16_t port), (override));
    MOCK_METHOD(void, Disconnect, (), (override));
    MOCK_METHOD(void, Close, (), (override));
};

/**
 * FakeSocket: A simple working socket implementation for basic testing
 * 
 * This implementation maintains internal state and can be used when
 * you need an actual working socket rather than a mock.
 * 
 * Example usage:
 *   FakeSocket socket;
 *   socket.Connect("127.0.0.1", 5000);
 *   socket.Send(data, size);
 */
class FakeSocket : public MockSocket {
public:
    FakeSocket() 
        : connected_(false), 
          send_buffer_(), 
          receive_queue_() {}

    bool Send(const uint8_t* data, size_t size) override {
        if (!connected_) {
            return false;
        }
        // Append to send buffer for inspection in tests
        send_buffer_.insert(send_buffer_.end(), data, data + size);
        return true;
    }

    int Receive(uint8_t* buffer, size_t max_size) override {
        if (receive_queue_.empty()) {
            return -1;  // No data available
        }
        
        const auto& packet = receive_queue_.front();
        size_t bytes_to_copy = std::min(max_size, packet.size());
        std::copy(packet.begin(), packet.begin() + bytes_to_copy, buffer);
        
        // Remove the packet from the queue after copying
        receive_queue_.erase(receive_queue_.begin());
        
        return static_cast<int>(bytes_to_copy);
    }

    bool IsConnected() const override {
        return connected_;
    }

    bool Connect(const std::string& address, uint16_t port) override {
        // Fake implementation: always succeeds
        address_ = address;
        port_ = port;
        connected_ = true;
        return true;
    }

    void Disconnect() override {
        connected_ = false;
        address_.clear();
        port_ = 0;
    }

    void Close() override {
        Disconnect();
        send_buffer_.clear();
        receive_queue_.clear();
    }

    // Test helper methods

    /**
     * @brief Get data that was sent through this socket
     * @return Vector of bytes that were sent
     */
    const std::vector<uint8_t>& GetSentData() const {
        return send_buffer_;
    }

    /**
     * @brief Queue data to be received
     * @param data Pointer to data
     * @param size Size of data
     */
    void QueueReceiveData(const uint8_t* data, size_t size) {
        std::vector<uint8_t> packet(data, data + size);
        receive_queue_.push_back(packet);
    }

    /**
     * @brief Clear the send buffer
     */
    void ClearSentData() {
        send_buffer_.clear();
    }

    /**
     * @brief Get the address this socket is connected to
     */
    const std::string& GetConnectedAddress() const {
        return address_;
    }

    /**
     * @brief Get the port this socket is connected to
     */
    uint16_t GetConnectedPort() const {
        return port_;
    }

private:
    bool connected_;
    std::string address_;
    uint16_t port_;
    std::vector<uint8_t> send_buffer_;
    std::vector<std::vector<uint8_t>> receive_queue_;
};

}  // namespace fixtures
}  // namespace testing

#endif  // MOCK_SOCKET_H
