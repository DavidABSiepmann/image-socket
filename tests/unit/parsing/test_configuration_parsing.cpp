/**
 * @file test_configuration_parsing.cpp
 * @brief Unit tests for configuration parsing logic
 *
 * Tests validate parsing and validation of configuration values:
 * - FPS (frames per second) validation
 * - Quality level validation
 * - Client ID validation
 * - Various boundary and edge cases
 *
 * Test structure:
 * - Table-driven tests for multiple configurations
 * - Boundary value testing
 * - Invalid input handling
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>

/**
 * Configuration parsing utilities (header-only, non-production)
 * These functions mirror the logic that would be in production parsing code
 */
namespace config_parsing {

// Configuration constants
constexpr int MIN_FPS = 1;
constexpr int MAX_FPS = 120;
constexpr int MIN_QUALITY = 0;
constexpr int MAX_QUALITY = 100;
constexpr uint32_t MIN_CLIENT_ID = 1;
constexpr uint32_t MAX_CLIENT_ID = 1000000;

/**
 * Validate FPS value
 * @param fps Value to validate
 * @return true if FPS is within valid range
 */
inline bool IsValidFps(int fps) {
    return fps >= MIN_FPS && fps <= MAX_FPS;
}

/**
 * Validate quality level
 * @param quality Value to validate (0-100)
 * @return true if quality is within valid range
 */
inline bool IsValidQuality(int quality) {
    return quality >= MIN_QUALITY && quality <= MAX_QUALITY;
}

/**
 * Validate client ID
 * @param client_id Value to validate
 * @return true if client_id is within valid range
 */
inline bool IsValidClientId(uint32_t client_id) {
    return client_id >= MIN_CLIENT_ID && client_id <= MAX_CLIENT_ID;
}

/**
 * Clamp FPS to valid range
 * @param fps Value to clamp
 * @return Clamped FPS value
 */
inline int ClampFps(int fps) {
    return std::max(MIN_FPS, std::min(fps, MAX_FPS));
}

/**
 * Clamp quality to valid range
 * @param quality Value to clamp
 * @return Clamped quality value
 */
inline int ClampQuality(int quality) {
    return std::max(MIN_QUALITY, std::min(quality, MAX_QUALITY));
}

} // namespace config_parsing

/**
 * Test fixture for configuration parsing
 */
class TestConfigurationParsing : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }

    void TearDown() override {
        // Per-test cleanup
    }
};

// ============================================================================
// FPS Validation Tests
// ============================================================================

TEST_F(TestConfigurationParsing, FpsMinimalBoundary) {
    // Test: Minimum valid FPS (1)
    EXPECT_TRUE(config_parsing::IsValidFps(1));
}

TEST_F(TestConfigurationParsing, FpsMaximalBoundary) {
    // Test: Maximum valid FPS (120)
    EXPECT_TRUE(config_parsing::IsValidFps(120));
}

TEST_F(TestConfigurationParsing, FpsBelow_Min) {
    // Test: FPS below minimum (0)
    EXPECT_FALSE(config_parsing::IsValidFps(0));
}

TEST_F(TestConfigurationParsing, FpsAbove_Max) {
    // Test: FPS above maximum (121)
    EXPECT_FALSE(config_parsing::IsValidFps(121));
}

TEST_F(TestConfigurationParsing, FpsTypicalValue) {
    // Test: Typical FPS values
    EXPECT_TRUE(config_parsing::IsValidFps(30));
    EXPECT_TRUE(config_parsing::IsValidFps(60));
    EXPECT_TRUE(config_parsing::IsValidFps(90));
}

TEST_F(TestConfigurationParsing, FpsNegativeValue) {
    // Test: Negative FPS values
    EXPECT_FALSE(config_parsing::IsValidFps(-1));
    EXPECT_FALSE(config_parsing::IsValidFps(-100));
}

TEST_F(TestConfigurationParsing, FpsClampBelowMin) {
    // Test: Clamping FPS below minimum
    EXPECT_EQ(config_parsing::ClampFps(0), 1);
    EXPECT_EQ(config_parsing::ClampFps(-50), 1);
}

TEST_F(TestConfigurationParsing, FpsClampAboveMax) {
    // Test: Clamping FPS above maximum
    EXPECT_EQ(config_parsing::ClampFps(121), 120);
    EXPECT_EQ(config_parsing::ClampFps(500), 120);
}

TEST_F(TestConfigurationParsing, FpsClampWithinRange) {
    // Test: Clamping FPS already within range (no change)
    EXPECT_EQ(config_parsing::ClampFps(30), 30);
    EXPECT_EQ(config_parsing::ClampFps(60), 60);
}

// ============================================================================
// Quality Validation Tests
// ============================================================================

TEST_F(TestConfigurationParsing, QualityMinimalBoundary) {
    // Test: Minimum quality (0)
    EXPECT_TRUE(config_parsing::IsValidQuality(0));
}

TEST_F(TestConfigurationParsing, QualityMaximalBoundary) {
    // Test: Maximum quality (100)
    EXPECT_TRUE(config_parsing::IsValidQuality(100));
}

TEST_F(TestConfigurationParsing, QualityBelow_Min) {
    // Test: Quality below minimum (-1)
    EXPECT_FALSE(config_parsing::IsValidQuality(-1));
}

TEST_F(TestConfigurationParsing, QualityAbove_Max) {
    // Test: Quality above maximum (101)
    EXPECT_FALSE(config_parsing::IsValidQuality(101));
}

TEST_F(TestConfigurationParsing, QualityTypicalValues) {
    // Test: Typical quality values
    EXPECT_TRUE(config_parsing::IsValidQuality(25));
    EXPECT_TRUE(config_parsing::IsValidQuality(50));
    EXPECT_TRUE(config_parsing::IsValidQuality(75));
}

TEST_F(TestConfigurationParsing, QualityClampBelowMin) {
    // Test: Clamping quality below minimum
    EXPECT_EQ(config_parsing::ClampQuality(-1), 0);
    EXPECT_EQ(config_parsing::ClampQuality(-100), 0);
}

TEST_F(TestConfigurationParsing, QualityClampAboveMax) {
    // Test: Clamping quality above maximum
    EXPECT_EQ(config_parsing::ClampQuality(101), 100);
    EXPECT_EQ(config_parsing::ClampQuality(255), 100);
}

TEST_F(TestConfigurationParsing, QualityClampWithinRange) {
    // Test: Clamping quality already within range (no change)
    EXPECT_EQ(config_parsing::ClampQuality(50), 50);
    EXPECT_EQ(config_parsing::ClampQuality(75), 75);
}

// ============================================================================
// Client ID Validation Tests
// ============================================================================

TEST_F(TestConfigurationParsing, ClientIdMinimalBoundary) {
    // Test: Minimum client ID (1)
    EXPECT_TRUE(config_parsing::IsValidClientId(1));
}

TEST_F(TestConfigurationParsing, ClientIdMaximalBoundary) {
    // Test: Maximum client ID (1000000)
    EXPECT_TRUE(config_parsing::IsValidClientId(1000000));
}

TEST_F(TestConfigurationParsing, ClientIdZero) {
    // Test: Client ID of 0 (invalid)
    EXPECT_FALSE(config_parsing::IsValidClientId(0));
}

TEST_F(TestConfigurationParsing, ClientIdAboveMax) {
    // Test: Client ID above maximum (1000001)
    EXPECT_FALSE(config_parsing::IsValidClientId(1000001));
}

TEST_F(TestConfigurationParsing, ClientIdTypicalValues) {
    // Test: Typical client IDs
    EXPECT_TRUE(config_parsing::IsValidClientId(1));
    EXPECT_TRUE(config_parsing::IsValidClientId(100));
    EXPECT_TRUE(config_parsing::IsValidClientId(500000));
    EXPECT_TRUE(config_parsing::IsValidClientId(1000000));
}

// ============================================================================
// Combined Configuration Parsing Tests (Table-Driven)
// ============================================================================

/**
 * Test case structure for table-driven tests
 */
struct ConfigParsingTestCase {
    int fps;
    int quality;
    uint32_t client_id;
    bool expected_valid_fps;
    bool expected_valid_quality;
    bool expected_valid_client_id;
    std::string description;
};

class TestConfigurationParsingTableDriven : public ::testing::TestWithParam<ConfigParsingTestCase> {
};

INSTANTIATE_TEST_SUITE_P(
    ConfigurationParsingCases,
    TestConfigurationParsingTableDriven,
    ::testing::Values(
        // Valid configurations
        ConfigParsingTestCase{30, 75, 100, true, true, true, "Typical valid config"},
        ConfigParsingTestCase{1, 0, 1, true, true, true, "Minimum boundary values"},
        ConfigParsingTestCase{120, 100, 1000000, true, true, true, "Maximum boundary values"},

        // Invalid FPS
        ConfigParsingTestCase{0, 50, 100, false, true, true, "FPS below minimum"},
        ConfigParsingTestCase{121, 50, 100, false, true, true, "FPS above maximum"},
        ConfigParsingTestCase{-10, 50, 100, false, true, true, "Negative FPS"},

        // Invalid quality
        ConfigParsingTestCase{60, -1, 100, true, false, true, "Quality below minimum"},
        ConfigParsingTestCase{60, 101, 100, true, false, true, "Quality above maximum"},

        // Invalid client ID
        ConfigParsingTestCase{60, 50, 0, true, true, false, "Client ID is zero"},
        ConfigParsingTestCase{60, 50, 1000001, true, true, false, "Client ID above maximum"},

        // Multiple invalid fields
        ConfigParsingTestCase{0, -1, 0, false, false, false, "All fields invalid"},
        ConfigParsingTestCase{121, 101, 1000001, false, false, false, "All fields above limits"}
    )
);

TEST_P(TestConfigurationParsingTableDriven, ConfigurationValidation) {
    const auto& test_case = GetParam();

    // Validate each field
    EXPECT_EQ(config_parsing::IsValidFps(test_case.fps), 
              test_case.expected_valid_fps)
        << "FPS validation failed for: " << test_case.description;

    EXPECT_EQ(config_parsing::IsValidQuality(test_case.quality), 
              test_case.expected_valid_quality)
        << "Quality validation failed for: " << test_case.description;

    EXPECT_EQ(config_parsing::IsValidClientId(test_case.client_id), 
              test_case.expected_valid_client_id)
        << "Client ID validation failed for: " << test_case.description;
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(TestConfigurationParsing, FpsEdgeCaseMaxInt) {
    // Test: Extremely large FPS value
    EXPECT_FALSE(config_parsing::IsValidFps(INT_MAX));
}

TEST_F(TestConfigurationParsing, QualityEdgeCaseMaxInt) {
    // Test: Extremely large quality value
    EXPECT_FALSE(config_parsing::IsValidQuality(INT_MAX));
}

TEST_F(TestConfigurationParsing, ClientIdEdgeCaseMaxUint32) {
    // Test: Client ID at max uint32 (exceeds max)
    EXPECT_FALSE(config_parsing::IsValidClientId(UINT32_MAX));
}

TEST_F(TestConfigurationParsing, FpsClampSequence) {
    // Test: Sequence of clamp operations remains stable
    int value = 200;
    value = config_parsing::ClampFps(value);
    EXPECT_EQ(value, 120);
    
    // Clamping again should be idempotent
    value = config_parsing::ClampFps(value);
    EXPECT_EQ(value, 120);
}

TEST_F(TestConfigurationParsing, QualityClampSequence) {
    // Test: Sequence of clamp operations remains stable
    int value = 150;
    value = config_parsing::ClampQuality(value);
    EXPECT_EQ(value, 100);
    
    // Clamping again should be idempotent
    value = config_parsing::ClampQuality(value);
    EXPECT_EQ(value, 100);
}
