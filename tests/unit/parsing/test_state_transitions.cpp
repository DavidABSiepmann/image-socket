/**
 * @file test_state_transitions.cpp
 * @brief Unit tests for state transition validation
 *
 * Tests validate:
 * - Legal state transitions
 * - Illegal state transitions (error cases)
 * - State enum values and ranges
 * - Deterministic state behavior
 * - CommandType to string conversions
 *
 * State model:
 * - Connection states: Disconnected, Connecting, Connected, Disconnecting
 * - Command types: UNKNOWN, SET_FPS, SET_QUALITY, PAUSE, RESUME, GET_STATE, etc.
 */

#include <gtest/gtest.h>
#include "control.pb.h"
#include <cstdint>
#include <string>
#include <vector>

/**
 * State transition utilities (header-only, non-production code)
 */
namespace state_transitions {

// Connection states
enum class ConnectionState {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Disconnecting = 3,
    Error = 4
};

/**
 * Check if a state transition is legal
 * @param from Source state
 * @param to Destination state
 * @return true if transition is allowed
 */
inline bool IsLegalTransition(ConnectionState from, ConnectionState to) {
    // Define legal transitions
    switch (from) {
        case ConnectionState::Disconnected:
            // From Disconnected, can go to Connecting or Error
            return to == ConnectionState::Connecting || to == ConnectionState::Error;
        
        case ConnectionState::Connecting:
            // From Connecting, can go to Connected or Error
            return to == ConnectionState::Connected || to == ConnectionState::Error;
        
        case ConnectionState::Connected:
            // From Connected, can go to Disconnecting or Error
            return to == ConnectionState::Disconnecting || to == ConnectionState::Error;
        
        case ConnectionState::Disconnecting:
            // From Disconnecting, can go to Disconnected or Error
            return to == ConnectionState::Disconnected || to == ConnectionState::Error;
        
        case ConnectionState::Error:
            // From Error, can only go to Disconnected
            return to == ConnectionState::Disconnected;
        
        default:
            return false;
    }
}

/**
 * Convert ConnectionState enum to string
 * @param state State to convert
 * @return String representation of state
 */
inline std::string StateToString(ConnectionState state) {
    switch (state) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connecting:
            return "Connecting";
        case ConnectionState::Connected:
            return "Connected";
        case ConnectionState::Disconnecting:
            return "Disconnecting";
        case ConnectionState::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

/**
 * Validate CommandType enum value
 * @param type CommandType value to validate
 * @return true if type is a valid command type
 */
inline bool IsValidCommandType(imagesocket::control::CommandType type) {
    switch (type) {
        case imagesocket::control::UNKNOWN:
        case imagesocket::control::PAUSE:
        case imagesocket::control::RESUME:
        case imagesocket::control::ID:
        case imagesocket::control::REQUEST_RESUME:
        case imagesocket::control::SET_FPS:
        case imagesocket::control::SET_QUALITY:
        case imagesocket::control::SUBSCRIBE:
        case imagesocket::control::UNSUBSCRIBE:
        case imagesocket::control::REQUEST_ALIAS:
        case imagesocket::control::ALIAS:
            return true;
        default:
            return false;
    }
}

/**
 * Convert CommandType enum to string
 * @param type CommandType to convert
 * @return String representation of command
 */
inline std::string CommandTypeToString(imagesocket::control::CommandType type) {
    switch (type) {
        case imagesocket::control::UNKNOWN:
            return "UNKNOWN";
        case imagesocket::control::PAUSE:
            return "PAUSE";
        case imagesocket::control::RESUME:
            return "RESUME";
        case imagesocket::control::ID:
            return "ID";
        case imagesocket::control::REQUEST_RESUME:
            return "REQUEST_RESUME";
        case imagesocket::control::SET_FPS:
            return "SET_FPS";
        case imagesocket::control::SET_QUALITY:
            return "SET_QUALITY";
        case imagesocket::control::SUBSCRIBE:
            return "SUBSCRIBE";
        case imagesocket::control::UNSUBSCRIBE:
            return "UNSUBSCRIBE";
        case imagesocket::control::REQUEST_ALIAS:
            return "REQUEST_ALIAS";
        case imagesocket::control::ALIAS:
            return "ALIAS";
        default:
            return "UNKNOWN";
    }
}

} // namespace state_transitions

/**
 * Test fixture for state transitions
 */
class TestStateTransitions : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }
};

// ============================================================================
// Legal State Transitions Tests
// ============================================================================

TEST_F(TestStateTransitions, TransitionDisconnectedToConnecting) {
    // Test: Legal transition from Disconnected to Connecting
    auto from = state_transitions::ConnectionState::Disconnected;
    auto to = state_transitions::ConnectionState::Connecting;
    EXPECT_TRUE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionConnectingToConnected) {
    // Test: Legal transition from Connecting to Connected
    auto from = state_transitions::ConnectionState::Connecting;
    auto to = state_transitions::ConnectionState::Connected;
    EXPECT_TRUE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionConnectedToDisconnecting) {
    // Test: Legal transition from Connected to Disconnecting
    auto from = state_transitions::ConnectionState::Connected;
    auto to = state_transitions::ConnectionState::Disconnecting;
    EXPECT_TRUE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionDisconnectingToDisconnected) {
    // Test: Legal transition from Disconnecting to Disconnected
    auto from = state_transitions::ConnectionState::Disconnecting;
    auto to = state_transitions::ConnectionState::Disconnected;
    EXPECT_TRUE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionAnyToError) {
    // Test: Legal transitions to Error state
    std::vector<state_transitions::ConnectionState> states = {
        state_transitions::ConnectionState::Disconnected,
        state_transitions::ConnectionState::Connecting,
        state_transitions::ConnectionState::Connected,
        state_transitions::ConnectionState::Disconnecting
    };
    
    auto error = state_transitions::ConnectionState::Error;
    for (auto state : states) {
        EXPECT_TRUE(state_transitions::IsLegalTransition(state, error))
            << "Transition from " << state_transitions::StateToString(state) 
            << " to Error should be legal";
    }
}

TEST_F(TestStateTransitions, TransitionErrorToDisconnected) {
    // Test: Legal transition from Error to Disconnected
    auto from = state_transitions::ConnectionState::Error;
    auto to = state_transitions::ConnectionState::Disconnected;
    EXPECT_TRUE(state_transitions::IsLegalTransition(from, to));
}

// ============================================================================
// Illegal State Transitions Tests
// ============================================================================

TEST_F(TestStateTransitions, TransitionDisconnectedToConnected) {
    // Test: Illegal transition (skip Connecting state)
    auto from = state_transitions::ConnectionState::Disconnected;
    auto to = state_transitions::ConnectionState::Connected;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionConnectingToDisconnecting) {
    // Test: Illegal transition (skip Connected state)
    auto from = state_transitions::ConnectionState::Connecting;
    auto to = state_transitions::ConnectionState::Disconnecting;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionConnectedToConnecting) {
    // Test: Illegal backward transition
    auto from = state_transitions::ConnectionState::Connected;
    auto to = state_transitions::ConnectionState::Connecting;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionDisconnectingToConnecting) {
    // Test: Illegal transition (wrong direction)
    auto from = state_transitions::ConnectionState::Disconnecting;
    auto to = state_transitions::ConnectionState::Connecting;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionErrorToError) {
    // Test: Illegal transition to same error state
    auto from = state_transitions::ConnectionState::Error;
    auto to = state_transitions::ConnectionState::Error;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionErrorToConnecting) {
    // Test: Illegal transition from Error (can only go to Disconnected)
    auto from = state_transitions::ConnectionState::Error;
    auto to = state_transitions::ConnectionState::Connecting;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

TEST_F(TestStateTransitions, TransitionConnectedToDisconnected) {
    // Test: Illegal transition (must go through Disconnecting)
    auto from = state_transitions::ConnectionState::Connected;
    auto to = state_transitions::ConnectionState::Disconnected;
    EXPECT_FALSE(state_transitions::IsLegalTransition(from, to));
}

// ============================================================================
// State String Conversion Tests
// ============================================================================

TEST_F(TestStateTransitions, StateToStringDisconnected) {
    // Test: Convert Disconnected state to string
    auto str = state_transitions::StateToString(
        state_transitions::ConnectionState::Disconnected);
    EXPECT_EQ(str, "Disconnected");
}

TEST_F(TestStateTransitions, StateToStringConnecting) {
    // Test: Convert Connecting state to string
    auto str = state_transitions::StateToString(
        state_transitions::ConnectionState::Connecting);
    EXPECT_EQ(str, "Connecting");
}

TEST_F(TestStateTransitions, StateToStringConnected) {
    // Test: Convert Connected state to string
    auto str = state_transitions::StateToString(
        state_transitions::ConnectionState::Connected);
    EXPECT_EQ(str, "Connected");
}

TEST_F(TestStateTransitions, StateToStringDisconnecting) {
    // Test: Convert Disconnecting state to string
    auto str = state_transitions::StateToString(
        state_transitions::ConnectionState::Disconnecting);
    EXPECT_EQ(str, "Disconnecting");
}

TEST_F(TestStateTransitions, StateToStringError) {
    // Test: Convert Error state to string
    auto str = state_transitions::StateToString(
        state_transitions::ConnectionState::Error);
    EXPECT_EQ(str, "Error");
}

// ============================================================================
// CommandType Enum Validation Tests
// ============================================================================

TEST_F(TestStateTransitions, CommandTypeUNKNOWN) {
    // Test: UNKNOWN command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::UNKNOWN));
}

TEST_F(TestStateTransitions, CommandTypeSET_FPS) {
    // Test: SET_FPS command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::SET_FPS));
}

TEST_F(TestStateTransitions, CommandTypeSET_QUALITY) {
    // Test: SET_QUALITY command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::SET_QUALITY));
}

TEST_F(TestStateTransitions, CommandTypePAUSE) {
    // Test: PAUSE command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::PAUSE));
}

TEST_F(TestStateTransitions, CommandTypeRESUME) {
    // Test: RESUME command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::RESUME));
}

TEST_F(TestStateTransitions, CommandTypeID) {
    // Test: ID command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::ID));
}

TEST_F(TestStateTransitions, CommandTypeREQUEST_RESUME) {
    // Test: REQUEST_RESUME command type is valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(
        imagesocket::control::REQUEST_RESUME));
}

TEST_F(TestStateTransitions, CommandTypeInvalid) {
    // Test: Invalid command type (out of enum range)
    EXPECT_FALSE(state_transitions::IsValidCommandType(
        static_cast<imagesocket::control::CommandType>(999)));
}

// ============================================================================
// CommandType String Conversion Tests
// ============================================================================

TEST_F(TestStateTransitions, CommandTypeToStringUNKNOWN) {
    // Test: Convert UNKNOWN to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::UNKNOWN);
    EXPECT_EQ(str, "UNKNOWN");
}

TEST_F(TestStateTransitions, CommandTypeToStringSET_FPS) {
    // Test: Convert SET_FPS to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::SET_FPS);
    EXPECT_EQ(str, "SET_FPS");
}

TEST_F(TestStateTransitions, CommandTypeToStringSET_QUALITY) {
    // Test: Convert SET_QUALITY to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::SET_QUALITY);
    EXPECT_EQ(str, "SET_QUALITY");
}

TEST_F(TestStateTransitions, CommandTypeToStringPAUSE) {
    // Test: Convert PAUSE to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::PAUSE);
    EXPECT_EQ(str, "PAUSE");
}

TEST_F(TestStateTransitions, CommandTypeToStringRESUME) {
    // Test: Convert RESUME to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::RESUME);
    EXPECT_EQ(str, "RESUME");
}

TEST_F(TestStateTransitions, CommandTypeToStringALIAS) {
    // Test: Convert ALIAS to string
    auto str = state_transitions::CommandTypeToString(
        imagesocket::control::ALIAS);
    EXPECT_EQ(str, "ALIAS");
}

// ============================================================================
// Complete State Sequence Tests
// ============================================================================

TEST_F(TestStateTransitions, NormalConnectionFlow) {
    // Test: Complete normal connection flow
    std::vector<state_transitions::ConnectionState> expected_sequence = {
        state_transitions::ConnectionState::Disconnected,
        state_transitions::ConnectionState::Connecting,
        state_transitions::ConnectionState::Connected
    };
    
    for (size_t i = 0; i < expected_sequence.size() - 1; i++) {
        EXPECT_TRUE(state_transitions::IsLegalTransition(
            expected_sequence[i], 
            expected_sequence[i + 1]
        )) << "Transition from " << state_transitions::StateToString(expected_sequence[i])
           << " to " << state_transitions::StateToString(expected_sequence[i + 1])
           << " should be legal";
    }
}

TEST_F(TestStateTransitions, NormalDisconnectionFlow) {
    // Test: Complete normal disconnection flow
    std::vector<state_transitions::ConnectionState> expected_sequence = {
        state_transitions::ConnectionState::Connected,
        state_transitions::ConnectionState::Disconnecting,
        state_transitions::ConnectionState::Disconnected
    };
    
    for (size_t i = 0; i < expected_sequence.size() - 1; i++) {
        EXPECT_TRUE(state_transitions::IsLegalTransition(
            expected_sequence[i], 
            expected_sequence[i + 1]
        )) << "Transition should be legal in normal disconnection flow";
    }
}

TEST_F(TestStateTransitions, ErrorRecoveryFlow) {
    // Test: Error state recovery flow
    std::vector<state_transitions::ConnectionState> expected_sequence = {
        state_transitions::ConnectionState::Connected,
        state_transitions::ConnectionState::Error,
        state_transitions::ConnectionState::Disconnected,
        state_transitions::ConnectionState::Connecting,
        state_transitions::ConnectionState::Connected
    };
    
    for (size_t i = 0; i < expected_sequence.size() - 1; i++) {
        EXPECT_TRUE(state_transitions::IsLegalTransition(
            expected_sequence[i], 
            expected_sequence[i + 1]
        )) << "Transition in error recovery should be legal";
    }
}

// ============================================================================
// All State Pairs Validation Tests
// ============================================================================

TEST_F(TestStateTransitions, AllStateTransitionsExplicit) {
    // Test: Verify all state pairs have deterministic results
    std::vector<state_transitions::ConnectionState> states = {
        state_transitions::ConnectionState::Disconnected,
        state_transitions::ConnectionState::Connecting,
        state_transitions::ConnectionState::Connected,
        state_transitions::ConnectionState::Disconnecting,
        state_transitions::ConnectionState::Error
    };
    
    // For each pair, verify that calling IsLegalTransition multiple times
    // produces the same result (determinism)
    for (auto from : states) {
        for (auto to : states) {
            bool legal1 = state_transitions::IsLegalTransition(from, to);
            bool legal2 = state_transitions::IsLegalTransition(from, to);
            EXPECT_EQ(legal1, legal2) 
                << "Non-deterministic result for " 
                << state_transitions::StateToString(from) 
                << " -> " 
                << state_transitions::StateToString(to);
        }
    }
}

// ============================================================================
// Enum Range Tests
// ============================================================================

TEST_F(TestStateTransitions, AllCommandTypesValid) {
    // Test: All protobuf CommandType enum values are valid
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::UNKNOWN));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::PAUSE));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::RESUME));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::ID));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::REQUEST_RESUME));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::SET_FPS));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::SET_QUALITY));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::SUBSCRIBE));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::UNSUBSCRIBE));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::REQUEST_ALIAS));
    EXPECT_TRUE(state_transitions::IsValidCommandType(imagesocket::control::ALIAS));
}

TEST_F(TestStateTransitions, CommandTypeStringDeterministic) {
    // Test: Command type to string conversion is deterministic
    auto type = imagesocket::control::SET_FPS;
    std::string str1 = state_transitions::CommandTypeToString(type);
    std::string str2 = state_transitions::CommandTypeToString(type);
    std::string str3 = state_transitions::CommandTypeToString(type);
    
    EXPECT_EQ(str1, str2);
    EXPECT_EQ(str2, str3);
}

TEST_F(TestStateTransitions, StateStringDeterministic) {
    // Test: State to string conversion is deterministic
    auto state = state_transitions::ConnectionState::Connected;
    std::string str1 = state_transitions::StateToString(state);
    std::string str2 = state_transitions::StateToString(state);
    std::string str3 = state_transitions::StateToString(state);
    
    EXPECT_EQ(str1, str2);
    EXPECT_EQ(str2, str3);
}
