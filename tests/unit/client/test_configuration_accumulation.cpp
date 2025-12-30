/**
 * @file test_configuration_accumulation.cpp
 * @brief Unit tests for client configuration accumulation logic
 *
 * Tests validate:
 * - Configuration updates persist across calls
 * - Override behavior (new values replace old ones)
 * - Selective field updates (other fields unchanged)
 * - Default/initial values
 * - Configuration state consistency
 *
 * Typical flow:
 * - Initial: all unset (default values)
 * - Server sends SET_FPS → FPS is updated
 * - Server sends SET_QUALITY → quality is updated, FPS persists
 * - Configuration reflects all accumulated updates
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <memory>

/**
 * Configuration accumulation utilities (header-only, non-production)
 */
namespace config_accumulation {

/**
 * Client configuration state
 */
struct ClientConfig {
    int fps;                    // Frames per second (0 = unset)
    int quality;                // Quality level 0-100 (0 = unset)
    std::string alias;          // Client alias string
    int client_id;              // Client ID (0 = unset)
    
    // Constructor with defaults
    ClientConfig()
        : fps(0), quality(0), alias(""), client_id(0) {
    }
    
    // Check if configuration is fully set
    bool IsFullyConfigured() const {
        return fps != 0 && quality != 0 && !alias.empty() && client_id != 0;
    }
    
    // Check if configuration is partially set
    bool IsPartiallyConfigured() const {
        int set_count = 0;
        if (fps != 0) set_count++;
        if (quality != 0) set_count++;
        if (!alias.empty()) set_count++;
        if (client_id != 0) set_count++;
        return set_count > 0;
    }
    
    // Equality comparison
    bool operator==(const ClientConfig& other) const {
        return fps == other.fps &&
               quality == other.quality &&
               alias == other.alias &&
               client_id == other.client_id;
    }
    
    // Inequality comparison
    bool operator!=(const ClientConfig& other) const {
        return !(*this == other);
    }
};

/**
 * Configuration accumulator for client
 */
class ConfigurationAccumulator {
private:
    ClientConfig m_config;
    int m_update_count;
    
public:
    ConfigurationAccumulator()
        : m_update_count(0) {
    }
    
    /**
     * Get current configuration
     */
    const ClientConfig& GetConfig() const {
        return m_config;
    }
    
    /**
     * Update FPS setting
     * @param fps FPS value to set (1-120)
     */
    void SetFps(int fps) {
        m_config.fps = fps;
        m_update_count++;
    }
    
    /**
     * Update quality setting
     * @param quality Quality value to set (0-100)
     */
    void SetQuality(int quality) {
        m_config.quality = quality;
        m_update_count++;
    }
    
    /**
     * Update client alias
     * @param alias Alias string to set
     */
    void SetAlias(const std::string& alias) {
        m_config.alias = alias;
        m_update_count++;
    }
    
    /**
     * Update client ID
     * @param client_id Client ID to set
     */
    void SetClientId(int client_id) {
        m_config.client_id = client_id;
        m_update_count++;
    }
    
    /**
     * Get number of configuration updates made
     */
    int GetUpdateCount() const {
        return m_update_count;
    }
    
    /**
     * Reset configuration to defaults
     */
    void Reset() {
        m_config = ClientConfig();
        m_update_count = 0;
    }
};

} // namespace config_accumulation

/**
 * Test fixture for configuration accumulation
 */
class TestConfigurationAccumulation : public ::testing::Test {
protected:
    config_accumulation::ConfigurationAccumulator m_accumulator;
    
    void SetUp() override {
        m_accumulator.Reset();
    }
};

// ============================================================================
// Initial Configuration Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, InitialConfigUnset) {
    // Test: Initial configuration is unset
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 0);
    EXPECT_EQ(config.quality, 0);
    EXPECT_EQ(config.alias, "");
    EXPECT_EQ(config.client_id, 0);
}

TEST_F(TestConfigurationAccumulation, InitialNotFullyConfigured) {
    // Test: Initial state is not fully configured
    auto config = m_accumulator.GetConfig();
    EXPECT_FALSE(config.IsFullyConfigured());
}

TEST_F(TestConfigurationAccumulation, InitialNotPartiallyConfigured) {
    // Test: Initial state is not partially configured
    auto config = m_accumulator.GetConfig();
    EXPECT_FALSE(config.IsPartiallyConfigured());
}

// ============================================================================
// Single Configuration Update Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, SetFpsOnly) {
    // Test: Set FPS, other fields remain unset
    m_accumulator.SetFps(30);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 30);
    EXPECT_EQ(config.quality, 0);      // Unchanged
    EXPECT_EQ(config.alias, "");       // Unchanged
    EXPECT_EQ(config.client_id, 0);    // Unchanged
}

TEST_F(TestConfigurationAccumulation, SetQualityOnly) {
    // Test: Set quality, other fields remain unset
    m_accumulator.SetQuality(75);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 0);          // Unchanged
    EXPECT_EQ(config.quality, 75);
    EXPECT_EQ(config.alias, "");       // Unchanged
    EXPECT_EQ(config.client_id, 0);    // Unchanged
}

TEST_F(TestConfigurationAccumulation, SetAliasOnly) {
    // Test: Set alias, other fields remain unset
    m_accumulator.SetAlias("laptop-1");
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 0);          // Unchanged
    EXPECT_EQ(config.quality, 0);      // Unchanged
    EXPECT_EQ(config.alias, "laptop-1");
    EXPECT_EQ(config.client_id, 0);    // Unchanged
}

TEST_F(TestConfigurationAccumulation, SetClientIdOnly) {
    // Test: Set client ID, other fields remain unset
    m_accumulator.SetClientId(12345);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 0);          // Unchanged
    EXPECT_EQ(config.quality, 0);      // Unchanged
    EXPECT_EQ(config.alias, "");       // Unchanged
    EXPECT_EQ(config.client_id, 12345);
}

// ============================================================================
// Sequential Configuration Update Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, SequentialUpdatesAccumulate) {
    // Test: Sequential updates accumulate without overwriting
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(75);
    m_accumulator.SetAlias("desktop");
    m_accumulator.SetClientId(100);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 30);
    EXPECT_EQ(config.quality, 75);
    EXPECT_EQ(config.alias, "desktop");
    EXPECT_EQ(config.client_id, 100);
}

TEST_F(TestConfigurationAccumulation, FpsAndQualityAccumulate) {
    // Test: FPS and quality updates persist together
    m_accumulator.SetFps(60);
    auto config1 = m_accumulator.GetConfig();
    EXPECT_EQ(config1.fps, 60);
    
    m_accumulator.SetQuality(90);
    auto config2 = m_accumulator.GetConfig();
    EXPECT_EQ(config2.fps, 60);      // FPS persists
    EXPECT_EQ(config2.quality, 90);
}

TEST_F(TestConfigurationAccumulation, AliasAndIdAccumulate) {
    // Test: Alias and ID updates persist together
    m_accumulator.SetAlias("server-1");
    auto config1 = m_accumulator.GetConfig();
    EXPECT_EQ(config1.alias, "server-1");
    
    m_accumulator.SetClientId(999);
    auto config2 = m_accumulator.GetConfig();
    EXPECT_EQ(config2.alias, "server-1");   // Alias persists
    EXPECT_EQ(config2.client_id, 999);
}

// ============================================================================
// Configuration Override Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, FpsOverride) {
    // Test: FPS override replaces previous value
    m_accumulator.SetFps(30);
    m_accumulator.SetFps(60);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 60);
}

TEST_F(TestConfigurationAccumulation, QualityOverride) {
    // Test: Quality override replaces previous value
    m_accumulator.SetQuality(50);
    m_accumulator.SetQuality(100);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.quality, 100);
}

TEST_F(TestConfigurationAccumulation, AliasOverride) {
    // Test: Alias override replaces previous value
    m_accumulator.SetAlias("old-name");
    m_accumulator.SetAlias("new-name");
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.alias, "new-name");
}

TEST_F(TestConfigurationAccumulation, ClientIdOverride) {
    // Test: Client ID override replaces previous value
    m_accumulator.SetClientId(100);
    m_accumulator.SetClientId(200);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.client_id, 200);
}

TEST_F(TestConfigurationAccumulation, MultipleOverridesWithAccumulation) {
    // Test: Multiple overrides with other fields persisting
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    
    auto config1 = m_accumulator.GetConfig();
    EXPECT_EQ(config1.fps, 30);
    EXPECT_EQ(config1.quality, 50);
    
    // Override FPS, quality should persist
    m_accumulator.SetFps(60);
    auto config2 = m_accumulator.GetConfig();
    EXPECT_EQ(config2.fps, 60);
    EXPECT_EQ(config2.quality, 50);   // Quality persists
    
    // Override quality, FPS should persist
    m_accumulator.SetQuality(75);
    auto config3 = m_accumulator.GetConfig();
    EXPECT_EQ(config3.fps, 60);       // FPS persists
    EXPECT_EQ(config3.quality, 75);
}

// ============================================================================
// Configuration Completeness Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, PartiallyConfigured_OneField) {
    // Test: Single field is partially configured
    m_accumulator.SetFps(30);
    auto config = m_accumulator.GetConfig();
    EXPECT_TRUE(config.IsPartiallyConfigured());
}

TEST_F(TestConfigurationAccumulation, PartiallyConfigured_TwoFields) {
    // Test: Two fields is partially configured
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    auto config = m_accumulator.GetConfig();
    EXPECT_TRUE(config.IsPartiallyConfigured());
}

TEST_F(TestConfigurationAccumulation, PartiallyConfigured_ThreeFields) {
    // Test: Three fields is partially configured
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    m_accumulator.SetAlias("test");
    auto config = m_accumulator.GetConfig();
    EXPECT_TRUE(config.IsPartiallyConfigured());
}

TEST_F(TestConfigurationAccumulation, FullyConfigured_AllFields) {
    // Test: All fields set means fully configured
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    m_accumulator.SetAlias("test");
    m_accumulator.SetClientId(100);
    auto config = m_accumulator.GetConfig();
    EXPECT_TRUE(config.IsFullyConfigured());
}

TEST_F(TestConfigurationAccumulation, NotFullyConfiguredMissingFps) {
    // Test: Missing FPS means not fully configured
    m_accumulator.SetQuality(50);
    m_accumulator.SetAlias("test");
    m_accumulator.SetClientId(100);
    auto config = m_accumulator.GetConfig();
    EXPECT_FALSE(config.IsFullyConfigured());
}

TEST_F(TestConfigurationAccumulation, NotFullyConfiguredMissingQuality) {
    // Test: Missing quality means not fully configured
    m_accumulator.SetFps(30);
    m_accumulator.SetAlias("test");
    m_accumulator.SetClientId(100);
    auto config = m_accumulator.GetConfig();
    EXPECT_FALSE(config.IsFullyConfigured());
}

// ============================================================================
// Update Count Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, UpdateCountStartsAtZero) {
    // Test: Initial update count is zero
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 0);
}

TEST_F(TestConfigurationAccumulation, UpdateCountIncrementsOnSet) {
    // Test: Update count increments with each set operation
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 0);
    
    m_accumulator.SetFps(30);
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 1);
    
    m_accumulator.SetQuality(50);
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 2);
    
    m_accumulator.SetAlias("test");
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 3);
    
    m_accumulator.SetClientId(100);
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 4);
}

TEST_F(TestConfigurationAccumulation, UpdateCountIncludesOverrides) {
    // Test: Update count includes overwrite operations
    m_accumulator.SetFps(30);
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 1);
    
    m_accumulator.SetFps(60);  // Override
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 2);
    
    m_accumulator.SetFps(90);  // Override again
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 3);
}

// ============================================================================
// Reset and Reuse Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, ResetClearsConfiguration) {
    // Test: Reset clears all configuration
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    m_accumulator.SetAlias("test");
    m_accumulator.SetClientId(100);
    
    m_accumulator.Reset();
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 0);
    EXPECT_EQ(config.quality, 0);
    EXPECT_EQ(config.alias, "");
    EXPECT_EQ(config.client_id, 0);
}

TEST_F(TestConfigurationAccumulation, ResetClearsUpdateCount) {
    // Test: Reset clears update count
    m_accumulator.SetFps(30);
    m_accumulator.SetQuality(50);
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 2);
    
    m_accumulator.Reset();
    EXPECT_EQ(m_accumulator.GetUpdateCount(), 0);
}

TEST_F(TestConfigurationAccumulation, ReuseAfterReset) {
    // Test: Can reuse accumulator after reset
    m_accumulator.SetFps(30);
    m_accumulator.Reset();
    m_accumulator.SetFps(60);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 60);
}

// ============================================================================
// Configuration Equality Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, ConfigEqualityDefault) {
    // Test: Two default configs are equal
    config_accumulation::ClientConfig cfg1;
    config_accumulation::ClientConfig cfg2;
    EXPECT_EQ(cfg1, cfg2);
}

TEST_F(TestConfigurationAccumulation, ConfigEqualityAfterUpdate) {
    // Test: Configs with same values are equal
    config_accumulation::ClientConfig cfg1;
    config_accumulation::ClientConfig cfg2;
    
    cfg1.fps = 30;
    cfg1.quality = 50;
    
    cfg2.fps = 30;
    cfg2.quality = 50;
    
    EXPECT_EQ(cfg1, cfg2);
}

TEST_F(TestConfigurationAccumulation, ConfigInequalityDifferentFps) {
    // Test: Configs with different FPS are not equal
    config_accumulation::ClientConfig cfg1;
    config_accumulation::ClientConfig cfg2;
    
    cfg1.fps = 30;
    cfg2.fps = 60;
    
    EXPECT_NE(cfg1, cfg2);
}

// ============================================================================
// Practical Scenario Tests
// ============================================================================

TEST_F(TestConfigurationAccumulation, ServerSetFpsMessage) {
    // Test: Simulate server SET_FPS message
    m_accumulator.SetFps(30);
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 30);
}

TEST_F(TestConfigurationAccumulation, ServerMultipleMessages) {
    // Test: Simulate server sending multiple messages
    m_accumulator.SetFps(30);           // SET_FPS message
    m_accumulator.SetQuality(75);       // SET_QUALITY message
    m_accumulator.SetClientId(100);     // Implicit from connection
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.fps, 30);
    EXPECT_EQ(config.quality, 75);
    EXPECT_EQ(config.client_id, 100);
}

TEST_F(TestConfigurationAccumulation, ClientAliasFromAlias) {
    // Test: Client alias can be set from ALIAS message
    m_accumulator.SetAlias("my-laptop");
    m_accumulator.SetClientId(123);
    
    auto config = m_accumulator.GetConfig();
    EXPECT_EQ(config.alias, "my-laptop");
    EXPECT_EQ(config.client_id, 123);
}
