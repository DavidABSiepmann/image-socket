# Testing Infrastructure: Pure C++ (std/Boost) Unit Tests

## 1. Purpose

This document defines the testing strategy for **pure C++ components** that depend only on the C++ Standard Library and Boost, with **no Qt, QML, or GUI dependencies**.

These are the fastest, most isolated, and easiest-to-maintain unit tests in the project.

**See also:** [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) — Qt backend / integration tests, and [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md) — QML/UI integration tests.

---

## 2. Scope

### What to test:
- **Protocol logic**: Protobuf serialization/deserialization (control messages, frame headers)
- **Data structures**: Parsing, encoding, decoding, validation
- **Helper utilities**: String conversion, byte order operations, configuration parsing
- **Client logic** (pure C++ aspects): 
  - Connection state machine (isolated, without actual socket operations)
  - Reconnection backoff calculation
  - Image encoding/JPEG handling (with mocked I/O)
  - Thread-safe queue operations (if used)
- **Smoke tests**: Basic API availability and entry point validation

### What NOT to test:
- Actual socket I/O operations (use Qt backend integration tests instead — see `testing_infrastructure_qt_backend.md`)
- Threading behavior and event-loop dependent code (use Qt integration tests with event loops)
- OpenCV image processing (assumed to work; test only custom wrapping or glue code)
- Any code that depends on `<QObject>`, `<QWebSocket>`, Qt types, or QML (these belong to Qt/QML test suites — see `testing_infrastructure_qt_qml.md`)

**Separation of concerns:** Keep these unit tests focused on pure algorithmic and protocol logic; do not duplicate tests that belong in integration or UI layers.

---

## 3. Test Categories

### 3.1 Protocol Tests (`unit_protocol_*`)
**Location:** `tests/unit/protocol/`

**What to test:**
- Protobuf message serialization (serialize → deserialize roundtrip)
- Protobuf message validation (required fields, ranges)
- Payload size limits enforcement (e.g., MAX_PAYLOAD constant)
- Message type discrimination

**Notes:**
- Prefer testing the Protobuf messages and framing logic here; end-to-end networking should be covered by Qt backend integration tests (see `testing_infrastructure_qt_backend.md`).
- Reference the control schema: [`docs/protobuf_control_protocol.md`](../docs/protobuf_control_protocol.md).

**Example test cases:**
```
test_protobuf_control_message_roundtrip
  → ControlMessage.set_fps(30) → Serialize → Parse → Verify fps == 30

test_protobuf_message_validation
  → Verify required fields cause parse errors if missing

test_protobuf_unknown_fields_ignored
  → Add unknown field → Serialize → Parse in older binary → Verify still valid
```

**Dependencies:**
- `protobuf::libprotobuf` or `protobuf::libprotobuf-lite` (link against the lite library when using the lite runtime)
- C++ Standard Library only

**Framework:**
- GoogleTest — prefer imported targets `GTest::gtest` and `GTest::gtest_main` for modern CMake usage
- Use GoogleMock (`gmock`) when mocking pure C++ interfaces is necessary
- Keep tests deterministic and fast (ideally <100ms per test) 

---

### 3.2 Data Structure & Parsing Tests (`unit_parsing_*`)
**Location:** `tests/unit/parsing/`

**What to test:**
- Configuration parsing (e.g., from `config.h` constants)
- Byte order conversion (host ↔ network byte order)
- Length-prefixed message framing logic (frame header validation)
- State transition validation (e.g., enum state changes)

**Example test cases:**
```
test_big_endian_roundtrip
  → Pack uint32 as big-endian → Unpack → Verify original value

test_frame_header_parsing
  → Create frame header (4-byte length + payload) → Parse → Verify length matches

test_state_enum_ranges
  → Verify all enum states in control.proto map correctly
```

**Dependencies:**
- C++ Standard Library only

**Framework:**
- GoogleTest (gtest)

---

### 3.3 Client Logic Tests (`unit_client_*`)
**Location:** `tests/unit/client/`

**What to test:**
- Reconnection backoff calculation (exponential backoff without actual sleep)
- Connection state machine transitions (isolated, no actual socket)
- Configuration accumulation logic
- Error callback mechanism (mocked)

**Example test cases:**
```
test_reconnect_backoff_exponential
  → Attempt 1 (0ms) → 2 (100ms) → 3 (200ms) → Verify exponential growth

test_client_state_transitions
  → Create ImageSocketClient → Verify initial state is Disconnected
  → Call connect() on mock socket → Verify state changes to Connecting

test_client_error_callback_propagation
  → Mock error callback → Trigger error → Verify callback invoked with correct error
```

**Dependencies:**
- C++ Standard Library
- Boost::asio (for client code inspection, but avoid actual socket binding)

**Framework:**
- GoogleTest (gtest)
- **Gmock** (for mocking async operations)

---

### 3.4 Smoke Tests (`smoke_*`)
**Location:** `tests/unit/smoke/`

**Purpose:** Quick, lightweight sanity checks that the basic APIs and dependencies are available.

**What to test:**
- Library linkage and symbol availability
- Basic object construction (no-op constructors)
- Enum constant availability
- Include path correctness

**Example test cases:**
```
test_imagesocket_lib_linkage
  → Verify ImageSocketClient can be instantiated

test_protobuf_enums_defined
  → Verify control.proto enum values are accessible

test_include_paths_resolvable
  → Verify headers can be included without errors
```

**Dependencies:**
- Headers only

**Framework:**
- GoogleTest (gtest) — minimal assertions

---

## 4. Test Structure & Naming

### File organization:
```
tests/unit/
├── CMakeLists.txt
├── protocol/
│   ├── test_protobuf_serialization.cpp
│   ├── test_protobuf_validation.cpp    
│   └── test_control_message_formats.cpp
├── parsing/
│   ├── test_byte_order_conversion.cpp  
│   └── test_frame_header_parsing.cpp   
├── client/
│   ├── test_reconnect_backoff.cpp      
│   └── test_client_state_machine.cpp   
├── smoke/
│   ├── test_linkage.cpp                
│   └── test_enums.cpp                  
└── fixtures/
    ├── mock_socket.h
    └── protobuf_helpers.h
```

### Naming conventions:
- File: `test_<feature>_<aspect>.cpp`
- Test class: `Test<Feature><Aspect>` (e.g., `TestProtobufSerialization`)
- Test method: `test<behavior>` (e.g., `testRoundtripPreservesData`)

### CMake integration:
```cmake
# tests/unit/CMakeLists.txt
enable_testing()
find_package(GTest REQUIRED)
find_package(Protobuf REQUIRED)

# Protobuf tests
add_executable(unit_protocol_serialization protocol/test_protobuf_serialization.cpp)
# Prefer linking libprotobuf-lite when using the lite runtime
target_link_libraries(unit_protocol_serialization PRIVATE imagesocket GTest::gtest GTest::gtest_main protobuf::libprotobuf-lite)
add_test(NAME unit_protocol_serialization COMMAND unit_protocol_serialization)

# Similar for other tests...
```

Note: If `protobuf::libprotobuf-lite` is not available in your environment, fall back to `protobuf::libprotobuf`.

---

## 5. Minimal Test Checklist

For each new pure C++ test:

- [ ] No `#include <Q*.h>` headers
- [ ] No Qt signals, slots, or events
- [ ] No actual socket or file I/O (mock if needed)
- [ ] No threading (use isolated state tests or gmock async)
- [ ] Assertions use `ASSERT_*` / `EXPECT_*` from GoogleTest
- [ ] Test is deterministic (no flakiness)
- [ ] Test runs in < 100 ms
- [ ] CMakeLists.txt entry created with `add_test(NAME unit_* ...)`
- [ ] Test name starts with `unit_` prefix
- [ ] Fixtures placed in `tests/unit/fixtures/` if reusable

---

## 6. Test Execution

### Run all C++ unit tests:
```bash
cd build
ctest -R "^unit_" -V
```

### Run specific category:
```bash
ctest -R "^unit_protocol_" -V
ctest -R "^smoke_" -V
```

### With verbose output:
```bash
ctest -R "^unit_" --output-on-failure
```

---

## 7. Example: Adding a New Test

### Scenario: Add test for message validation

**File:** `tests/unit/protocol/test_protobuf_validation.cpp`

```cpp
#include <gtest/gtest.h>
#include "control.pb.h"

class TestProtobufValidation : public ::testing::Test {
protected:
    void SetUp() override {
        // Per-test setup
    }
};

TEST_F(TestProtobufValidation, RequiredFieldMissing) {
    imagesocket::control::ControlMessage msg;
    // client_id is required; leave unset
    
    std::string serialized;
    EXPECT_TRUE(msg.SerializeToString(&serialized));  // Protobuf allows serialize
    
    imagesocket::control::ControlMessage parsed;
    EXPECT_TRUE(parsed.ParseFromString(serialized));
    
    // Verify missing required field has default value
    EXPECT_EQ(parsed.client_id(), 0);  // Default if unset
}

TEST_F(TestProtobufValidation, FpsRangeValidation) {
    imagesocket::control::ControlMessage msg;
    msg.set_fps(1000);  // Arbitrary high value
    
    std::string serialized;
    ASSERT_TRUE(msg.SerializeToString(&serialized));
    
    imagesocket::control::ControlMessage parsed;
    ASSERT_TRUE(parsed.ParseFromString(serialized));
    
    EXPECT_EQ(parsed.fps(), 1000);
}
```

**Update CMakeLists.txt:**
```cmake
add_executable(unit_protocol_validation protocol/test_protobuf_validation.cpp)
target_link_libraries(unit_protocol_validation PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_protocol_validation COMMAND unit_protocol_validation)
```

---

## 8. What to Avoid

- ❌ Testing Qt-specific behavior (signals, slots, event loops) in this layer
- ❌ Testing actual networking (use Qt integration tests instead)
- ❌ Testing OpenCV processing (assume correctness; test only custom wrappers)
- ❌ Complex mocking that defeats the purpose of unit testing
- ❌ Tests that depend on external resources (files, network, timing)
- ❌ Tests that cannot run in parallel

---

## 9. Integration with Main Build

**In top-level `CMakeLists.txt`:**
```cmake
add_subdirectory(tests)
enable_testing()
```

**In `tests/CMakeLists.txt`:**
```cmake
add_subdirectory(unit)
add_subdirectory(qt)
add_subdirectory(qml)
```

This ensures `ctest` can discover all tests from the build root:
```bash
cd build && ctest -R "^unit_"
```

