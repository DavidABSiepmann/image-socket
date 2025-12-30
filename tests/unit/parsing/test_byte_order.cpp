/**
 * @file test_byte_order.cpp
 * @brief Unit tests for byte order conversion and endianness
 *
 * Tests validate:
 * - Host to network byte order conversion (hton)
 * - Network to host byte order conversion (ntoh)
 * - Roundtrip conversions (value → encoded → decoded → value)
 * - 32-bit and 16-bit conversions
 * - Boundary values and special cases
 *
 * Test philosophy:
 * - Byte order conversion must be deterministic and reversible
 * - roundtrip(value) must equal value
 * - Different input values must produce different byte representations
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

/**
 * Byte order conversion utilities (header-only test helpers)
 * These mirror the common_test_helpers.h functions
 */
namespace byte_order {

/**
 * Convert 32-bit value from host to network byte order (big-endian)
 * @param value Host byte order value
 * @return Network byte order (big-endian) value
 */
inline uint32_t HostToNetworkByteOrder32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x000000FF) << 24);
}

/**
 * Convert 32-bit value from network to host byte order
 * @param value Network byte order (big-endian) value
 * @return Host byte order value
 */
inline uint32_t NetworkToHostByteOrder32(uint32_t value) {
    // For big-endian conversion, the operation is symmetric
    return HostToNetworkByteOrder32(value);
}

/**
 * Convert 16-bit value from host to network byte order (big-endian)
 * @param value Host byte order value
 * @return Network byte order (big-endian) value
 */
inline uint16_t HostToNetworkByteOrder16(uint16_t value) {
    return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
}

/**
 * Convert 16-bit value from network to host byte order
 * @param value Network byte order (big-endian) value
 * @return Host byte order value
 */
inline uint16_t NetworkToHostByteOrder16(uint16_t value) {
    return HostToNetworkByteOrder16(value);
}

/**
 * Extract individual bytes from a 32-bit network order value
 * @param value Network byte order (big-endian) value
 * @return Vector of 4 bytes in order [MSB, ..., LSB]
 */
inline std::vector<uint8_t> ExtractBytes32(uint32_t value) {
    return {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
}

/**
 * Extract individual bytes from a 16-bit network order value
 * @param value Network byte order (big-endian) value
 * @return Vector of 2 bytes in order [MSB, LSB]
 */
inline std::vector<uint8_t> ExtractBytes16(uint16_t value) {
    return {
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
}

} // namespace byte_order

/**
 * Test fixture for byte order conversion
 */
class TestByteOrder : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }
};

// ============================================================================
// 32-bit Byte Order Conversion Tests
// ============================================================================

TEST_F(TestByteOrder, Host32ToNetworkZero) {
    // Test: Zero value conversion
    uint32_t value = 0x00000000;
    uint32_t converted = byte_order::HostToNetworkByteOrder32(value);
    EXPECT_EQ(converted, 0x00000000);
}

TEST_F(TestByteOrder, Host32ToNetworkAllOnes) {
    // Test: All bits set conversion
    uint32_t value = 0xFFFFFFFF;
    uint32_t converted = byte_order::HostToNetworkByteOrder32(value);
    EXPECT_EQ(converted, 0xFFFFFFFF);
}

TEST_F(TestByteOrder, Host32ToNetworkTypicalValue) {
    // Test: Typical value (0x12345678)
    uint32_t value = 0x12345678;
    uint32_t converted = byte_order::HostToNetworkByteOrder32(value);
    // Big-endian: bytes become [0x12, 0x34, 0x56, 0x78]
    EXPECT_EQ(converted, 0x78563412);
}

TEST_F(TestByteOrder, Host32ToNetworkSingleByteSet) {
    // Test: Only MSB set
    uint32_t value = 0x80000000;
    uint32_t converted = byte_order::HostToNetworkByteOrder32(value);
    // MSB (0x80) moves to LSB position
    EXPECT_EQ(converted, 0x00000080);
}

TEST_F(TestByteOrder, Host32ToNetworkLSBSet) {
    // Test: Only LSB set
    uint32_t value = 0x00000001;
    uint32_t converted = byte_order::HostToNetworkByteOrder32(value);
    // LSB (0x01) moves to MSB position
    EXPECT_EQ(converted, 0x01000000);
}

// ============================================================================
// 32-bit Roundtrip Conversion Tests
// ============================================================================

TEST_F(TestByteOrder, Host32RoundtripZero) {
    // Test: Zero value roundtrip
    uint32_t original = 0x00000000;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(original);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host32RoundtripAllOnes) {
    // Test: All bits roundtrip
    uint32_t original = 0xFFFFFFFF;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(original);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host32RoundtripTypicalValue) {
    // Test: Typical value roundtrip
    uint32_t original = 0x12345678;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(original);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host32RoundtripSequentialValues) {
    // Test: Multiple sequential values roundtrip correctly
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t original = i * 1000 + 12345;
        uint32_t encoded = byte_order::HostToNetworkByteOrder32(original);
        uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
        EXPECT_EQ(decoded, original) 
            << "Roundtrip failed for value: " << original;
    }
}

TEST_F(TestByteOrder, Host32RoundtripPowerOfTwo) {
    // Test: Powers of two roundtrip correctly
    uint32_t powers[] = {1, 2, 4, 8, 16, 256, 65536, 1048576};
    for (uint32_t value : powers) {
        uint32_t encoded = byte_order::HostToNetworkByteOrder32(value);
        uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
        EXPECT_EQ(decoded, value);
    }
}

// ============================================================================
// 16-bit Byte Order Conversion Tests
// ============================================================================

TEST_F(TestByteOrder, Host16ToNetworkZero) {
    // Test: Zero value conversion
    uint16_t value = 0x0000;
    uint16_t converted = byte_order::HostToNetworkByteOrder16(value);
    EXPECT_EQ(converted, 0x0000);
}

TEST_F(TestByteOrder, Host16ToNetworkAllOnes) {
    // Test: All bits set conversion
    uint16_t value = 0xFFFF;
    uint16_t converted = byte_order::HostToNetworkByteOrder16(value);
    EXPECT_EQ(converted, 0xFFFF);
}

TEST_F(TestByteOrder, Host16ToNetworkTypicalValue) {
    // Test: Typical value (0x1234)
    uint16_t value = 0x1234;
    uint16_t converted = byte_order::HostToNetworkByteOrder16(value);
    // Big-endian: bytes swap
    EXPECT_EQ(converted, 0x3412);
}

TEST_F(TestByteOrder, Host16ToNetworkMSBOnly) {
    // Test: MSB set
    uint16_t value = 0x8000;
    uint16_t converted = byte_order::HostToNetworkByteOrder16(value);
    EXPECT_EQ(converted, 0x0080);
}

TEST_F(TestByteOrder, Host16ToNetworkLSBOnly) {
    // Test: LSB set
    uint16_t value = 0x0001;
    uint16_t converted = byte_order::HostToNetworkByteOrder16(value);
    EXPECT_EQ(converted, 0x0100);
}

// ============================================================================
// 16-bit Roundtrip Conversion Tests
// ============================================================================

TEST_F(TestByteOrder, Host16RoundtripZero) {
    // Test: Zero roundtrip
    uint16_t original = 0x0000;
    uint16_t encoded = byte_order::HostToNetworkByteOrder16(original);
    uint16_t decoded = byte_order::NetworkToHostByteOrder16(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host16RoundtripAllOnes) {
    // Test: All bits roundtrip
    uint16_t original = 0xFFFF;
    uint16_t encoded = byte_order::HostToNetworkByteOrder16(original);
    uint16_t decoded = byte_order::NetworkToHostByteOrder16(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host16RoundtripTypicalValue) {
    // Test: Typical value roundtrip
    uint16_t original = 0x1234;
    uint16_t encoded = byte_order::HostToNetworkByteOrder16(original);
    uint16_t decoded = byte_order::NetworkToHostByteOrder16(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(TestByteOrder, Host16RoundtripAllValues) {
    // Test: All 16-bit values roundtrip correctly (sampling every 256)
    for (int i = 0; i < 65536; i += 256) {
        uint16_t original = static_cast<uint16_t>(i);
        uint16_t encoded = byte_order::HostToNetworkByteOrder16(original);
        uint16_t decoded = byte_order::NetworkToHostByteOrder16(encoded);
        EXPECT_EQ(decoded, original);
    }
}

// ============================================================================
// Byte Extraction and Representation Tests
// ============================================================================

TEST_F(TestByteOrder, ExtractBytes32_Zero) {
    // Test: Extract bytes from zero
    auto bytes = byte_order::ExtractBytes32(0x00000000);
    EXPECT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0x00);
    EXPECT_EQ(bytes[1], 0x00);
    EXPECT_EQ(bytes[2], 0x00);
    EXPECT_EQ(bytes[3], 0x00);
}

TEST_F(TestByteOrder, ExtractBytes32_Typical) {
    // Test: Extract bytes from typical value
    auto bytes = byte_order::ExtractBytes32(0x12345678);
    EXPECT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0x12);
    EXPECT_EQ(bytes[1], 0x34);
    EXPECT_EQ(bytes[2], 0x56);
    EXPECT_EQ(bytes[3], 0x78);
}

TEST_F(TestByteOrder, ExtractBytes32_AllOnes) {
    // Test: Extract bytes from all ones
    auto bytes = byte_order::ExtractBytes32(0xFFFFFFFF);
    EXPECT_EQ(bytes.size(), 4);
    EXPECT_EQ(bytes[0], 0xFF);
    EXPECT_EQ(bytes[1], 0xFF);
    EXPECT_EQ(bytes[2], 0xFF);
    EXPECT_EQ(bytes[3], 0xFF);
}

TEST_F(TestByteOrder, ExtractBytes16_Zero) {
    // Test: Extract bytes from zero
    auto bytes = byte_order::ExtractBytes16(0x0000);
    EXPECT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x00);
    EXPECT_EQ(bytes[1], 0x00);
}

TEST_F(TestByteOrder, ExtractBytes16_Typical) {
    // Test: Extract bytes from typical value
    auto bytes = byte_order::ExtractBytes16(0x1234);
    EXPECT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0x12);
    EXPECT_EQ(bytes[1], 0x34);
}

TEST_F(TestByteOrder, ExtractBytes16_AllOnes) {
    // Test: Extract bytes from all ones
    auto bytes = byte_order::ExtractBytes16(0xFFFF);
    EXPECT_EQ(bytes.size(), 2);
    EXPECT_EQ(bytes[0], 0xFF);
    EXPECT_EQ(bytes[1], 0xFF);
}

// ============================================================================
// Determinism and Consistency Tests
// ============================================================================

TEST_F(TestByteOrder, ConversionDeterministic32) {
    // Test: Same input produces same output every time
    uint32_t value = 0xDEADBEEF;
    uint32_t result1 = byte_order::HostToNetworkByteOrder32(value);
    uint32_t result2 = byte_order::HostToNetworkByteOrder32(value);
    uint32_t result3 = byte_order::HostToNetworkByteOrder32(value);
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result2, result3);
}

TEST_F(TestByteOrder, ConversionDeterministic16) {
    // Test: Same input produces same output every time
    uint16_t value = 0xDEAD;
    uint16_t result1 = byte_order::HostToNetworkByteOrder16(value);
    uint16_t result2 = byte_order::HostToNetworkByteOrder16(value);
    uint16_t result3 = byte_order::HostToNetworkByteOrder16(value);
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result2, result3);
}

TEST_F(TestByteOrder, DifferentInputsDifferentOutputs32) {
    // Test: Different inputs produce different outputs
    uint32_t value1 = 0x12345678;
    uint32_t value2 = 0x87654321;
    uint32_t conv1 = byte_order::HostToNetworkByteOrder32(value1);
    uint32_t conv2 = byte_order::HostToNetworkByteOrder32(value2);
    EXPECT_NE(conv1, conv2);
}

TEST_F(TestByteOrder, DifferentInputsDifferentOutputs16) {
    // Test: Different inputs produce different outputs
    uint16_t value1 = 0x1234;
    uint16_t value2 = 0x4321;
    uint16_t conv1 = byte_order::HostToNetworkByteOrder16(value1);
    uint16_t conv2 = byte_order::HostToNetworkByteOrder16(value2);
    EXPECT_NE(conv1, conv2);
}

// ============================================================================
// Special Value Tests
// ============================================================================

TEST_F(TestByteOrder, SpecialValue_MaxUint32) {
    // Test: Maximum uint32 value
    uint32_t value = UINT32_MAX;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(value);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, value);
}

TEST_F(TestByteOrder, SpecialValue_MaxUint16) {
    // Test: Maximum uint16 value
    uint16_t value = UINT16_MAX;
    uint16_t encoded = byte_order::HostToNetworkByteOrder16(value);
    uint16_t decoded = byte_order::NetworkToHostByteOrder16(encoded);
    EXPECT_EQ(decoded, value);
}

TEST_F(TestByteOrder, SpecialValue_One) {
    // Test: Value 1 roundtrips correctly
    uint32_t value = 1;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(value);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, value);
}

TEST_F(TestByteOrder, SpecialValue_AlternateBits) {
    // Test: Alternating bit pattern (0xAAAAAAAA)
    uint32_t value = 0xAAAAAAAA;
    uint32_t encoded = byte_order::HostToNetworkByteOrder32(value);
    uint32_t decoded = byte_order::NetworkToHostByteOrder32(encoded);
    EXPECT_EQ(decoded, value);
}
