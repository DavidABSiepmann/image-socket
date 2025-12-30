# Testing Infrastructure

## Overview

The project testing infrastructure is organized into three categories, each with its own framework and scope:

1. **C++ Standard Library / Boost Tests** (`tests/unit/`)
2. **Qt C++ Tests** (`tests/qt/`)
3. **QML Tests** (`tests/qml/`)

## Directory Structure

```
tests/
├── unit/                          # Pure C++ unit tests (GoogleTest)
│   ├── protocol/                  # Protobuf serialization/validation
│   ├── parsing/                   # Data structure parsing
│   ├── client/                    # Client logic (state machine, backoff)
│   ├── smoke/                     # Smoke tests (linkage, availability)
│   └── fixtures/                  # Reusable test helpers
├── qt/                            # Qt-based integration tests (QtTest)
│   ├── websocket/                 # WebSocket connectivity tests
│   ├── state/                     # State machine behavior tests
│   ├── signals/                   # Signal emission tests
│   ├── models/                    # Model and property tests
│   ├── smoke/                     # Qt smoke tests
│   └── fixtures/                  # Reusable Qt test helpers
├── qml/                           # QML component tests (QtTest + QQmlEngine)
│   ├── components/                # QML component functionality
│   ├── integration/               # QML + C++ backend integration
│   └── fixtures/                  # Test-only QML components
└── CMakeLists.txt                 # Test infrastructure root
```

## Building Tests

```bash
cd build_new
cmake ..
make -j4
ctest -V                          # Run all tests with verbose output
```

## Running Tests by Category

```bash
# Run all C++ unit tests
ctest -R "^unit_" -V

# Run all Qt tests
ctest -R "^qt_" -V

# Run all QML tests
ctest -R "^qml_" -V

# Run specific test
ctest -R "test_name" -V
```

## Test Naming Conventions

### C++ Unit Tests (`tests/unit/`)
- File: `test_<feature>_<aspect>.cpp`
- Test class: `Test<Feature><Aspect>`
- Test method: `test<Behavior>`
- CMake target: `unit_protocol_*`, `unit_parsing_*`, etc.

### Qt Tests (`tests/qt/`)
- File: `test_<feature>_<aspect>.cpp`
- Framework: QtTest with QTEST_MAIN
- CMake target: `qt_websocket_*`, `qt_state_*`, etc.

### QML Tests (`tests/qml/`)
- File: `test_<feature>_<aspect>.cpp`
- Framework: QtTest + QQmlEngine
- CMake target: `qml_component_*`, `qml_integration_*`, etc.

## Test Categories Details

### C++ Unit Tests (GoogleTest)

#### Protocol Tests
- **Location:** `tests/unit/protocol/`
- **Framework:** GoogleTest
- **Scope:** Protobuf serialization, validation, message types
- **Dependencies:** C++ Standard Library, Protobuf only

#### Parsing Tests
- **Location:** `tests/unit/parsing/`
- **Framework:** GoogleTest
- **Scope:** Data parsing, byte order conversion, frame headers
- **Dependencies:** C++ Standard Library only

#### Client Logic Tests
- **Location:** `tests/unit/client/`
- **Framework:** GoogleTest + Gmock
- **Scope:** Reconnection logic, state machines, mocked async operations
- **Dependencies:** C++ Standard Library, Boost::asio (inspection only)

#### Smoke Tests
- **Location:** `tests/unit/smoke/`
- **Framework:** GoogleTest
- **Scope:** Library linkage, symbol availability, basic construction
- **Dependencies:** C++ Standard Library, minimal

### Qt Tests (QtTest)

#### WebSocket Tests
- **Location:** `tests/qt/websocket/`
- **Scope:** Client/server connectivity, control messages, concurrent clients
- **Dependencies:** Qt5 Core, WebSockets, Test, Protobuf

#### State Tests
- **Location:** `tests/qt/state/`
- **Scope:** State machine transitions, property persistence
- **Dependencies:** Qt5 Core, Test

#### Signal Tests
- **Location:** `tests/qt/signals/`
- **Scope:** Signal emissions, parameter verification, event codes
- **Dependencies:** Qt5 Core, WebSockets, Test

#### Model Tests
- **Location:** `tests/qt/models/`
- **Scope:** Model row management, property updates, FPS calculations
- **Dependencies:** Qt5 Core, WebSockets, Test, Gui

#### Smoke Tests
- **Location:** `tests/qt/smoke/`
- **Scope:** Basic Qt connectivity, initialization
- **Dependencies:** Qt5 Core, Test

### QML Tests (QtTest + QQmlEngine)

#### Component Tests
- **Location:** `tests/qml/components/`
- **Scope:** QML component behavior, animations, visibility
- **Dependencies:** Qt5 Core, Qml, Quick, Test

#### Integration Tests
- **Location:** `tests/qml/integration/`
- **Scope:** QML + C++ backend integration, signal propagation
- **Dependencies:** Qt5 Core, Qml, Quick, Test, Protobuf, WebSockets

## Adding New Tests

### For C++ Unit Tests:

1. Create file in appropriate subdirectory under `tests/unit/`
2. Follow GoogleTest structure:
   ```cpp
   #include <gtest/gtest.h>
   
   class Test<Feature><Aspect> : public ::testing::Test {
   protected:
       void SetUp() override { }
   };
   
   TEST_F(Test<Feature><Aspect>, test<Behavior>) {
       // Test implementation
   }
   ```
3. Update `tests/unit/CMakeLists.txt` with `add_executable()` and `add_test()`

### For Qt Tests:

1. Create file in appropriate subdirectory under `tests/qt/`
2. Follow QtTest structure:
   ```cpp
   #include <QtTest>
   
   class Test<Feature><Aspect> : public QObject {
       Q_OBJECT
   private slots:
       void test<Behavior>();
   };
   
   QTEST_MAIN(Test<Feature><Aspect>)
   #include "test_file.moc"
   ```
3. Update relevant `tests/qt/*/CMakeLists.txt` with `add_executable()` and `add_test()`

### For QML Tests:

1. Create file in appropriate subdirectory under `tests/qml/`
2. Use QQmlEngine to load QML components
3. Verify component behavior with QTest assertions
4. Update `tests/qml/CMakeLists.txt` with `add_executable()` and `add_test()`

## CMake Integration

The infrastructure uses FetchContent to automatically download and build GoogleTest (v1.14.0) for C++ unit tests. This ensures consistent testing across all environments.

Key CMakeLists.txt files:
- **Root:** `/home/david/Projetos/image-socket/CMakeLists.txt` - Enables testing
- **Tests root:** `tests/CMakeLists.txt` - Coordinates subdirectories
- **Unit tests:** `tests/unit/CMakeLists.txt` - GoogleTest setup
- **Qt tests:** `tests/qt/CMakeLists.txt` - Qt framework setup
- **QML tests:** `tests/qml/CMakeLists.txt` - Qt QML setup

## Best Practices

1. **Keep tests focused** - One assertion per test when possible
2. **Use fixtures** - Reuse common setup in `tests/*/fixtures/`
3. **Mock external dependencies** - Use Gmock for async operations
4. **Test behavior, not implementation** - Avoid testing internals
5. **Clean up resources** - Use tearDown/destructors properly
6. **Run tests frequently** - Integrate with CI/CD pipeline
7. **Document test purpose** - Add comments explaining what and why

## Future Considerations

- Coverage reports with `gcov`
- Benchmark tests for critical paths
- Fuzz testing for protocol parsing (libFuzzer)
- Integration test suite for end-to-end scenarios
