/**
 * @file test_frame_parsing.cpp
 * @brief Unit tests for length-prefixed message framing logic
 *
 * Tests validate:
 * - Frame header parsing (length-prefixed format)
 * - Payload extraction from frames
 * - Frame validation (length consistency)
 * - Boundary conditions (min/max payload sizes)
 * - Multiple frames in a stream (partial, complete, multiple)
 *
 * Frame format:
 * - 4-byte network order length prefix (big-endian)
 * - Variable-length payload
 * - Total frame size = 4 (header) + payload_length
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <cstring>

/**
 * Frame parsing utilities (header-only test helpers)
 */
namespace frame_parsing {

// Frame constants
constexpr size_t FRAME_HEADER_SIZE = 4;  // 4-byte length prefix
constexpr size_t MAX_PAYLOAD_SIZE = 10 * 1024 * 1024;  // 10 MB
constexpr size_t MIN_PAYLOAD_SIZE = 0;

/**
 * Pack payload length into a 4-byte network order (big-endian) header
 * @param length Length value to encode
 * @return 4-byte header in network byte order
 */
inline std::vector<uint8_t> PackFrameHeader(uint32_t length) {
    return {
        static_cast<uint8_t>((length >> 24) & 0xFF),
        static_cast<uint8_t>((length >> 16) & 0xFF),
        static_cast<uint8_t>((length >> 8) & 0xFF),
        static_cast<uint8_t>(length & 0xFF)
    };
}

/**
 * Parse a 4-byte network order header to extract payload length
 * @param header 4-byte header (must be at least 4 bytes)
 * @return Payload length in host byte order
 */
inline uint32_t ParseFrameHeader(const uint8_t* header) {
    return (static_cast<uint32_t>(header[0]) << 24) |
           (static_cast<uint32_t>(header[1]) << 16) |
           (static_cast<uint32_t>(header[2]) << 8) |
           (static_cast<uint32_t>(header[3]));
}

/**
 * Parse a 4-byte network order header from vector
 * @param header Vector containing at least 4 bytes
 * @return Payload length in host byte order
 */
inline uint32_t ParseFrameHeader(const std::vector<uint8_t>& header) {
    if (header.size() < FRAME_HEADER_SIZE) {
        return 0;  // Invalid header
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"
    return ParseFrameHeader(header.data());
#pragma GCC diagnostic pop
}

/**
 * Create a complete frame (header + payload)
 * @param payload Payload bytes
 * @return Complete frame (header + payload)
 */
inline std::vector<uint8_t> CreateFrame(const std::vector<uint8_t>& payload) {
    auto header = PackFrameHeader(static_cast<uint32_t>(payload.size()));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"
    header.insert(header.end(), payload.begin(), payload.end());
#pragma GCC diagnostic pop
    return header;
}

/**
 * Check if a buffer contains enough bytes for a complete frame
 * @param buffer Buffer to check
 * @return true if buffer has complete frame (header + payload)
 */
inline bool HasCompleteFrame(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < FRAME_HEADER_SIZE) {
        return false;  // Not even header is complete
    }

    uint32_t payload_length = ParseFrameHeader(buffer.data());
    return buffer.size() >= (FRAME_HEADER_SIZE + payload_length);
}

/**
 * Extract the next frame from a buffer
 * @param buffer Buffer containing frame data
 * @param[out] frame Output vector for the extracted frame
 * @return Size of extracted frame (0 if no complete frame)
 */
inline size_t ExtractFrame(const std::vector<uint8_t>& buffer, std::vector<uint8_t>& frame) {
    if (!HasCompleteFrame(buffer)) {
        return 0;
    }

    uint32_t payload_length = ParseFrameHeader(buffer.data());
    size_t frame_size = FRAME_HEADER_SIZE + payload_length;
    frame.assign(buffer.begin(), buffer.begin() + frame_size);
    return frame_size;
}

/**
 * Extract payload from a frame
 * @param frame Complete frame (header + payload)
 * @return Payload portion of frame (empty if invalid)
 */
inline std::vector<uint8_t> ExtractPayload(const std::vector<uint8_t>& frame) {
    if (frame.size() < FRAME_HEADER_SIZE) {
        return {};  // Invalid frame
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"
    uint32_t payload_length = ParseFrameHeader(frame.data());
#pragma GCC diagnostic pop
    size_t expected_size = FRAME_HEADER_SIZE + payload_length;

    if (frame.size() != expected_size) {
        return {};  // Frame size mismatch
    }

    return std::vector<uint8_t>(frame.begin() + FRAME_HEADER_SIZE, frame.end());
}

} // namespace frame_parsing

/**
 * Test fixture for frame parsing
 */
class TestFrameParsing : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }
};

// ============================================================================
// Frame Header Packing Tests
// ============================================================================

TEST_F(TestFrameParsing, PackHeaderZeroLength) {
    // Test: Pack zero-length payload
    auto header = frame_parsing::PackFrameHeader(0);
    ASSERT_EQ(header.size(), 4);
    EXPECT_EQ(header[0], 0x00);
    EXPECT_EQ(header[1], 0x00);
    EXPECT_EQ(header[2], 0x00);
    EXPECT_EQ(header[3], 0x00);
}

TEST_F(TestFrameParsing, PackHeaderSmallLength) {
    // Test: Pack small payload length (256)
    auto header = frame_parsing::PackFrameHeader(256);
    ASSERT_EQ(header.size(), 4);
    EXPECT_EQ(header[0], 0x00);
    EXPECT_EQ(header[1], 0x00);
    EXPECT_EQ(header[2], 0x01);
    EXPECT_EQ(header[3], 0x00);
}

TEST_F(TestFrameParsing, PackHeaderLargeLength) {
    // Test: Pack large payload length (1 MB)
    uint32_t length = 1024 * 1024;
    auto header = frame_parsing::PackFrameHeader(length);
    ASSERT_EQ(header.size(), 4);
    EXPECT_EQ(header[0], 0x00);
    EXPECT_EQ(header[1], 0x10);
    EXPECT_EQ(header[2], 0x00);
    EXPECT_EQ(header[3], 0x00);
}

TEST_F(TestFrameParsing, PackHeaderMaxUint32) {
    // Test: Pack maximum uint32 value
    auto header = frame_parsing::PackFrameHeader(UINT32_MAX);
    ASSERT_EQ(header.size(), 4);
    EXPECT_EQ(header[0], 0xFF);
    EXPECT_EQ(header[1], 0xFF);
    EXPECT_EQ(header[2], 0xFF);
    EXPECT_EQ(header[3], 0xFF);
}

// ============================================================================
// Frame Header Parsing Tests
// ============================================================================

TEST_F(TestFrameParsing, ParseHeaderZeroLength) {
    // Test: Parse header with zero length
    uint8_t header_bytes[] = {0x00, 0x00, 0x00, 0x00};
    uint32_t length = frame_parsing::ParseFrameHeader(header_bytes);
    EXPECT_EQ(length, 0);
}

TEST_F(TestFrameParsing, ParseHeaderSmallLength) {
    // Test: Parse header with small length
    uint8_t header_bytes[] = {0x00, 0x00, 0x00, 0xFF};
    uint32_t length = frame_parsing::ParseFrameHeader(header_bytes);
    EXPECT_EQ(length, 255);
}

TEST_F(TestFrameParsing, ParseHeaderLargeLength) {
    // Test: Parse header with large length (1 MB)
    uint8_t header_bytes[] = {0x00, 0x10, 0x00, 0x00};
    uint32_t length = frame_parsing::ParseFrameHeader(header_bytes);
    EXPECT_EQ(length, 1024 * 1024);
}

TEST_F(TestFrameParsing, ParseHeaderMaxUint32) {
    // Test: Parse header with maximum value
    uint8_t header_bytes[] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t length = frame_parsing::ParseFrameHeader(header_bytes);
    EXPECT_EQ(length, UINT32_MAX);
}

// ============================================================================
// Header Roundtrip Tests
// ============================================================================

TEST_F(TestFrameParsing, HeaderRoundtripZero) {
    // Test: Pack and parse zero-length header
    uint32_t original = 0;
    auto packed = frame_parsing::PackFrameHeader(original);
    uint32_t parsed = frame_parsing::ParseFrameHeader(packed);
    EXPECT_EQ(parsed, original);
}

TEST_F(TestFrameParsing, HeaderRoundtripSmall) {
    // Test: Pack and parse small length
    uint32_t original = 256;
    auto packed = frame_parsing::PackFrameHeader(original);
    uint32_t parsed = frame_parsing::ParseFrameHeader(packed);
    EXPECT_EQ(parsed, original);
}

TEST_F(TestFrameParsing, HeaderRoundtripLarge) {
    // Test: Pack and parse large length (10 MB)
    uint32_t original = 10 * 1024 * 1024;
    auto packed = frame_parsing::PackFrameHeader(original);
    uint32_t parsed = frame_parsing::ParseFrameHeader(packed);
    EXPECT_EQ(parsed, original);
}

TEST_F(TestFrameParsing, HeaderRoundtripMaxUint32) {
    // Test: Pack and parse max uint32
    uint32_t original = UINT32_MAX;
    auto packed = frame_parsing::PackFrameHeader(original);
    uint32_t parsed = frame_parsing::ParseFrameHeader(packed);
    EXPECT_EQ(parsed, original);
}

TEST_F(TestFrameParsing, HeaderRoundtripSequential) {
    // Test: Multiple sequential roundtrips
    for (uint32_t i = 0; i < 100000; i += 10000) {
        auto packed = frame_parsing::PackFrameHeader(i);
        uint32_t parsed = frame_parsing::ParseFrameHeader(packed);
        EXPECT_EQ(parsed, i) 
            << "Roundtrip failed for length: " << i;
    }
}

// ============================================================================
// Frame Creation and Complete Frame Tests
// ============================================================================

TEST_F(TestFrameParsing, CreateFrameEmptyPayload) {
    // Test: Create frame with empty payload
    std::vector<uint8_t> payload;
    auto frame = frame_parsing::CreateFrame(payload);
    
    // Should have only header (4 bytes)
    EXPECT_EQ(frame.size(), 4);
    EXPECT_EQ(frame[0], 0x00);
    EXPECT_EQ(frame[1], 0x00);
    EXPECT_EQ(frame[2], 0x00);
    EXPECT_EQ(frame[3], 0x00);
}

TEST_F(TestFrameParsing, CreateFrameSmallPayload) {
    // Test: Create frame with small payload
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC};
    auto frame = frame_parsing::CreateFrame(payload);
    
    // Should have header (4) + payload (3) = 7 bytes
    EXPECT_EQ(frame.size(), 7);
    // Verify header encodes length 3
    EXPECT_EQ(frame[0], 0x00);
    EXPECT_EQ(frame[1], 0x00);
    EXPECT_EQ(frame[2], 0x00);
    EXPECT_EQ(frame[3], 0x03);
    // Verify payload
    EXPECT_EQ(frame[4], 0xAA);
    EXPECT_EQ(frame[5], 0xBB);
    EXPECT_EQ(frame[6], 0xCC);
}

TEST_F(TestFrameParsing, CreateFrameLargePayload) {
    // Test: Create frame with large payload
    std::vector<uint8_t> payload(1024, 0xFF);
    auto frame = frame_parsing::CreateFrame(payload);
    
    // Should have header + 1024 bytes
    EXPECT_EQ(frame.size(), 4 + 1024);
    // Verify header encodes length 1024
    uint32_t header_length = frame_parsing::ParseFrameHeader(frame);
    EXPECT_EQ(header_length, 1024);
}

// ============================================================================
// Complete Frame Detection Tests
// ============================================================================

TEST_F(TestFrameParsing, HasCompleteFrameEmptyBuffer) {
    // Test: Empty buffer has no complete frame
    std::vector<uint8_t> buffer;
    EXPECT_FALSE(frame_parsing::HasCompleteFrame(buffer));
}

TEST_F(TestFrameParsing, HasCompleteFramePartialHeader) {
    // Test: Partial header (only 2 bytes)
    std::vector<uint8_t> buffer = {0x00, 0x00};
    EXPECT_FALSE(frame_parsing::HasCompleteFrame(buffer));
}

TEST_F(TestFrameParsing, HasCompleteFrameCompleteHeaderOnly) {
    // Test: Complete header but zero-length payload (valid complete frame)
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x00};
    EXPECT_TRUE(frame_parsing::HasCompleteFrame(buffer));
}

TEST_F(TestFrameParsing, HasCompleteFrameHeaderPlusPartialPayload) {
    // Test: Header + partial payload
    std::vector<uint8_t> buffer = {
        0x00, 0x00, 0x00, 0x10,  // Length = 16
        0xAA, 0xBB               // Only 2 of 16 bytes
    };
    EXPECT_FALSE(frame_parsing::HasCompleteFrame(buffer));
}

TEST_F(TestFrameParsing, HasCompleteFrameComplete) {
    // Test: Complete frame
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC};
    auto frame = frame_parsing::CreateFrame(payload);
    EXPECT_TRUE(frame_parsing::HasCompleteFrame(frame));
}

TEST_F(TestFrameParsing, HasCompleteFrameExtraData) {
    // Test: Complete frame with extra data
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC};
    auto frame = frame_parsing::CreateFrame(payload);
    frame.push_back(0xFF);
    frame.push_back(0xFF);
    EXPECT_TRUE(frame_parsing::HasCompleteFrame(frame));
}

// ============================================================================
// Frame Extraction Tests
// ============================================================================

TEST_F(TestFrameParsing, ExtractFrameEmpty) {
    // Test: Extract from empty buffer
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> extracted;
    size_t size = frame_parsing::ExtractFrame(buffer, extracted);
    EXPECT_EQ(size, 0);
}

TEST_F(TestFrameParsing, ExtractFrameIncomplete) {
    // Test: Extract incomplete frame
    std::vector<uint8_t> buffer = {0x00, 0x00, 0x00, 0x10, 0xAA};  // Header says 16 bytes, only 1 present
    std::vector<uint8_t> extracted;
    size_t size = frame_parsing::ExtractFrame(buffer, extracted);
    EXPECT_EQ(size, 0);
}

TEST_F(TestFrameParsing, ExtractFrameComplete) {
    // Test: Extract complete frame
    std::vector<uint8_t> payload = {0x11, 0x22, 0x33};
    auto frame = frame_parsing::CreateFrame(payload);
    std::vector<uint8_t> extracted;
    size_t size = frame_parsing::ExtractFrame(frame, extracted);
    EXPECT_EQ(size, frame.size());
    EXPECT_EQ(extracted, frame);
}

TEST_F(TestFrameParsing, ExtractFrameMultiple) {
    // Test: Extract first frame from buffer with multiple frames
    std::vector<uint8_t> payload1 = {0xAA, 0xBB};
    std::vector<uint8_t> payload2 = {0xCC, 0xDD};
    auto frame1 = frame_parsing::CreateFrame(payload1);
    auto frame2 = frame_parsing::CreateFrame(payload2);
    
    std::vector<uint8_t> combined = frame1;
    combined.insert(combined.end(), frame2.begin(), frame2.end());
    
    std::vector<uint8_t> extracted;
    size_t extracted_size = frame_parsing::ExtractFrame(combined, extracted);
    EXPECT_EQ(extracted_size, frame1.size());
    EXPECT_EQ(extracted, frame1);
}

// ============================================================================
// Payload Extraction Tests
// ============================================================================

TEST_F(TestFrameParsing, ExtractPayloadEmpty) {
    // Test: Extract empty payload from frame
    std::vector<uint8_t> payload;
    auto frame = frame_parsing::CreateFrame(payload);
    auto extracted = frame_parsing::ExtractPayload(frame);
    EXPECT_EQ(extracted, payload);
}

TEST_F(TestFrameParsing, ExtractPayloadSmall) {
    // Test: Extract small payload
    std::vector<uint8_t> payload = {0x11, 0x22, 0x33};
    auto frame = frame_parsing::CreateFrame(payload);
    auto extracted = frame_parsing::ExtractPayload(frame);
    EXPECT_EQ(extracted, payload);
}

TEST_F(TestFrameParsing, ExtractPayloadLarge) {
    // Test: Extract large payload
    std::vector<uint8_t> payload(1024, 0xFF);
    auto frame = frame_parsing::CreateFrame(payload);
    auto extracted = frame_parsing::ExtractPayload(frame);
    EXPECT_EQ(extracted, payload);
}

TEST_F(TestFrameParsing, ExtractPayloadInvalidFrameSize) {
    // Test: Extract from frame with mismatched size
    std::vector<uint8_t> frame = {0x00, 0x00, 0x00, 0x10};  // Says 16 bytes
    // But frame is only 4 bytes
    auto extracted = frame_parsing::ExtractPayload(frame);
    EXPECT_TRUE(extracted.empty());
}

TEST_F(TestFrameParsing, ExtractPayloadInsufficientHeader) {
    // Test: Extract from frame too small for header
    std::vector<uint8_t> frame = {0x00, 0x00};  // Only 2 bytes
    auto extracted = frame_parsing::ExtractPayload(frame);
    EXPECT_TRUE(extracted.empty());
}

// ============================================================================
// Multiple Frame Processing Tests
// ============================================================================

TEST_F(TestFrameParsing, ProcessMultipleFramesSequential) {
    // Test: Process stream with multiple sequential frames
    std::vector<uint8_t> payloads[] = {
        {0x11},
        {0x22, 0x33},
        {0x44, 0x55, 0x66}
    };
    
    std::vector<uint8_t> stream;
    for (auto& payload : payloads) {
        auto frame = frame_parsing::CreateFrame(payload);
        stream.insert(stream.end(), frame.begin(), frame.end());
    }
    
    // Extract and verify each frame
    size_t offset = 0;
    for (int i = 0; i < 3; i++) {
        std::vector<uint8_t> buffer(stream.begin() + offset, stream.end());
        std::vector<uint8_t> extracted;
        size_t frame_size = frame_parsing::ExtractFrame(buffer, extracted);
        
        EXPECT_GT(frame_size, 0) << "Failed to extract frame " << i;
        auto payload = frame_parsing::ExtractPayload(extracted);
        EXPECT_EQ(payload, payloads[i]) 
            << "Payload mismatch at frame " << i;
        
        offset += frame_size;
    }
}

TEST_F(TestFrameParsing, ProcessFrameStreamWithPartialRead) {
    // Test: Receive frames in chunks, some partial
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC, 0xDD};
    auto complete_frame = frame_parsing::CreateFrame(payload);
    
    // Simulate partial receives: first 2 bytes, then rest
    std::vector<uint8_t> buffer;
    
    // Partial read 1: 2 bytes (incomplete)
    buffer.insert(buffer.end(), complete_frame.begin(), complete_frame.begin() + 2);
    EXPECT_FALSE(frame_parsing::HasCompleteFrame(buffer));
    
    // Partial read 2: add 4 more bytes (still incomplete)
    buffer.insert(buffer.end(), complete_frame.begin() + 2, complete_frame.begin() + 6);
    EXPECT_FALSE(frame_parsing::HasCompleteFrame(buffer));
    
    // Final read: add remaining bytes (now complete)
    buffer.insert(buffer.end(), complete_frame.begin() + 6, complete_frame.end());
    EXPECT_TRUE(frame_parsing::HasCompleteFrame(buffer));
    
    // Extract and verify
    std::vector<uint8_t> extracted;
    size_t size = frame_parsing::ExtractFrame(buffer, extracted);
    EXPECT_GT(size, 0);  // Verify extraction succeeded
    auto recovered = frame_parsing::ExtractPayload(extracted);
    EXPECT_EQ(recovered, payload);
}

// ============================================================================
// Frame Size Limit Tests
// ============================================================================

TEST_F(TestFrameParsing, FrameHeaderMaximumSize) {
    // Test: Frame with maximum possible size
    uint32_t max_size = UINT32_MAX;
    auto header = frame_parsing::PackFrameHeader(max_size);
    uint32_t parsed = frame_parsing::ParseFrameHeader(header);
    EXPECT_EQ(parsed, max_size);
}

TEST_F(TestFrameParsing, FramePayloadZeroSize) {
    // Test: Frame with zero-sized payload is valid
    std::vector<uint8_t> frame = frame_parsing::CreateFrame({});
    EXPECT_EQ(frame.size(), 4);  // Only header
    EXPECT_TRUE(frame_parsing::HasCompleteFrame(frame));
}

TEST_F(TestFrameParsing, FramePayloadKilobyte) {
    // Test: Frame with 1 KB payload
    std::vector<uint8_t> payload(1024, 0x42);
    auto frame = frame_parsing::CreateFrame(payload);
    EXPECT_EQ(frame.size(), 4 + 1024);
    auto recovered = frame_parsing::ExtractPayload(frame);
    EXPECT_EQ(recovered, payload);
}

TEST_F(TestFrameParsing, FramePayloadMegabyte) {
    // Test: Frame with 1 MB payload
    std::vector<uint8_t> payload(1024 * 1024, 0x55);
    auto frame = frame_parsing::CreateFrame(payload);
    EXPECT_EQ(frame.size(), 4 + 1024 * 1024);
    auto recovered = frame_parsing::ExtractPayload(frame);
    EXPECT_EQ(recovered, payload);
}
