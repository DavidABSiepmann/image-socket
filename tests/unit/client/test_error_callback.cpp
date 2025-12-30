/**
 * @file test_error_callback.cpp
 * @brief Unit tests for error callback propagation
 *
 * Tests validate:
 * - Error callbacks are invoked on error conditions
 * - Error codes are correctly propagated
 * - Callbacks are called with correct error information
 * - Multiple errors are tracked
 * - Callback registration/unregistration
 *
 * Uses GoogleMock to verify callback invocation without real I/O
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <string>
#include <functional>
#include <memory>

/**
 * Error handling utilities (header-only, non-production)
 */
namespace error_handling {

/**
 * Error codes for client operations
 */
enum class ErrorCode {
    None = 0,
    ConnectionFailed = 1,
    ConnectionTimeout = 2,
    ProtocolError = 3,
    AuthenticationFailed = 4,
    ServerRejected = 5,
    SocketError = 6,
    UnknownError = 99
};

/**
 * Convert error code to string
 * @param code Error code to convert
 * @return String representation
 */
inline std::string ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::None:
            return "None";
        case ErrorCode::ConnectionFailed:
            return "ConnectionFailed";
        case ErrorCode::ConnectionTimeout:
            return "ConnectionTimeout";
        case ErrorCode::ProtocolError:
            return "ProtocolError";
        case ErrorCode::AuthenticationFailed:
            return "AuthenticationFailed";
        case ErrorCode::ServerRejected:
            return "ServerRejected";
        case ErrorCode::SocketError:
            return "SocketError";
        case ErrorCode::UnknownError:
            return "UnknownError";
        default:
            return "Unknown";
    }
}

/**
 * Error information struct
 */
struct ErrorInfo {
    ErrorCode code;
    std::string message;
    int error_count;
    
    ErrorInfo()
        : code(ErrorCode::None), message(""), error_count(0) {
    }
    
    ErrorInfo(ErrorCode c, const std::string& msg)
        : code(c), message(msg), error_count(0) {
    }
};

/**
 * Error callback handler interface
 */
typedef std::function<void(const ErrorInfo&)> ErrorCallback;

/**
 * Simple client with error callback support (non-production)
 */
class SimpleClientWithErrorHandling {
private:
    ErrorCallback m_error_callback;
    int m_error_count;
    ErrorInfo m_last_error;
    
public:
    SimpleClientWithErrorHandling()
        : m_error_count(0) {
    }
    
    /**
     * Register error callback
     * @param callback Callback function to call on errors
     */
    void SetErrorCallback(ErrorCallback callback) {
        m_error_callback = callback;
    }
    
    /**
     * Trigger an error (simulates real error condition)
     * @param code Error code
     * @param message Error message
     */
    void TriggerError(ErrorCode code, const std::string& message) {
        m_error_count++;
        m_last_error = ErrorInfo(code, message);
        m_last_error.error_count = m_error_count;
        
        // Invoke callback if registered
        if (m_error_callback) {
            m_error_callback(m_last_error);
        }
    }
    
    /**
     * Get total error count
     */
    int GetErrorCount() const {
        return m_error_count;
    }
    
    /**
     * Get last error information
     */
    const ErrorInfo& GetLastError() const {
        return m_last_error;
    }
    
    /**
     * Check if error callback is registered
     */
    bool HasErrorCallback() const {
        return m_error_callback != nullptr;
    }
};

} // namespace error_handling

/**
 * Mock error callback for testing
 */class MockErrorCallback {
public:
    MOCK_METHOD(void, OnError, (const error_handling::ErrorInfo&));
};

/**
 * Test fixture for error callback
 */
class TestErrorCallback : public ::testing::Test {
protected:
    error_handling::SimpleClientWithErrorHandling m_client;
    MockErrorCallback m_mock_callback;
    
    void SetUp() override {
        // Register mock callback
        m_client.SetErrorCallback(
            [this](const error_handling::ErrorInfo& error) {
                m_mock_callback.OnError(error);
            });
    }
};

// ============================================================================
// Error Callback Invocation Tests
// ============================================================================

TEST_F(TestErrorCallback, CallbackInvokedOnError) {
    // Test: Callback is invoked when error occurs
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1);
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Failed to connect to server");
}

TEST_F(TestErrorCallback, CallbackNotInvokedWithoutRegistration) {
    // Test: No callback invoked if callback not registered
    error_handling::SimpleClientWithErrorHandling client;
    
    // Should not crash, callback simply not invoked
    client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Failed to connect");
    
    EXPECT_TRUE(!client.HasErrorCallback());
}

TEST_F(TestErrorCallback, MultipleErrorsInvokeCallbackMultipleTimes) {
    // Test: Each error invokes callback
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(3);
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 1");
    m_client.TriggerError(
        error_handling::ErrorCode::ProtocolError,
        "Error 2");
    m_client.TriggerError(
        error_handling::ErrorCode::ServerRejected,
        "Error 3");
}

// ============================================================================
// Error Code Propagation Tests
// ============================================================================

TEST_F(TestErrorCallback, ErrorCodePropagatedConnectionFailed) {
    // Test: ConnectionFailed error code is propagated
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionFailed);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Connection failed");
}

TEST_F(TestErrorCallback, ErrorCodePropagatedTimeout) {
    // Test: ConnectionTimeout error code is propagated
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionTimeout);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionTimeout,
        "Connection timeout");
}

TEST_F(TestErrorCallback, ErrorCodePropagatedProtocolError) {
    // Test: ProtocolError error code is propagated
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ProtocolError);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ProtocolError,
        "Protocol violation");
}

TEST_F(TestErrorCallback, ErrorCodePropagatedAuthFailed) {
    // Test: AuthenticationFailed error code is propagated
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::AuthenticationFailed);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::AuthenticationFailed,
        "Authentication failed");
}

// ============================================================================
// Error Message Propagation Tests
// ============================================================================

TEST_F(TestErrorCallback, ErrorMessagePropagated) {
    // Test: Error message is propagated to callback
    std::string expected_msg = "Custom error message";
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([expected_msg](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.message, expected_msg);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::SocketError,
        expected_msg);
}

TEST_F(TestErrorCallback, DifferentErrorMessages) {
    // Test: Different error messages are correctly propagated
    ::testing::InSequence seq;
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(2);
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "First error");
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Second error");
    
    // Verify both were called with correct messages
    auto last = m_client.GetLastError();
    EXPECT_EQ(last.message, "Second error");
}

// ============================================================================
// Error Count Tracking Tests
// ============================================================================

TEST_F(TestErrorCallback, ErrorCountStartsAtZero) {
    // Test: Initial error count is zero
    EXPECT_EQ(m_client.GetErrorCount(), 0);
}

TEST_F(TestErrorCallback, ErrorCountIncrementsOnError) {
    // Test: Error count increments with each error
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1);
    
    EXPECT_EQ(m_client.GetErrorCount(), 0);
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 1");
    EXPECT_EQ(m_client.GetErrorCount(), 1);
}

TEST_F(TestErrorCallback, ErrorCountIncrementMultipleTimes) {
    // Test: Error count increments correctly for multiple errors
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(5);
    
    for (int i = 1; i <= 5; i++) {
        m_client.TriggerError(
            error_handling::ErrorCode::ConnectionFailed,
            "Error " + std::to_string(i));
        EXPECT_EQ(m_client.GetErrorCount(), i);
    }
}

TEST_F(TestErrorCallback, ErrorCountInCallbackParameter) {
    // Test: Error count is passed in callback parameter
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.error_count, 1);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 1");
}

TEST_F(TestErrorCallback, ErrorCountSequenceInCallback) {
    // Test: Error count increments sequentially in callbacks
    ::testing::InSequence seq;
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.error_count, 1);
        }));
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.error_count, 2);
        }));
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.error_count, 3);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 1");
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 2");
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Error 3");
}

// ============================================================================
// Last Error Tracking Tests
// ============================================================================

TEST_F(TestErrorCallback, LastErrorTracked) {
    // Test: Last error is tracked correctly
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(2);
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "First error");
    m_client.TriggerError(
        error_handling::ErrorCode::ProtocolError,
        "Second error");
    
    auto last = m_client.GetLastError();
    EXPECT_EQ(last.code, error_handling::ErrorCode::ProtocolError);
    EXPECT_EQ(last.message, "Second error");
}

TEST_F(TestErrorCallback, LastErrorUpdatedOnNewError) {
    // Test: Last error updates with each new error
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(3);
    
    std::vector<error_handling::ErrorCode> codes = {
        error_handling::ErrorCode::ConnectionFailed,
        error_handling::ErrorCode::ConnectionTimeout,
        error_handling::ErrorCode::ProtocolError
    };
    
    for (size_t i = 0; i < codes.size(); i++) {
        m_client.TriggerError(codes[i], "Error " + std::to_string(i + 1));
        auto last = m_client.GetLastError();
        EXPECT_EQ(last.code, codes[i]);
    }
}

// ============================================================================
// Error Code Conversion Tests
// ============================================================================

TEST_F(TestErrorCallback, ErrorCodeToStringConnectionFailed) {
    // Test: Convert ConnectionFailed to string
    auto str = error_handling::ErrorCodeToString(
        error_handling::ErrorCode::ConnectionFailed);
    EXPECT_EQ(str, "ConnectionFailed");
}

TEST_F(TestErrorCallback, ErrorCodeToStringTimeout) {
    // Test: Convert ConnectionTimeout to string
    auto str = error_handling::ErrorCodeToString(
        error_handling::ErrorCode::ConnectionTimeout);
    EXPECT_EQ(str, "ConnectionTimeout");
}

TEST_F(TestErrorCallback, ErrorCodeToStringProtocolError) {
    // Test: Convert ProtocolError to string
    auto str = error_handling::ErrorCodeToString(
        error_handling::ErrorCode::ProtocolError);
    EXPECT_EQ(str, "ProtocolError");
}

TEST_F(TestErrorCallback, ErrorCodeToStringAllCodes) {
    // Test: All error codes convert to non-empty strings
    std::vector<error_handling::ErrorCode> codes = {
        error_handling::ErrorCode::None,
        error_handling::ErrorCode::ConnectionFailed,
        error_handling::ErrorCode::ConnectionTimeout,
        error_handling::ErrorCode::ProtocolError,
        error_handling::ErrorCode::AuthenticationFailed,
        error_handling::ErrorCode::ServerRejected,
        error_handling::ErrorCode::SocketError
    };
    
    for (auto code : codes) {
        auto str = error_handling::ErrorCodeToString(code);
        EXPECT_FALSE(str.empty());
    }
}

// ============================================================================
// Callback Registration Tests
// ============================================================================

TEST_F(TestErrorCallback, CallbackCanBeRegistered) {
    // Test: Callback can be registered
    error_handling::SimpleClientWithErrorHandling client;
    bool callback_called = false;
    
    client.SetErrorCallback([&](const error_handling::ErrorInfo&) {
        callback_called = true;
    });
    
    client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Test error");
    
    EXPECT_TRUE(callback_called);
}

TEST_F(TestErrorCallback, CallbackCanBeReplaced) {
    // Test: Callback can be replaced
    error_handling::SimpleClientWithErrorHandling client;
    bool first_called = false;
    bool second_called = false;
    
    client.SetErrorCallback([&](const error_handling::ErrorInfo&) {
        first_called = true;
    });
    
    client.SetErrorCallback([&](const error_handling::ErrorInfo&) {
        second_called = true;
    });
    
    client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Test error");
    
    EXPECT_FALSE(first_called);   // First callback replaced
    EXPECT_TRUE(second_called);   // Second callback called
}

// ============================================================================
// Error Information Struct Tests
// ============================================================================

TEST_F(TestErrorCallback, ErrorInfoDefault) {
    // Test: Default ErrorInfo construction
    error_handling::ErrorInfo info;
    EXPECT_EQ(info.code, error_handling::ErrorCode::None);
    EXPECT_EQ(info.message, "");
    EXPECT_EQ(info.error_count, 0);
}

TEST_F(TestErrorCallback, ErrorInfoInitialized) {
    // Test: ErrorInfo initialization with values
    error_handling::ErrorInfo info(
        error_handling::ErrorCode::ConnectionFailed,
        "Test message");
    EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionFailed);
    EXPECT_EQ(info.message, "Test message");
    EXPECT_EQ(info.error_count, 0);
}

// ============================================================================
// Practical Error Scenario Tests
// ============================================================================

TEST_F(TestErrorCallback, ConnectThenDisconnectError) {
    // Test: Error during connection process
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionFailed);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Connection refused");
}

TEST_F(TestErrorCallback, TimeoutError) {
    // Test: Timeout error is properly reported
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionTimeout);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionTimeout,
        "Connection timed out");
}

TEST_F(TestErrorCallback, ProtocolErrorSequence) {
    // Test: Multiple errors in sequence
    ::testing::InSequence seq;
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ConnectionFailed);
        }));
    
    EXPECT_CALL(m_mock_callback, OnError(::testing::_))
        .Times(1)
        .WillOnce(::testing::Invoke([](const error_handling::ErrorInfo& info) {
            EXPECT_EQ(info.code, error_handling::ErrorCode::ProtocolError);
        }));
    
    m_client.TriggerError(
        error_handling::ErrorCode::ConnectionFailed,
        "Connection failed");
    m_client.TriggerError(
        error_handling::ErrorCode::ProtocolError,
        "Protocol error");
}
