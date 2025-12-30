/**
 * @file test_reconnect_backoff.cpp
 * @brief Unit tests for reconnection backoff calculation
 *
 * Tests validate:
 * - Deterministic backoff calculation (exponential growth)
 * - Backoff sequence without real delays
 * - Maximum backoff cap (30 seconds)
 * - Attempt counter progression
 *
 * Backoff formula (from websocketimageclient.cpp):
 * waitSeconds = min(30, 1 << min(attempt, 6))
 * 
 * Sequence:
 * Attempt 0: 2^0 = 1 second
 * Attempt 1: 2^1 = 2 seconds
 * Attempt 2: 2^2 = 4 seconds
 * Attempt 3: 2^3 = 8 seconds
 * Attempt 4: 2^4 = 16 seconds
 * Attempt 5: 2^5 = 32 -> min(30, 32) = 30 seconds
 * Attempt 6+: 2^6 = 64 -> min(30, 64) = 30 seconds (capped)
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

/**
 * Reconnection backoff utilities (header-only, non-production)
 * Mirrors the logic from websocketimageclient.cpp without threading
 */
namespace reconnect_logic {

// Maximum backoff cap (seconds)
constexpr int MAX_BACKOFF_SECONDS = 30;

// Maximum bit shift before hitting the cap
constexpr int MAX_BIT_SHIFT = 6;

/**
 * Calculate backoff delay for a given reconnection attempt
 * @param attempt Attempt number (0-based)
 * @return Backoff delay in seconds
 * 
 * Formula: min(30, 1 << min(attempt, 6))
 */
inline int CalculateBackoffSeconds(int attempt) {
    int shift = std::min(attempt, MAX_BIT_SHIFT);
    int raw_value = 1 << shift;
    return std::min(MAX_BACKOFF_SECONDS, raw_value);
}

/**
 * Generate the backoff sequence for N attempts
 * @param num_attempts Number of attempts to calculate
 * @return Vector of backoff delays in seconds
 */
inline std::vector<int> GenerateBackoffSequence(int num_attempts) {
    std::vector<int> sequence;
    for (int i = 0; i < num_attempts; ++i) {
        sequence.push_back(CalculateBackoffSeconds(i));
    }
    return sequence;
}

/**
 * Check if backoff sequence is monotonically non-decreasing
 * @param sequence Backoff sequence to check
 * @return true if sequence is non-decreasing
 */
inline bool IsMonotonicNonDecreasing(const std::vector<int>& sequence) {
    for (size_t i = 1; i < sequence.size(); ++i) {
        if (sequence[i] < sequence[i - 1]) {
            return false;
        }
    }
    return true;
}

} // namespace reconnect_logic

/**
 * Test fixture for reconnection backoff logic
 */
class TestReconnectBackoff : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }
};

// ============================================================================
// Individual Backoff Calculation Tests
// ============================================================================

TEST_F(TestReconnectBackoff, BackoffAttempt0) {
    // Test: First attempt (2^0 = 1 second)
    int backoff = reconnect_logic::CalculateBackoffSeconds(0);
    EXPECT_EQ(backoff, 1);
}

TEST_F(TestReconnectBackoff, BackoffAttempt1) {
    // Test: Second attempt (2^1 = 2 seconds)
    int backoff = reconnect_logic::CalculateBackoffSeconds(1);
    EXPECT_EQ(backoff, 2);
}

TEST_F(TestReconnectBackoff, BackoffAttempt2) {
    // Test: Third attempt (2^2 = 4 seconds)
    int backoff = reconnect_logic::CalculateBackoffSeconds(2);
    EXPECT_EQ(backoff, 4);
}

TEST_F(TestReconnectBackoff, BackoffAttempt3) {
    // Test: Fourth attempt (2^3 = 8 seconds)
    int backoff = reconnect_logic::CalculateBackoffSeconds(3);
    EXPECT_EQ(backoff, 8);
}

TEST_F(TestReconnectBackoff, BackoffAttempt4) {
    // Test: Fifth attempt (2^4 = 16 seconds)
    int backoff = reconnect_logic::CalculateBackoffSeconds(4);
    EXPECT_EQ(backoff, 16);
}

TEST_F(TestReconnectBackoff, BackoffAttempt5) {
    // Test: Sixth attempt (2^5 = 32 -> capped at 30)
    int backoff = reconnect_logic::CalculateBackoffSeconds(5);
    EXPECT_EQ(backoff, 30);
}

TEST_F(TestReconnectBackoff, BackoffAttempt6) {
    // Test: Seventh attempt (2^6 = 64 -> capped at 30)
    int backoff = reconnect_logic::CalculateBackoffSeconds(6);
    EXPECT_EQ(backoff, 30);
}

TEST_F(TestReconnectBackoff, BackoffAttemptLarge) {
    // Test: Large attempt number (capped at 30)
    int backoff = reconnect_logic::CalculateBackoffSeconds(100);
    EXPECT_EQ(backoff, 30);
}

// ============================================================================
// Backoff Sequence Tests
// ============================================================================

TEST_F(TestReconnectBackoff, BackoffSequence10Attempts) {
    // Test: Generate backoff sequence for 10 attempts
    auto sequence = reconnect_logic::GenerateBackoffSequence(10);
    
    // Verify correct length
    ASSERT_EQ(sequence.size(), 10);
    
    // Verify exact values for first 6 attempts
    EXPECT_EQ(sequence[0], 1);   // 2^0
    EXPECT_EQ(sequence[1], 2);   // 2^1
    EXPECT_EQ(sequence[2], 4);   // 2^2
    EXPECT_EQ(sequence[3], 8);   // 2^3
    EXPECT_EQ(sequence[4], 16);  // 2^4
    EXPECT_EQ(sequence[5], 30);  // min(30, 2^5)
    
    // Verify capped at 30 for remaining attempts
    for (size_t i = 6; i < sequence.size(); ++i) {
        EXPECT_EQ(sequence[i], 30)
            << "Attempt " << i << " should be capped at 30";
    }
}

TEST_F(TestReconnectBackoff, BackoffSequenceMonotonicNonDecreasing) {
    // Test: Backoff sequence is monotonically non-decreasing
    auto sequence = reconnect_logic::GenerateBackoffSequence(20);
    EXPECT_TRUE(reconnect_logic::IsMonotonicNonDecreasing(sequence))
        << "Backoff sequence should be non-decreasing";
}

TEST_F(TestReconnectBackoff, BackoffSequenceExactValues) {
    // Test: Verify exact backoff sequence
    std::vector<int> expected = {
        1,    // Attempt 0: 2^0
        2,    // Attempt 1: 2^1
        4,    // Attempt 2: 2^2
        8,    // Attempt 3: 2^3
        16,   // Attempt 4: 2^4
        30,   // Attempt 5: min(30, 32)
        30,   // Attempt 6: min(30, 64) - capped
        30,   // Attempt 7: min(30, 128) - capped
        30,   // Attempt 8: min(30, 256) - capped
    };
    
    auto sequence = reconnect_logic::GenerateBackoffSequence(expected.size());
    EXPECT_EQ(sequence, expected);
}

// ============================================================================
// Backoff Cap Tests
// ============================================================================

TEST_F(TestReconnectBackoff, MaxBackoffRespected) {
    // Test: No backoff exceeds maximum
    auto sequence = reconnect_logic::GenerateBackoffSequence(50);
    
    for (size_t i = 0; i < sequence.size(); ++i) {
        EXPECT_LE(sequence[i], reconnect_logic::MAX_BACKOFF_SECONDS)
            << "Backoff at attempt " << i << " exceeds maximum";
    }
}

TEST_F(TestReconnectBackoff, CapEnforcedAt5thAttempt) {
    // Test: Cap is first applied at attempt 5
    int backoff_4 = reconnect_logic::CalculateBackoffSeconds(4);
    int backoff_5 = reconnect_logic::CalculateBackoffSeconds(5);
    int backoff_6 = reconnect_logic::CalculateBackoffSeconds(6);
    
    // Attempt 4 is under cap
    EXPECT_LT(backoff_4, reconnect_logic::MAX_BACKOFF_SECONDS);
    
    // Attempt 5 hits cap
    EXPECT_EQ(backoff_5, reconnect_logic::MAX_BACKOFF_SECONDS);
    
    // Attempt 6 stays at cap
    EXPECT_EQ(backoff_6, reconnect_logic::MAX_BACKOFF_SECONDS);
}

// ============================================================================
// Determinism Tests
// ============================================================================

TEST_F(TestReconnectBackoff, BackoffDeterministic) {
    // Test: Same attempt always produces same backoff
    for (int attempt = 0; attempt < 20; ++attempt) {
        int result1 = reconnect_logic::CalculateBackoffSeconds(attempt);
        int result2 = reconnect_logic::CalculateBackoffSeconds(attempt);
        int result3 = reconnect_logic::CalculateBackoffSeconds(attempt);
        
        EXPECT_EQ(result1, result2)
            << "Non-deterministic backoff at attempt " << attempt;
        EXPECT_EQ(result2, result3)
            << "Non-deterministic backoff at attempt " << attempt;
    }
}

TEST_F(TestReconnectBackoff, SequenceDeterministic) {
    // Test: Same sequence length always produces same values
    auto seq1 = reconnect_logic::GenerateBackoffSequence(15);
    auto seq2 = reconnect_logic::GenerateBackoffSequence(15);
    auto seq3 = reconnect_logic::GenerateBackoffSequence(15);
    
    EXPECT_EQ(seq1, seq2);
    EXPECT_EQ(seq2, seq3);
}

// ============================================================================
// Practical Reconnect Scenario Tests
// ============================================================================

TEST_F(TestReconnectBackoff, QuickFailureFastRetry) {
    // Test: Quick failures have short initial backoff
    int backoff_0 = reconnect_logic::CalculateBackoffSeconds(0);
    EXPECT_LT(backoff_0, 2);
}

TEST_F(TestReconnectBackoff, SlowRecoveryLongBackoff) {
    // Test: After several failures, backoff reaches reasonable minimum
    int backoff_4 = reconnect_logic::CalculateBackoffSeconds(4);
    EXPECT_GE(backoff_4, 10);
}

TEST_F(TestReconnectBackoff, ExtendedOutageMaxBackoff) {
    // Test: Very frequent failure attempts eventually stop thrashing
    int backoff_10 = reconnect_logic::CalculateBackoffSeconds(10);
    EXPECT_EQ(backoff_10, reconnect_logic::MAX_BACKOFF_SECONDS);
}

TEST_F(TestReconnectBackoff, TotalWaitTimeFirstFiveAttempts) {
    // Test: Total wait time for first 5 attempts is reasonable
    // Sum: 1 + 2 + 4 + 8 + 16 = 31 seconds total
    auto sequence = reconnect_logic::GenerateBackoffSequence(5);
    
    int total = 0;
    for (int backoff : sequence) {
        total += backoff;
    }
    
    EXPECT_EQ(total, 31);
}

TEST_F(TestReconnectBackoff, TotalWaitTimeFirstTenAttempts) {
    // Test: Total wait time for first 10 attempts
    // Sum: 1 + 2 + 4 + 8 + 16 + 30*5 = 31 + 150 = 181 seconds (~3 minutes)
    auto sequence = reconnect_logic::GenerateBackoffSequence(10);
    
    int total = 0;
    for (int backoff : sequence) {
        total += backoff;
    }
    
    EXPECT_EQ(total, 181);  // 1+2+4+8+16+30*5
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(TestReconnectBackoff, ZeroAttempt) {
    // Test: Attempt 0 returns valid backoff
    int backoff = reconnect_logic::CalculateBackoffSeconds(0);
    EXPECT_GT(backoff, 0);
    EXPECT_LE(backoff, reconnect_logic::MAX_BACKOFF_SECONDS);
}

TEST_F(TestReconnectBackoff, NegativeAttempt) {
    // Test: Negative attempt (edge case, undefined behavior in bit shift)
    // The formula with negative values will likely overflow or produce garbage
    // We treat negative attempts as invalid input - should be clamped/caught upstream
    // Just verify the function doesn't crash
    int backoff = reconnect_logic::CalculateBackoffSeconds(-1);
    // For negative attempts, the function may return unpredictable values due to bit shift
    // This is expected as negative attempts are invalid input
    (void)backoff;  // Mark as used to avoid unused variable warning
}

TEST_F(TestReconnectBackoff, VeryLargeAttempt) {
    // Test: Very large attempt number (capped)
    int backoff_1000 = reconnect_logic::CalculateBackoffSeconds(1000);
    int backoff_10000 = reconnect_logic::CalculateBackoffSeconds(10000);
    
    EXPECT_EQ(backoff_1000, reconnect_logic::MAX_BACKOFF_SECONDS);
    EXPECT_EQ(backoff_10000, reconnect_logic::MAX_BACKOFF_SECONDS);
}

// ============================================================================
// Exponential Growth Tests
// ============================================================================

TEST_F(TestReconnectBackoff, ExponentialGrowthInitial) {
    // Test: Early backoff doubles with each attempt
    for (int attempt = 0; attempt < 4; ++attempt) {
        int current = reconnect_logic::CalculateBackoffSeconds(attempt);
        int next = reconnect_logic::CalculateBackoffSeconds(attempt + 1);
        
        EXPECT_EQ(next, current * 2)
            << "Backoff should double from attempt " << attempt 
            << " to " << (attempt + 1);
    }
}

TEST_F(TestReconnectBackoff, DoubleAttempts0To4) {
    // Test: First 5 attempts follow perfect doubling
    std::vector<int> expected = {1, 2, 4, 8, 16};
    
    for (size_t i = 0; i < expected.size(); ++i) {
        int actual = reconnect_logic::CalculateBackoffSeconds(i);
        EXPECT_EQ(actual, expected[i])
            << "Attempt " << i << " should be " << expected[i];
    }
}
