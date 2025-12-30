# Protocol Tests

Unit tests for protocol-level functionality using Protocol Buffers testing message handling without real I/O or networking.
**Total: 49 tests, 100% passing**

## Test Files

### test_protobuf_serialization.cpp (11 tests)
Validates Protobuf message serialization and deserialization:
- Message creation with all fields set
- Serialization to byte buffer
- Deserialization from byte buffer
- Round-trip verification: `message == deserialize(serialize(message))`
- Field value preservation through serialization
- Binary data integrity
- Empty message handling

### test_protobuf_validation.cpp (16 tests)
Validates Protobuf message field requirements and constraints:
- CommandType enum values validation (PAUSE, RESUME, ID, REQUEST_RESUME, SET_FPS, SET_QUALITY, etc.)
- Client ID field validation
- FPS field range validation
- Quality field range validation
- Timestamp and reason field validation
- Alias field validation
- Required field enforcement
- Default value assignment

### test_payload_size_limits.cpp (11 tests)
Validates message payload size enforcement:
- Message serialization size calculation
- Maximum payload size limits
- Small message handling (< 1KB)
- Medium message handling (1-10KB)
- Large message handling (10-100KB)
- Boundary cases at size limits
- Deterministic serialization size

### test_message_type_discrimination.cpp (11 tests)
Validates correct message type identification and handling:
- CommandType enum discrimination (PAUSE vs RESUME vs SET_FPS, etc.)
- Correct message creation by type
- Type field preservation
- Type-specific field requirements
- Wrong type detection
- Unknown type handling
- Type equality comparison

## Framework
- GoogleTest (gtest) v1.14.0
- Protobuf v3 (libprotobuf-lite)
- C++ Standard Library only
- Generated protobuf code from control.proto

## Build Configuration
- CMake targets: unit_protocol_serialization, unit_protocol_validation, unit_protocol_size_limits, unit_protocol_type_discrimination
- Linked with: imagesocket, GTest::gtest, GTest::gtest_main
- Dependencies: Generated protobuf headers from ${CMAKE_BINARY_DIR}/src/imagesocket/generated
- Test discovery: `ctest -R "^unit_protocol_"`

## Naming Convention
test_*.cpp
