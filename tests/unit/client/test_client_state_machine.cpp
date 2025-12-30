/**
 * @file test_client_state_machine.cpp
 * @brief Unit tests for client connection state machine
 *
 * Tests validate:
 * - Initial state is Disconnected
 * - Legal state transitions
 * - State persistence after transition
 * - Multiple sequential transitions
 * - State getter accuracy
 *
 * State machine:
 * - Initial: Disconnected
 * - From Disconnected: can go to Connecting
 * - From Connecting: can go to Connected or back to Disconnected (on error)
 * - From Connected: can go to Disconnecting
 * - From Disconnecting: can go to Disconnected
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

/**
 * Client state machine utilities (header-only, non-production)
 */
namespace client_state_machine {

/**
 * Connection state enumeration
 */
enum class ConnectionState {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Disconnecting = 3
};

/**
 * Convert state to string for debugging
 * @param state State to convert
 * @return String representation
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
        default:
            return "Unknown";
    }
}

/**
 * Simple client state machine (non-production, for testing)
 */
class SimpleClientStateMachine {
private:
    ConnectionState m_state;
    int m_transition_count;
    
public:
    SimpleClientStateMachine() 
        : m_state(ConnectionState::Disconnected), m_transition_count(0) {
    }
    
    /**
     * Get current state
     */
    ConnectionState GetState() const {
        return m_state;
    }
    
    /**
     * Get number of transitions executed
     */
    int GetTransitionCount() const {
        return m_transition_count;
    }
    
    /**
     * Attempt transition to new state
     * @param new_state State to transition to
     * @return true if transition was valid, false otherwise
     */
    bool TransitionTo(ConnectionState new_state) {
        if (!IsValidTransition(m_state, new_state)) {
            return false;
        }
        m_state = new_state;
        m_transition_count++;
        return true;
    }
    
    /**
     * Reset state to initial (Disconnected)
     */
    void Reset() {
        m_state = ConnectionState::Disconnected;
        m_transition_count = 0;
    }
    
private:
    /**
     * Check if transition is valid
     */
    bool IsValidTransition(ConnectionState from, ConnectionState to) const {
        // Prevent transitioning to same state
        if (from == to) {
            return false;
        }
        
        // Define legal transitions
        switch (from) {
            case ConnectionState::Disconnected:
                return to == ConnectionState::Connecting;
            
            case ConnectionState::Connecting:
                return to == ConnectionState::Connected || 
                       to == ConnectionState::Disconnected;
            
            case ConnectionState::Connected:
                return to == ConnectionState::Disconnecting;
            
            case ConnectionState::Disconnecting:
                return to == ConnectionState::Disconnected;
            
            default:
                return false;
        }
    }
};

} // namespace client_state_machine

/**
 * Test fixture for client state machine
 */
class TestClientStateMachine : public ::testing::Test {
protected:
    client_state_machine::SimpleClientStateMachine m_client;
    
    void SetUp() override {
        m_client.Reset();
    }
};

// ============================================================================
// Initial State Tests
// ============================================================================

TEST_F(TestClientStateMachine, InitialStateDisconnected) {
    // Test: Initial state is Disconnected
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
}

TEST_F(TestClientStateMachine, InitialTransitionCountZero) {
    // Test: Initial transition count is zero
    EXPECT_EQ(m_client.GetTransitionCount(), 0);
}

// ============================================================================
// Valid Transition Tests
// ============================================================================

TEST_F(TestClientStateMachine, TransitionDisconnectedToConnecting) {
    // Test: Transition from Disconnected to Connecting (legal)
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connecting);
    EXPECT_EQ(m_client.GetTransitionCount(), 1);
}

TEST_F(TestClientStateMachine, TransitionConnectingToConnected) {
    // Test: Transition from Connecting to Connected (legal)
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Connected);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connected);
    EXPECT_EQ(m_client.GetTransitionCount(), 2);
}

TEST_F(TestClientStateMachine, TransitionConnectedToDisconnecting) {
    // Test: Transition from Connected to Disconnecting (legal)
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnecting);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnecting);
    EXPECT_EQ(m_client.GetTransitionCount(), 3);
}

TEST_F(TestClientStateMachine, TransitionDisconnectingToDisconnected) {
    // Test: Transition from Disconnecting to Disconnected (legal)
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    m_client.TransitionTo(client_state_machine::ConnectionState::Disconnecting);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
    EXPECT_EQ(m_client.GetTransitionCount(), 4);
}

TEST_F(TestClientStateMachine, TransitionConnectingBackToDisconnected) {
    // Test: Transition from Connecting back to Disconnected (error recovery)
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
}

// ============================================================================
// Invalid Transition Tests
// ============================================================================

TEST_F(TestClientStateMachine, InvalidTransitionDisconnectedToConnected) {
    // Test: Cannot skip Connecting state
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Connected);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
}

TEST_F(TestClientStateMachine, InvalidTransitionToSameState) {
    // Test: Cannot transition to same state
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
}

TEST_F(TestClientStateMachine, InvalidTransitionConnectedToConnecting) {
    // Test: Cannot go backwards from Connected to Connecting
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connected);
}

TEST_F(TestClientStateMachine, InvalidTransitionConnectedToDisconnected) {
    // Test: Cannot skip Disconnecting state
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connected);
}

TEST_F(TestClientStateMachine, InvalidTransitionDisconnectingToConnecting) {
    // Test: Cannot transition to Connecting from Disconnecting
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    m_client.TransitionTo(client_state_machine::ConnectionState::Disconnecting);
    
    bool success = m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting);
    
    EXPECT_FALSE(success);
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnecting);
}

// ============================================================================
// Normal Connection Flow Tests
// ============================================================================

TEST_F(TestClientStateMachine, NormalConnectionFlow) {
    // Test: Complete normal connection sequence
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting));
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connected));
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnecting));
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected));
    
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
    EXPECT_EQ(m_client.GetTransitionCount(), 4);
}

TEST_F(TestClientStateMachine, NormalDisconnectionFlow) {
    // Test: Disconnection from connected state
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnecting));
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected));
    
    EXPECT_EQ(m_client.GetTransitionCount(), 4);
}

// ============================================================================
// Error Recovery Tests
// ============================================================================

TEST_F(TestClientStateMachine, ErrorDuringConnecting) {
    // Test: Connection error during Connecting state
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting));
    
    // Error: go back to Disconnected without reaching Connected
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Disconnected));
    
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
}

TEST_F(TestClientStateMachine, ReconnectAfterError) {
    // Test: Reconnect after connection error
    // First attempt: error during connecting
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Disconnected);
    
    // Second attempt: successful connection
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting));
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connected));
    
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connected);
    EXPECT_EQ(m_client.GetTransitionCount(), 4);
}

// ============================================================================
// State Persistence Tests
// ============================================================================

TEST_F(TestClientStateMachine, StatePersistsAfterTransition) {
    // Test: State remains unchanged after transition
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    auto state1 = m_client.GetState();
    auto state2 = m_client.GetState();
    auto state3 = m_client.GetState();
    
    EXPECT_EQ(state1, state2);
    EXPECT_EQ(state2, state3);
    EXPECT_EQ(state1, client_state_machine::ConnectionState::Connecting);
}

TEST_F(TestClientStateMachine, MultipleGettersConsistent) {
    // Test: Multiple calls to GetState return same value
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    auto state = m_client.GetState();
    EXPECT_EQ(state, client_state_machine::ConnectionState::Connected);
    EXPECT_EQ(state, m_client.GetState());
    EXPECT_EQ(state, m_client.GetState());
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_F(TestClientStateMachine, ResetToInitialState) {
    // Test: Reset returns to initial state
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    
    m_client.Reset();
    
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Disconnected);
    EXPECT_EQ(m_client.GetTransitionCount(), 0);
}

TEST_F(TestClientStateMachine, ReconnectAfterReset) {
    // Test: Can reconnect after reset
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    m_client.Reset();
    
    EXPECT_TRUE(m_client.TransitionTo(
        client_state_machine::ConnectionState::Connecting));
    
    EXPECT_EQ(m_client.GetState(), 
              client_state_machine::ConnectionState::Connecting);
    EXPECT_EQ(m_client.GetTransitionCount(), 1);
}

// ============================================================================
// Transition Counter Tests
// ============================================================================

TEST_F(TestClientStateMachine, TransitionCountIncrementsOnValidTransition) {
    // Test: Counter increments only on valid transitions
    EXPECT_EQ(m_client.GetTransitionCount(), 0);
    
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    EXPECT_EQ(m_client.GetTransitionCount(), 1);
    
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);
    EXPECT_EQ(m_client.GetTransitionCount(), 2);
}

TEST_F(TestClientStateMachine, TransitionCountIgnoresInvalidTransitions) {
    // Test: Counter does not increment on invalid transitions
    m_client.TransitionTo(client_state_machine::ConnectionState::Connecting);
    auto count_before = m_client.GetTransitionCount();
    
    // Attempt invalid transition
    m_client.TransitionTo(client_state_machine::ConnectionState::Disconnected);
    m_client.TransitionTo(client_state_machine::ConnectionState::Connected);  // Invalid
    
    auto count_after = m_client.GetTransitionCount();
    
    // Count should have increased by 1 (only for the valid transition)
    EXPECT_EQ(count_after, count_before + 1);
}

// ============================================================================
// State Conversion Tests
// ============================================================================

TEST_F(TestClientStateMachine, StateToStringDisconnected) {
    // Test: Convert Disconnected state to string
    auto str = client_state_machine::StateToString(
        client_state_machine::ConnectionState::Disconnected);
    EXPECT_EQ(str, "Disconnected");
}

TEST_F(TestClientStateMachine, StateToStringConnecting) {
    // Test: Convert Connecting state to string
    auto str = client_state_machine::StateToString(
        client_state_machine::ConnectionState::Connecting);
    EXPECT_EQ(str, "Connecting");
}

TEST_F(TestClientStateMachine, StateToStringConnected) {
    // Test: Convert Connected state to string
    auto str = client_state_machine::StateToString(
        client_state_machine::ConnectionState::Connected);
    EXPECT_EQ(str, "Connected");
}

TEST_F(TestClientStateMachine, StateToStringDisconnecting) {
    // Test: Convert Disconnecting state to string
    auto str = client_state_machine::StateToString(
        client_state_machine::ConnectionState::Disconnecting);
    EXPECT_EQ(str, "Disconnecting");
}
