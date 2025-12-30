#ifndef COMMON_TEST_HELPERS_H
#define COMMON_TEST_HELPERS_H

#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

/**
 * @file common_test_helpers.h
 * @brief Common test utilities shared across unit tests
 *
 * This fixture provides utility functions for:
 * - Byte manipulation and conversion
 * - Data comparison and validation
 * - Common test setup/teardown patterns
 * - Buffer/payload generation
 *
 * Usage:
 *   TestHelpers helpers;
 *   auto be_value = helpers.HostToNetworkByteOrder(12345);
 *   ASSERT_TRUE(helpers.BuffersEqual(buffer1, buffer2));
 */

namespace testing {
namespace fixtures {

/**
 * TestHelpers: Common utilities for all unit tests
 * 
 * Provides platform-independent byte order handling, buffer utilities,
 * and common validation functions.
 */
class TestHelpers {
public:
    // -----------------------------------------------------------------------
    // Byte order conversion (host <-> network)
    // -----------------------------------------------------------------------

    /**
     * @brief Convert a 32-bit value from host to network byte order (big-endian)
     * @param value Host byte order value
     * @return Network byte order (big-endian) value
     */
    static uint32_t HostToNetworkByteOrder(uint32_t value) {
        return ((value & 0xFF000000) >> 24) |
               ((value & 0x00FF0000) >> 8) |
               ((value & 0x0000FF00) << 8) |
               ((value & 0x000000FF) << 24);
    }

    /**
     * @brief Convert a 32-bit value from network to host byte order
     * @param value Network byte order (big-endian) value
     * @return Host byte order value
     */
    static uint32_t NetworkToHostByteOrder(uint32_t value) {
        // Same operation for big-endian conversion
        return HostToNetworkByteOrder(value);
    }

    /**
     * @brief Convert a 16-bit value from host to network byte order (big-endian)
     * @param value Host byte order value
     * @return Network byte order (big-endian) value
     */
    static uint16_t HostToNetworkByteOrder16(uint16_t value) {
        return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
    }

    /**
     * @brief Convert a 16-bit value from network to host byte order
     * @param value Network byte order (big-endian) value
     * @return Host byte order value
     */
    static uint16_t NetworkToHostByteOrder16(uint16_t value) {
        return HostToNetworkByteOrder16(value);
    }

    // -----------------------------------------------------------------------
    // Buffer and data comparison
    // -----------------------------------------------------------------------

    /**
     * @brief Compare two buffers for equality
     * @param buffer1 First buffer
     * @param size1 Size of first buffer
     * @param buffer2 Second buffer
     * @param size2 Size of second buffer
     * @return true if buffers are identical, false otherwise
     */
    static bool BuffersEqual(const uint8_t* buffer1, size_t size1,
                             const uint8_t* buffer2, size_t size2) {
        if (size1 != size2) {
            return false;
        }
        return std::equal(buffer1, buffer1 + size1, buffer2);
    }

    /**
     * @brief Compare two vectors for equality
     * @param vec1 First vector
     * @param vec2 Second vector
     * @return true if vectors are identical, false otherwise
     */
    static bool VectorsEqual(const std::vector<uint8_t>& vec1,
                             const std::vector<uint8_t>& vec2) {
        return vec1 == vec2;
    }

    /**
     * @brief Find a subsequence in a buffer
     * @param buffer Pointer to buffer to search
     * @param buffer_size Size of buffer
     * @param subsequence Pointer to subsequence to find
     * @param subseq_size Size of subsequence
     * @return Offset of first occurrence, or -1 if not found
     */
    static int FindSubsequence(const uint8_t* buffer, size_t buffer_size,
                               const uint8_t* subsequence, size_t subseq_size) {
        auto result = std::search(buffer, buffer + buffer_size,
                                  subsequence, subsequence + subseq_size);
        if (result == buffer + buffer_size) {
            return -1;  // Not found
        }
        return static_cast<int>(std::distance(buffer, result));
    }

    // -----------------------------------------------------------------------
    // Buffer generation and manipulation
    // -----------------------------------------------------------------------

    /**
     * @brief Create a buffer filled with a specific byte value
     * @param size Size of buffer to create
     * @param fill_byte Byte value to fill with
     * @return Vector filled with the specified byte
     */
    static std::vector<uint8_t> CreateFilledBuffer(size_t size, uint8_t fill_byte) {
        return std::vector<uint8_t>(size, fill_byte);
    }

    /**
     * @brief Create a buffer with sequential byte values
     * @param size Size of buffer to create
     * @param start Starting byte value
     * @return Vector with bytes 0, 1, 2, ... (size-1)
     */
    static std::vector<uint8_t> CreateSequentialBuffer(size_t size, uint8_t start = 0) {
        std::vector<uint8_t> buffer;
        for (size_t i = 0; i < size; ++i) {
            buffer.push_back(static_cast<uint8_t>((start + i) % 256));
        }
        return buffer;
    }

    /**
     * @brief Create a random-like buffer using simple PRNG
     * @param size Size of buffer to create
     * @param seed Seed for PRNG
     * @return Vector with pseudo-random bytes
     */
    static std::vector<uint8_t> CreatePseudoRandomBuffer(size_t size, uint32_t seed = 12345) {
        std::vector<uint8_t> buffer;
        uint32_t random = seed;
        for (size_t i = 0; i < size; ++i) {
            random = (random * 1103515245 + 12345) % (1u << 31);
            buffer.push_back(static_cast<uint8_t>((random >> 8) & 0xFF));
        }
        return buffer;
    }

    // -----------------------------------------------------------------------
    // String and hex conversion
    // -----------------------------------------------------------------------

    /**
     * @brief Convert a buffer to hexadecimal string for debugging
     * @param buffer Pointer to buffer
     * @param size Size of buffer
     * @return Hexadecimal string representation (e.g., "48656C6C6F")
     */
    static std::string BufferToHexString(const uint8_t* buffer, size_t size) {
        static const char hex_chars[] = "0123456789ABCDEF";
        std::string result;
        for (size_t i = 0; i < size; ++i) {
            uint8_t byte = buffer[i];
            result += hex_chars[(byte >> 4) & 0xF];
            result += hex_chars[byte & 0xF];
        }
        return result;
    }

    /**
     * @brief Convert vector to hexadecimal string
     * @param data Vector of bytes
     * @return Hexadecimal string representation
     */
    static std::string VectorToHexString(const std::vector<uint8_t>& data) {
        return BufferToHexString(data.data(), data.size());
    }

    /**
     * @brief Convert bytes to string (useful for debugging)
     * @param buffer Pointer to buffer
     * @param size Size of buffer
     * @return String representation
     */
    static std::string BufferToString(const uint8_t* buffer, size_t size) {
        return std::string(reinterpret_cast<const char*>(buffer), size);
    }

    // -----------------------------------------------------------------------
    // Validation helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Check if all bytes in a buffer have a specific value
     * @param buffer Pointer to buffer
     * @param size Size of buffer
     * @param value Expected byte value
     * @return true if all bytes match the value, false otherwise
     */
    static bool BufferAllBytesEqual(const uint8_t* buffer, size_t size, uint8_t value) {
        return std::all_of(buffer, buffer + size, [value](uint8_t byte) {
            return byte == value;
        });
    }

    /**
     * @brief Check if buffer is valid UTF-8
     * @param buffer Pointer to buffer
     * @param size Size of buffer
     * @return true if buffer contains valid UTF-8, false otherwise
     */
    static bool IsValidUtf8(const uint8_t* buffer, size_t size) {
        // Simplified UTF-8 validation
        size_t i = 0;
        while (i < size) {
            uint8_t byte = buffer[i];
            
            if ((byte & 0x80) == 0) {
                // Single byte character (0xxxxxxx)
                i++;
            } else if ((byte & 0xE0) == 0xC0) {
                // Two byte character (110xxxxx 10xxxxxx)
                if (i + 1 >= size || (buffer[i + 1] & 0xC0) != 0x80) {
                    return false;
                }
                i += 2;
            } else if ((byte & 0xF0) == 0xE0) {
                // Three byte character (1110xxxx 10xxxxxx 10xxxxxx)
                if (i + 2 >= size || 
                    (buffer[i + 1] & 0xC0) != 0x80 ||
                    (buffer[i + 2] & 0xC0) != 0x80) {
                    return false;
                }
                i += 3;
            } else if ((byte & 0xF8) == 0xF0) {
                // Four byte character (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                if (i + 3 >= size ||
                    (buffer[i + 1] & 0xC0) != 0x80 ||
                    (buffer[i + 2] & 0xC0) != 0x80 ||
                    (buffer[i + 3] & 0xC0) != 0x80) {
                    return false;
                }
                i += 4;
            } else {
                return false;  // Invalid UTF-8 sequence
            }
        }
        return true;
    }

    // -----------------------------------------------------------------------
    // Range and boundary checking
    // -----------------------------------------------------------------------

    /**
     * @brief Check if a value is within a valid range
     * @param value Value to check
     * @param min Minimum valid value (inclusive)
     * @param max Maximum valid value (inclusive)
     * @return true if value is in range, false otherwise
     */
    template<typename T>
    static bool IsInRange(T value, T min, T max) {
        return value >= min && value <= max;
    }

    /**
     * @brief Get a value clamped to a range
     * @param value Value to clamp
     * @param min Minimum value
     * @param max Maximum value
     * @return Clamped value
     */
    template<typename T>
    static T ClampValue(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
};

}  // namespace fixtures
}  // namespace testing

#endif  // COMMON_TEST_HELPERS_H
