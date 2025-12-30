# Unit Test Fixtures

This directory contains reusable test fixtures and helpers for C++ unit tests.

These fixtures are designed to:
- Reduce code duplication across tests
- Provide consistent test setup patterns
- Offer reusable mock and fake implementations
- Simplify common testing operations

## Files

### `mock_socket.h`
**Mock socket interface for testing client logic without actual network operations.**

Provides:
- `MockSocket` - Abstract interface for socket operations
- `MockSocketImpl` - GoogleMock implementation for mocking with expectations
- `FakeSocket` - Working implementation for deterministic testing

Key methods:
- `Send()` - Send data through socket
- `Receive()` - Receive data from socket
- `IsConnected()` - Check connection status
- `Connect()` - Establish connection
- `Disconnect()` - Close connection

Usage example:
```cpp
using testing::fixtures::MockSocketImpl;
MockSocketImpl mock;
EXPECT_CALL(mock, Send(_, _))
    .Times(AtLeast(1))
    .WillRepeatedly(Return(true));
```

### `protobuf_helpers.h`
**Helper utilities for protobuf message manipulation.**

Provides `ProtobufHelpers` class with:

**Factory Methods:**
- `CreateControlMessage()` - Create basic ControlMessage
- `CreateSetFpsMessage()` - Create SET_FPS message
- `CreateSetQualityMessage()` - Create SET_QUALITY message
- `CreateAliasMessage()` - Create ALIAS message
- `CreateFullMessage()` - Create message with all fields

**Serialization:**
- `SerializeMessage()` - Convert message to bytes
- `DeserializeMessage()` - Convert bytes to message
- `CreateLengthPrefixedFrame()` - Add length prefix (4-byte big-endian)
- `ParseLengthPrefixedFrame()` - Extract length and payload

**Boundary/Edge Cases:**
- `CreateMinimalMessage()` - Minimum valid payload
- `CreateMaximalMessage()` - Maximum string lengths
- `CreateBoundaryFpsMessage()` - Test FPS ranges
- `GenerateValidMessages()` - Create multiple varying messages

**Utilities:**
- `MessagesEqual()` - Deep equality comparison
- `MessageToString()` - Debug string representation
- `GetCurrentTimestampMs()` - Generate timestamps

Usage example:
```cpp
using testing::fixtures::ProtobufHelpers;
auto msg = ProtobufHelpers::CreateSetFpsMessage(30);
auto serialized = ProtobufHelpers::SerializeMessage(msg);
```

### `common_test_helpers.h`
**Common test utilities shared across all unit tests.**

Provides `TestHelpers` class with:

**Byte Order Conversion:**
- `HostToNetworkByteOrder()` - Convert 32-bit value to big-endian
- `HostToNetworkByteOrder16()` - Convert 16-bit value to big-endian
- `NetworkToHostByteOrder()` - Convert from big-endian
- `NetworkToHostByteOrder16()` - Convert 16-bit from big-endian

**Buffer Comparison:**
- `BuffersEqual()` - Compare two buffers
- `VectorsEqual()` - Compare two vectors
- `FindSubsequence()` - Find pattern in buffer

**Buffer Generation:**
- `CreateFilledBuffer()` - Create buffer filled with byte value
- `CreateSequentialBuffer()` - Create buffer with sequential bytes
- `CreatePseudoRandomBuffer()` - Generate pseudo-random buffer

**String/Debug Conversion:**
- `BufferToHexString()` - Convert buffer to hex string
- `VectorToHexString()` - Convert vector to hex string
- `BufferToString()` - Convert buffer to string

**Validation:**
- `BufferAllBytesEqual()` - Check if all bytes match value
- `IsValidUtf8()` - Validate UTF-8 encoding
- `IsInRange()` - Check if value is in range
- `ClampValue()` - Clamp value to range

Usage example:
```cpp
using testing::fixtures::TestHelpers;
auto buffer = TestHelpers::CreateSequentialBuffer(10);
auto hex = TestHelpers::BufferToHexString(buffer.data(), buffer.size());
EXPECT_TRUE(TestHelpers::IsValidUtf8(data, size));
```

## Design Principles

1. **No Assertions** - Fixtures provide helpers, not test logic
2. **Minimal Dependencies** - No Qt, no production code coupling
3. **Reusable** - Designed for use across multiple tests
4. **Explicit** - Clear intent, easy to understand
5. **Documented** - Well-commented with usage examples

## When to Use Fixtures

- ✅ Creating common test data
- ✅ Serialization/deserialization helpers
- ✅ Mock objects and fakes
- ✅ Buffer manipulation utilities
- ❌ Test assertions
- ❌ Test-specific logic
- ❌ Hidden behaviors

## Adding New Fixtures

1. Create new header file in `fixtures/`
2. Use namespace `testing::fixtures::`
3. Provide clear documentation with examples
4. Include only what's reusable across 3+ tests
5. No assertions or test logic
6. Keep dependencies minimal

## Related Documentation

- See `../smoke/` for example usage in smoke tests
- See `../protocol/` for protocol-specific tests
- See `../parsing/` for data structure tests

