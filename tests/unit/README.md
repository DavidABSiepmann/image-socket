# Unit Tests - Complete Test Suite

Comprehensive unit test suite for image-socket following TESTS_CPP_STD.md guidelines.
**Total: 327+ tests across 16 targets, 100% passing**

## Test Categories

### 1. Smoke Tests (19 tests) - Entry Point
Quick sanity checks verifying build environment and basic functionality:
- Library linkage and symbols
- Protobuf enums
- Object construction
- Test fixture infrastructure

**Directory:** `smoke/`
**Run:** `ctest -R "^smoke_"`

### 2. Protocol Tests (49 tests) - Message Handling
Protocol-level functionality with Protobuf messages:
- Message serialization/deserialization
- Field validation
- Payload size limits
- Message type discrimination

**Directory:** `protocol/`
**Run:** `ctest -R "^unit_protocol_"`

### 3. Parsing Tests (148 tests) - Data Conversion
Data parsing and conversion algorithms:
- Configuration parsing
- Byte order conversion (host ↔ network)
- Message framing (length-prefixed)
- State transition validation

**Directory:** `parsing/`
**Run:** `ctest -R "^unit_parsing_"`

### 4. Client Logic Tests (111 tests) - Business Logic
Pure C++ client logic without I/O or networking:
- Exponential backoff calculation
- Connection state machine
- Configuration accumulation
- Error callback propagation (mocked)

**Directory:** `client/`
**Run:** `ctest -R "^unit_client_"`

## Comprehensive Test Commands

```bash
# Run all unit tests
cd build && ctest -R "^(smoke_|unit_)" -V

# Run only basic tests (smoke + unit)
ctest -R "^(smoke_|unit_)" --output-on-failure

# Run specific categories
ctest -R "^smoke_"            # Smoke tests only
ctest -R "^unit_protocol_"    # Protocol tests only
ctest -R "^unit_parsing_"     # Parsing tests only
ctest -R "^unit_client_"      # Client tests only

# Run with verbose output
ctest -R "^unit_" -VV
```

## Testing Framework

- **GoogleTest (gtest)** v1.14.0 - Test framework and assertions
- **GoogleMock (gmock)** - Callback and interface mocking
- **Protocol Buffers v3** (libprotobuf-lite) - Message serialization
- **CMake 3.5+** - Build system with FetchContent integration
- **C++ Standard Library** - Core dependencies only

## Key Design Principles

1. **Isolated Logic:** No real I/O, networking, or threading in unit tests
2. **Deterministic:** All tests are reproducible with no timing dependencies
3. **Fast Execution:** Complete suite runs in < 100ms
4. **No Side Effects:** Tests don't modify system state
5. **Clear Patterns:** Table-driven tests, fixtures, and named assertions
6. **Header-Only Utilities:** Reusable test helpers without compilation overhead

## Test File Structure

```
tests/unit/
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── fixtures/                   # Reusable test helpers
│   ├── common_test_helpers.h   # Byte order, buffer utilities
│   ├── protobuf_helpers.h      # Message factories, serialization
│   └── mock_socket.h           # Mock socket interface
├── smoke/                      # Entry point sanity checks
│   ├── test_smoke_linkage.cpp
│   ├── test_smoke_enums.cpp
│   ├── test_smoke_construction.cpp
│   ├── test_smoke_fixtures.cpp
│   └── README.md
├── protocol/                   # Protocol message tests
│   ├── test_protobuf_serialization.cpp
│   ├── test_protobuf_validation.cpp
│   ├── test_payload_size_limits.cpp
│   ├── test_message_type_discrimination.cpp
│   └── README.md
├── parsing/                    # Data parsing tests
│   ├── test_configuration_parsing.cpp
│   ├── test_byte_order.cpp
│   ├── test_frame_parsing.cpp
│   ├── test_state_transitions.cpp
│   └── README.md
└── client/                     # Client logic tests
    ├── test_reconnect_backoff.cpp
    ├── test_client_state_machine.cpp
    ├── test_configuration_accumulation.cpp
    ├── test_error_callback.cpp
    └── README.md
```

## Build Configuration

### Dependencies
- CMake 3.5+
- C++14 compatible compiler
- protobuf compiler (for message generation)

### Targets Generated
- **Smoke:** smoke_linkage, smoke_enums, smoke_construction, smoke_fixtures
- **Protocol:** unit_protocol_serialization, unit_protocol_validation, unit_protocol_size_limits, unit_protocol_type_discrimination
- **Parsing:** unit_parsing_configuration, unit_parsing_byte_order, unit_parsing_frames, unit_parsing_state_transitions
- **Client:** unit_client_backoff, unit_client_state_machine, unit_client_accumulation, unit_client_error_callback

### Building
```bash
cd /path/to/image-socket
mkdir -p build && cd build
cmake ..
make -j4

# Or build specific test
make unit_client_backoff
```

## Test Coverage Summary

| Category | Tests | Status | Coverage |
|----------|-------|--------|----------|
| Smoke    | 19    | ✓ Pass | Build/linkage |
| Protocol | 49    | ✓ Pass | Message handling |
| Parsing  | 148   | ✓ Pass | Data conversion |
| Client   | 111   | ✓ Pass | Business logic |
| **TOTAL**| **327+**| **✓ 100%** | **Comprehensive** |

## Guidelines for Adding Tests

When adding new unit tests:

1. **Choose the right directory:**
   - smoke/ - Build/linkage verification
   - protocol/ - Protobuf message handling
   - parsing/ - Data parsing/conversion
   - client/ - Business logic

2. **Follow naming conventions:**
   - File: `test_<category>_<topic>.cpp`
   - Fixture: `Test<Topic>` (inherit from `::testing::Test`)
   - Methods: `TestFixture.TestName`

3. **Use test patterns:**
   - Table-driven tests for parameter validation
   - Roundtrip tests for conversions
   - Determinism tests for algorithms
   - Boundary value tests for limits

4. **Maintain isolation:**
   - No real I/O or networking
   - No threading or timing dependencies
   - No global state modification
   - Use mocks for interfaces (GoogleMock)

5. **Update CMakeLists.txt:**
   - Add new executable target
   - Link with appropriate libraries
   - Register with add_test()

## Continuous Integration

Tests are designed to run in CI pipelines:
- Fast execution (< 100ms total)
- No external dependencies
- Deterministic results
- Clear pass/fail status
- Verbose output on failure

```bash
# CI pipeline example
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
ctest -R "^(smoke_|unit_)" --output-on-failure
```

## Troubleshooting

### Build Issues
- Ensure CMake >= 3.5: cmake version 3.22.1 `cmake --version`
- Check protobuf compiler: libprotoc 3.12.4 `protoc --version`
- Verify C++14 support: g++ (Ubuntu 11.4.0-1ubuntu1~22.04.2) 11.4.0 `g++ --version`

### Test Failures
- Run with verbose output: `ctest -VV`
- Check compilation warnings
- Verify no system side effects
- Use GDB for debugging: `gdb ./build/tests/unit/<test_name>`

### Performance Issues
- Ensure no real I/O operations
- Check for timing-dependent tests
- Verify mock objects aren't real
- Profile with: `time ctest -R "^unit_"`

## References

- [TESTS_CPP_STD.md](../../docs/TESTS_CPP_STD.md) - Test standards and guidelines
- [GoogleTest Documentation](https://google.github.io/googletest/)
- [GoogleMock Documentation](https://google.github.io/googlemock/)
- [Protocol Buffers Guide](https://developers.google.com/protocol-buffers)