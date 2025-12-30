#ifndef PROTOBUF_HELPERS_H
#define PROTOBUF_HELPERS_H

#include <string>
#include <vector>
#include <cstdint>
#include "control.pb.h"

/**
 * @file protobuf_helpers.h
 * @brief Helper utilities for protobuf message testing
 *
 * This fixture provides utilities for:
 * - Creating valid protobuf messages
 * - Serializing/deserializing messages
 * - Generating boundary payloads
 * - Message validation helpers
 *
 * Usage:
 *   ProtobufHelpers helpers;
 *   auto msg = helpers.CreateControlMessage(SET_FPS, 30);
 *   auto serialized = helpers.SerializeMessage(msg);
 */

namespace testing {
namespace fixtures {

/**
 * ProtobufHelpers: Utilities for protobuf message manipulation
 * 
 * Provides factory methods and serialization helpers to avoid
 * duplicating message setup logic across multiple tests.
 */
class ProtobufHelpers {
public:
    ProtobufHelpers() = default;
    ~ProtobufHelpers() = default;

    // -----------------------------------------------------------------------
    // Factory methods for creating common message types
    // -----------------------------------------------------------------------

    /**
     * @brief Create a basic ControlMessage with a command type
     * @param type CommandType (e.g., SET_FPS, PAUSE, RESUME)
     * @return ControlMessage with type set, other fields at defaults
     */
    static imagesocket::control::ControlMessage CreateControlMessage(
        imagesocket::control::CommandType type) {
        imagesocket::control::ControlMessage msg;
        msg.set_type(type);
        return msg;
    }

    /**
     * @brief Create a ControlMessage with command type and client ID
     * @param type CommandType
     * @param client_id Client identifier
     * @return ControlMessage with type and client_id set
     */
    static imagesocket::control::ControlMessage CreateControlMessage(
        imagesocket::control::CommandType type,
        int32_t client_id) {
        auto msg = CreateControlMessage(type);
        msg.set_client_id(client_id);
        return msg;
    }

    /**
     * @brief Create a SET_FPS control message
     * @param fps Frames per second value
     * @param client_id Optional client identifier (defaults to 0)
     * @return ControlMessage with type=SET_FPS and fps value
     */
    static imagesocket::control::ControlMessage CreateSetFpsMessage(
        int32_t fps,
        int32_t client_id = 0) {
        auto msg = CreateControlMessage(imagesocket::control::SET_FPS, client_id);
        msg.set_fps(fps);
        return msg;
    }

    /**
     * @brief Create a SET_QUALITY control message
     * @param quality Quality value (typically 0-100)
     * @param client_id Optional client identifier
     * @return ControlMessage with type=SET_QUALITY and quality value
     */
    static imagesocket::control::ControlMessage CreateSetQualityMessage(
        int32_t quality,
        int32_t client_id = 0) {
        auto msg = CreateControlMessage(imagesocket::control::SET_QUALITY, client_id);
        msg.set_quality(quality);
        return msg;
    }

    /**
     * @brief Create an ALIAS control message
     * @param alias Human-friendly alias string
     * @param client_id Optional client identifier
     * @return ControlMessage with type=ALIAS and alias string
     */
    static imagesocket::control::ControlMessage CreateAliasMessage(
        const std::string& alias,
        int32_t client_id = 0) {
        auto msg = CreateControlMessage(imagesocket::control::ALIAS, client_id);
        msg.set_alias(alias);
        return msg;
    }

    /**
     * @brief Create a message with all fields populated
     * @param type CommandType
     * @param client_id Client identifier
     * @param fps FPS value
     * @param quality Quality value
     * @param alias Alias string
     * @param reason Reason string
     * @return Fully populated ControlMessage
     */
    static imagesocket::control::ControlMessage CreateFullMessage(
        imagesocket::control::CommandType type,
        int32_t client_id,
        int32_t fps,
        int32_t quality,
        const std::string& alias,
        const std::string& reason) {
        auto msg = CreateControlMessage(type, client_id);
        msg.set_fps(fps);
        msg.set_quality(quality);
        msg.set_alias(alias);
        msg.set_reason(reason);
        msg.set_timestamp_ms(GetCurrentTimestampMs());
        return msg;
    }

    // -----------------------------------------------------------------------
    // Serialization and deserialization helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Serialize a ControlMessage to bytes
     * @param msg Message to serialize
     * @return Serialized message as vector of bytes
     */
    static std::vector<uint8_t> SerializeMessage(
        const imagesocket::control::ControlMessage& msg) {
        std::string serialized;
        msg.SerializeToString(&serialized);
        return std::vector<uint8_t>(serialized.begin(), serialized.end());
    }

    /**
     * @brief Deserialize bytes into a ControlMessage
     * @param data Pointer to serialized data
     * @param size Size of serialized data
     * @param out_msg Output message (populated on success)
     * @return true if deserialization succeeded, false otherwise
     */
    static bool DeserializeMessage(
        const uint8_t* data,
        size_t size,
        imagesocket::control::ControlMessage& out_msg) {
        std::string serialized(reinterpret_cast<const char*>(data), size);
        return out_msg.ParseFromString(serialized);
    }

    /**
     * @brief Serialize a message and create a length-prefixed frame
     * 
     * Creates a frame with 4-byte big-endian length prefix followed by payload.
     * Format: [4 bytes: length] [payload]
     * 
     * @param msg Message to serialize
     * @return Length-prefixed frame as vector of bytes
     */
    static std::vector<uint8_t> CreateLengthPrefixedFrame(
        const imagesocket::control::ControlMessage& msg) {
        auto payload = SerializeMessage(msg);
        
        // Create frame: 4-byte length (big-endian) + payload
        std::vector<uint8_t> frame;
        uint32_t length = static_cast<uint32_t>(payload.size());
        
        // Add length as big-endian
        frame.push_back((length >> 24) & 0xFF);
        frame.push_back((length >> 16) & 0xFF);
        frame.push_back((length >> 8) & 0xFF);
        frame.push_back(length & 0xFF);
        
        // Add payload
        frame.insert(frame.end(), payload.begin(), payload.end());
        
        return frame;
    }

    /**
     * @brief Parse a length-prefixed frame
     * 
     * Extracts the 4-byte length and payload from a frame.
     * 
     * @param frame Pointer to frame data
     * @param frame_size Total frame size (including length prefix)
     * @param out_length Output: extracted length value
     * @param out_payload Output: extracted payload
     * @return true if frame is valid, false otherwise
     */
    static bool ParseLengthPrefixedFrame(
        const uint8_t* frame,
        size_t frame_size,
        uint32_t& out_length,
        std::vector<uint8_t>& out_payload) {
        if (frame_size < 4) {
            return false;  // Frame too small for length prefix
        }
        
        // Extract big-endian length
        out_length = (frame[0] << 24) | (frame[1] << 16) | 
                     (frame[2] << 8) | frame[3];
        
        size_t payload_size = frame_size - 4;
        if (out_length != payload_size) {
            return false;  // Length mismatch
        }
        
        out_payload.assign(frame + 4, frame + frame_size);
        return true;
    }

    // -----------------------------------------------------------------------
    // Boundary and edge case payload generators
    // -----------------------------------------------------------------------

    /**
     * @brief Create a message with minimum valid payload
     * @return ControlMessage with only type field set
     */
    static imagesocket::control::ControlMessage CreateMinimalMessage() {
        return CreateControlMessage(imagesocket::control::UNKNOWN);
    }

    /**
     * @brief Create a message with maximum string lengths
     * @return ControlMessage with long alias and reason strings
     */
    static imagesocket::control::ControlMessage CreateMaximalMessage() {
        std::string long_string(1000, 'A');  // 1000-character string
        return CreateFullMessage(
            imagesocket::control::SET_FPS,
            999999,           // Max int32
            60,
            100,
            long_string,      // Long alias
            long_string       // Long reason
        );
    }

    /**
     * @brief Create a message with boundary FPS values
     * @param fps FPS value to test
     * @return ControlMessage with given FPS
     */
    static imagesocket::control::ControlMessage CreateBoundaryFpsMessage(int32_t fps) {
        return CreateSetFpsMessage(fps);
    }

    /**
     * @brief Generate a series of valid messages
     * @param count Number of messages to generate
     * @return Vector of ControlMessages with varying command types
     */
    static std::vector<imagesocket::control::ControlMessage> GenerateValidMessages(
        size_t count) {
        std::vector<imagesocket::control::ControlMessage> messages;
        
        imagesocket::control::CommandType types[] = {
            imagesocket::control::PAUSE,
            imagesocket::control::RESUME,
            imagesocket::control::SET_FPS,
            imagesocket::control::SET_QUALITY,
            imagesocket::control::ALIAS
        };
        
        for (size_t i = 0; i < count; ++i) {
            auto type = types[i % 5];
            auto msg = CreateControlMessage(type, static_cast<int32_t>(i));
            if (type == imagesocket::control::SET_FPS) {
                msg.set_fps(30 + (i % 60));
            }
            messages.push_back(msg);
        }
        
        return messages;
    }

    // -----------------------------------------------------------------------
    // Utility functions
    // -----------------------------------------------------------------------

    /**
     * @brief Get current timestamp in milliseconds
     * @return Current time as int64_t milliseconds since epoch
     */
    static int64_t GetCurrentTimestampMs() {
        // Simple implementation: for testing only
        // In production, use proper timestamp library
        static int64_t counter = 1000000;
        return counter++;
    }

    /**
     * @brief Compare two ControlMessages for equality
     * @param a First message
     * @param b Second message
     * @return true if messages are equal, false otherwise
     */
    static bool MessagesEqual(
        const imagesocket::control::ControlMessage& a,
        const imagesocket::control::ControlMessage& b) {
        return a.type() == b.type() &&
               a.client_id() == b.client_id() &&
               a.fps() == b.fps() &&
               a.quality() == b.quality() &&
               a.reason() == b.reason() &&
               a.alias() == b.alias() &&
               a.timestamp_ms() == b.timestamp_ms();
    }

    /**
     * @brief Create a debug string representation of a message
     * @param msg Message to format
     * @return Human-readable string representation
     */
    static std::string MessageToString(const imagesocket::control::ControlMessage& msg) {
        std::string result = "ControlMessage{";
        result += "type=" + std::to_string(msg.type());
        result += ", client_id=" + std::to_string(msg.client_id());
        result += ", fps=" + std::to_string(msg.fps());
        result += ", quality=" + std::to_string(msg.quality());
        result += ", alias=" + msg.alias();
        result += ", reason=" + msg.reason();
        result += "}";
        return result;
    }
};

}  // namespace fixtures
}  // namespace testing

#endif  // PROTOBUF_HELPERS_H
