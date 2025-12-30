# Parsing Tests

Unit tests for data parsing, conversion, and validation logic testing isolated algorithms without I/O or networking.
**Total: 148 tests, 100% passing**

## Test Files

### test_configuration_parsing.cpp (39 tests)
Validates FPS, quality, and client ID parsing and validation:
- FPS validation: range 1-240, rejection of invalid values
- Quality validation: range 0-100, rejection of invalid values
- Client ID validation: positive integers, rejection of zero/negative
- Table-driven tests for multiple parameter combinations
- Boundary value testing (min, max, off-by-one)
- Error handling for out-of-range values
- Default value assignment

### test_byte_order.cpp (33 tests)
Validates byte order conversion (host ↔ network byte order):
- Host-to-network (hton) conversion for 16-bit, 32-bit, 64-bit values
- Network-to-host (ntoh) conversion for 16-bit, 32-bit, 64-bit values
- Roundtrip verification: `value == ntoh(hton(value))`
- Deterministic behavior across multiple invocations
- Edge cases: 0, max values, single bit patterns
- Endianness preservation in conversions

### test_frame_parsing.cpp (37 tests)
Validates length-prefixed message framing logic:
- Frame header parsing (4-byte length prefix)
- Payload extraction from frames
- Incomplete frame detection
- Multiple frames in buffer handling
- Frame boundary validation
- Zero-length frame handling
- Maximum frame size enforcement
- Concatenated frame processing

### test_state_transitions.cpp (39 tests)
Validates connection state transition logic:
- State enumeration: Disconnected, Connecting, Connected, Disconnecting
- Valid transitions: Disc→Conn→Connected→Disconnecting→Disc
- Invalid transition detection
- State persistence through transitions
- Error recovery (early disconnect from connecting state)
- State to enum and string conversion

## Framework
- GoogleTest (gtest) v1.14.0
- C++ Standard Library only
- No external networking or file I/O dependencies

## Build Configuration
- CMake targets: unit_parsing_configuration, unit_parsing_byte_order, unit_parsing_frames, unit_parsing_state_transitions
- Linked with: GTest::gtest, GTest::gtest_main (unit_parsing_state_transitions also links imagesocket)
- Test discovery: `ctest -R "^unit_parsing_"`

## Naming Convention
test_*.cpp
