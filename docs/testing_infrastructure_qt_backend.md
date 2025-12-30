# Testing Infrastructure: Qt + C++ Component Tests

## 1. Purpose

This document defines the testing strategy for **Qt-dependent C++ components** that use QtCore, QtNetwork, QtWebSockets, and QtGui, but **do not include QML or UI logic**.

These tests validate signals, state transitions, event loop behavior, and integration between C++ and Qt's asynchronous mechanisms.

**Architecture reference:** See [`ARCHITECTURE.md`](ARCHITECTURE.md) for component design and [`DECISIONS.md`](DECISIONS.md) for testing strategy rationale.

**See also:**
- [`testing_infrastructure_std.md`](testing_infrastructure_std.md) — Pure C++ / Protobuf unit tests
- [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md) — QML / UI component tests

---

## 2. Scope

### What to test:
- **State machines**: `ImageServerBridge`, `ClientSession` state transitions with signal validation
- **Signal emissions**: Correct timing and parameters of Qt signals
- **Event loop behavior**: Async operations, timers, deferred cleanup (deleteLater)
- **Qt model integration**: `ClientModel` item count, role data, signals
- **WebSocket communication**: Connection handling, message framing (end-to-end, not mocked)
- **Image provider integration**: QML image provider availability and caching
- **Client sessions**: Creation, destruction, client connection/disconnection

### What NOT to test:
- QML UI logic (use [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md) instead)
- Layout, styling, visual appearance (QML tests handle this)
- OpenCV image processing (assume correctness; test only custom wrappers)
- Threading beyond Qt's event loop (e.g., custom worker threads) — Qt is single-threaded with signal/slot dispatch
- Pure C++ algorithmic logic without Qt dependencies (use [`testing_infrastructure_std.md`](testing_infrastructure_std.md) instead)

**Important:** These tests run in Qt's event loop; long-running or blocking operations will hang the test. Always use `QTRY_*` macros and event loop spins instead of `qWait()` or `sleep()`.

---

## 3. Test Categories

### 3.1 State Transition Tests (`qt_state_*`)
**Location:** `tests/qt/state/`

**What to test:**
- `ImageServerBridge` states: Idle → Starting → Running → Stopping → Idle
- Connection states: NoClients → ClientsConnected → ReceivingFrames
- Signal emissions at each transition (state changed, connection changed, status message)
- Correct event codes (e.g., `ServerStarted`, `ClientConnected`)

**Example test cases:**
```
test_server_bridge_initial_state
  → Create ImageServerBridge → Verify serverState() == Idle
  → Verify connectionState() == NoClients

test_server_start_transitions
  → Start server → Capture stateChanged signals
  → Verify: Idle → Starting → Running (or direct Idle → Running)
  → Verify eventOccurred(ServerStarted) emitted

test_client_connect_state_transition
  → Connect client to running server → Capture signals
  → Verify: NoClients → ClientsConnected

test_client_disconnect_state_transition
  → Disconnect client → Verify: ClientsConnected → NoClients
  → Verify eventOccurred(ClientDisconnected) emitted
```

**Dependencies:**
- Qt5::Core, Qt5::WebSockets, Qt5::Network
- Qt5::Test for signal introspection (`QSignalSpy`, `QTRY_*` macros, assertions)

**Framework:**
- QtTest — built-in Qt testing framework
- Signal capture via `QSignalSpy` (preferred over manual signal tracking)
- Event loop control via `QTRY_*` macros and `QEventLoop`

**Lifetime management:**
- Test objects should inherit from `QObject` and properly manage Qt object tree
- Use `QSignalSpy` to capture signals before triggering state changes
- Always ensure event loop processes pending events (use `QCoreApplication::processEvents()` in cleanup)

---

### 3.2 Signal & Slot Tests (`qt_signals_*`)
**Location:** `tests/qt/signals/`

**What to test:**
- Signal parameters are correct (e.g., client ID, FPS value)
- Signals are emitted at the right time (not too early, not too late)
- Slot responses trigger expected side effects
- QSignalSpy usage to verify signal counts and arguments

**Example test cases:**
```
test_status_message_signal
  → Set statusMessage("New message") → Capture statusMessageChanged signal
  → Verify signal emitted exactly once with correct message

test_fps_change_signal_parameters
  → Send SET_FPS(24) control message → Capture signal
  → Verify signal contains: clientId, newFps=24

test_info_overlay_signals
  → Connect to server → Verify activeClientAlias signal
  → Send frame → Verify currentFps signal with correct value

test_client_signals_on_reconnect
  → Client connects → Disconnect -> Reconnect
  → Verify clientConnected signal emitted both times
```

**Dependencies:**
- Qt5::Core, Qt5::WebSockets
- QtTest for `QSignalSpy`

**Framework:**
- QtTest with `QSignalSpy`

---

### 3.3 Qt Model Tests (`qt_models_*`)
**Location:** `tests/qt/models/`

**What to test:**
- `ClientModel` row count after client connect/disconnect
- Model role data (client alias, status, FPS)
- Model signals (`rowsInserted`, `rowsRemoved`, `dataChanged`)
- Model data update on property changes

**Example test cases:**
```
test_client_model_insert_row
  → Create ClientModel → Connect client
  → Verify rowCount() == 1
  → Verify data(index(0), ClientAliasRole) == "Alice"

test_client_model_remove_row
  → Connect client → Disconnect
  → Verify rowCount() == 0
  → Verify rowsRemoved signal emitted

test_client_model_role_data_updates
  → Connect client → Update FPS property
  → Verify dataChanged signal emitted
  → Verify data(index(0), CurrentFpsRole) reflects new value
```

**Dependencies:**
- Qt5::Core, Qt5::Gui (for QAbstractListModel)
- QtTest

**Framework:**
- QtTest with `QSignalSpy`

---

### 3.4 WebSocket Integration Tests (`qt_websocket_*`)
**Location:** `tests/qt/websocket/`

**What to test:**
- Server accepts WebSocket connections from actual `QWebSocket` clients
- Client sends binary messages (control messages as per [`protobuf_control_protocol.md`](protobuf_control_protocol.md))
- Server receives, parses (Protobuf), and dispatches messages correctly
- Multiple concurrent clients handled correctly
- Client connection cleanup (deleteLater) — connections should be dropped gracefully
- Message framing (0x01 prefix for control, 0x00 for image)

**Event loop requirements:**
- Connections are asynchronous; use `QTRY_*` macros to wait for signals
- Do NOT use `qWait()` or blocking operations
- Each test should spin the event loop long enough for socket handshake (~100–500 ms timeout)
- Ensure `processEvents()` is called in cleanup to honor `deleteLater()` callbacks

**Example test cases:**
```
test_websocket_server_accept_client
  → Start WebSocketServer → Connect QWebSocket
  → Verify clientConnected signal emitted
  → Verify port is bound

test_websocket_send_receive_message
  → Connect client → Send ControlMessage (binary)
  → Verify server emits controlMessageReceived with parsed message
  → Verify FPS value preserved

test_multiple_clients_concurrent
  → Start server → Connect 3 clients simultaneously
  → Verify all clientConnected signals emitted
  → Send message from each → Verify server receives all

test_client_disconnect_cleanup
  → Connect client → Disconnect
  → Verify clientDisconnected signal emitted
  → Verify socket cleaned up (deleteLater honored)
```

**Dependencies:**
- Qt5::Core, Qt5::Network, Qt5::WebSockets
- Qt5::Test (for `QSignalSpy`, `QTRY_*`, assertions)
- Protobuf (link `protobuf::libprotobuf-lite` for control message parsing — see [`protobuf_control_protocol.md`](protobuf_control_protocol.md))

**Framework:**
- QtTest with event loop spin (`QEventLoop`, `QTRY_VERIFY_WITH_TIMEOUT`, `QTRY_COMPARE_WITH_TIMEOUT`)
- `QSignalSpy` for connection and message events
- Manual event loop management only when needed; prefer `QTRY_*` macros

---

### 3.5 Smoking Tests (`qt_smoke_*`)
**Location:** `tests/qt/smoke/`

**Purpose:** Lightweight checks that Qt components initialize correctly and key dependencies are linked.

**What to test:**
- `ImageServerBridge` can be instantiated
- `ClientSession` can be created without segfault
- `ClientModel` can be created and set as QML context
- Image provider can be registered with QML engine
- Qt event loop is available

**Example test cases:**
```
test_image_server_bridge_instantiation
  → Create ImageServerBridge → Verify not null
  → Verify serverState() accessible

test_client_model_instantiation
  → Create ClientModel → Verify rowCount() == 0

test_qml_image_provider_registration
  → Create ImageServerBridge → Register image provider
  → Verify provider accessible from QML engine
```

**Framework:**
- QtTest

---

## 4. Test Structure & Naming

### File organization:
```
tests/qt/
├── CMakeLists.txt
├── state/
│   ├── test_server_state_transitions.cpp
│   ├── test_connection_state_transitions.cpp
│   └── test_state_recovery.cpp
├── signals/
│   ├── test_status_message_signal.cpp
│   ├── test_fps_signals.cpp
│   └── test_client_signals.cpp
├── models/
│   ├── test_client_model_rows.cpp
│   └── test_model_role_data.cpp
├── websocket/
│   ├── test_websocket_server_basic.cpp
│   ├── test_client_sessions.cpp
│   ├── test_multiple_clients.cpp
│   └── test_client_cleanup.cpp
├── smoke/
│   ├── test_component_instantiation.cpp
│   └── test_dependencies.cpp
└── fixtures/
    ├── qt_test_base.h
    └── mock_client.h
```

### Naming conventions:
- File: `test_<component>_<aspect>.cpp`
- Test class: `Test<Component><Aspect>` (inherits from `QObject`)
- Test method: `test<behavior>()` (decorated with `private slots:`)

### CMake integration:
```cmake
# tests/qt/CMakeLists.txt
find_package(Qt5 COMPONENTS Core Network WebSockets Test Gui REQUIRED)
find_package(Protobuf REQUIRED)  # For WebSocket tests

# State transition tests
add_executable(qt_state_server_transitions state/test_server_state_transitions.cpp)
set_target_properties(qt_state_server_transitions PROPERTIES AUTOMOC ON)  # Enable moc for QObject
target_link_libraries(qt_state_server_transitions PRIVATE 
    imagesocket Qt5::Core Qt5::WebSockets Qt5::Test)
add_test(NAME qt_state_server_transitions COMMAND qt_state_server_transitions)

# WebSocket tests (with Protobuf)
add_executable(qt_websocket_server websocket/test_websocket_server_basic.cpp)
set_target_properties(qt_websocket_server PROPERTIES AUTOMOC ON)
target_link_libraries(qt_websocket_server PRIVATE 
    imagesocket Qt5::Core Qt5::Network Qt5::WebSockets Qt5::Test protobuf::libprotobuf-lite)
add_test(NAME qt_websocket_server COMMAND qt_websocket_server)

# Similar for other test categories...
```

**Note:** `AUTOMOC ON` is essential for test classes inheriting from `QObject`; it auto-generates `.moc` files.

---

## 5. Qt Testing Best Practices

### 5.0 Fixture-based Architecture

This project uses a **fixture-based** test structure (base classes in `tests/qt/fixtures/`) to reduce boilerplate and ensure consistent setup/teardown:

```cpp
// tests/qt/fixtures/qt_test_base.h
class QtTestBase : public QObject {
    Q_OBJECT
protected:
    void setUp() { /* initialize common test objects */ }
    void tearDown() { /* cleanup, process deferred deletions */ }
};
```

Derived test classes inherit from `QtTestBase` and reuse common setup, ensuring:
- Event loop is properly initialized
- Qt object tree is clean between tests
- `deleteLater()` calls are honored (via `processEvents()`)

See `tests/qt/fixtures/` directory for base classes and mocks.

### 5.1 QSignalSpy Usage
```cpp
QSignalSpy spy(&bridge, &ImageServerBridge::serverStateChanged);

bridge.start();

// Wait for signal and verify count
QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 1000);

// Extract signal arguments
QVariantList args = spy.takeFirst();
ImageServerBridge::ServerState newState = 
    args.at(0).value<ImageServerBridge::ServerState>();
QCOMPARE(newState, ImageServerBridge::Running);
```

### 5.2 Event Loop Spinning
```cpp
QEventLoop loop;

// Connect signal to quit loop
QObject::connect(&client, &QWebSocket::connected, &loop, &QEventLoop::quit);

// Open connection
client.open(QUrl("ws://127.0.0.1:5000"));

// Spin event loop until signal or timeout
QTimer::singleShot(1000, &loop, &QEventLoop::quit);
loop.exec();

// Verify connection was established
QVERIFY(client.isValid());
```

### 5.3 Timeout-aware Verification (Event Loop Spinning)

**Always prefer `QTRY_*` macros over `qWait()` or manual loops:**

```cpp
// QTRY_* macros automatically spin the event loop until the condition is true or timeout
QTRY_VERIFY_WITH_TIMEOUT(server.isListening(), 1000);  // 1000 ms timeout

// Or with explicit comparison
QTRY_COMPARE_WITH_TIMEOUT(model.rowCount(), 1, 2000);  // Wait for rowCount to become 1

// For signals
QSignalSpy spy(&client, &QWebSocket::connected);
client.open(QUrl("ws://127.0.0.1:5000"));
QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 1000);  // Wait for connected signal
```

**Why NOT `qWait()`:**
- `qWait(100)` blocks the entire event loop → asynchronous signals won't fire
- `QTRY_*` spins the event loop, processing pending signals and timers
- Tests become slower and flakier with `qWait()`

### 5.4 Resource Cleanup and Lifetime Management

**Qt object lifetime is deferred:** Objects marked with `deleteLater()` are not immediately freed; they're queued for deletion when the event loop processes.

```cpp
class TestComponent : public QtTestBase {  // Inherits from fixture
    Q_OBJECT
private slots:
    void cleanup() override {
        // CRITICAL: Process deferred deletions (deleteLater calls)
        QCoreApplication::instance()->processEvents();
        // NOW it's safe to verify no leaked resources
    }
};
```

**Common lifetime pitfalls:**
- ❌ Creating a `QWebSocket`, disconnecting, and immediately checking if it's destroyed → it won't be (queued for deletion)
- ✅ Disconnect → `processEvents()` → check destroyed state
- ❌ Assuming parent-child relationships are freed automatically in test destructor → they're not
- ✅ Use `QSignalSpy` to verify `destroyed()` signal after `processEvents()`

---

## 6. Minimal Test Checklist

For each new Qt + C++ test:

- [ ] Test class inherits from `QObject` (not `QWidget` or `QMainWindow`)
- [ ] Test methods are `private slots:`
- [ ] `AUTOMOC ON` set in CMake for this target
- [ ] Signals captured with `QSignalSpy` (not manual tracking)
- [ ] Event loop spinned with `QTRY_*` macros (not `qWait()`)
- [ ] No blocking operations (use event loop instead)
- [ ] All Qt objects deleted or parentage clear (via Qt object tree)
- [ ] Test is deterministic (can run in any order, no global state)
- [ ] Test runs in < 500 ms (may need to increase for slow CI)
- [ ] Assertions use `QVERIFY`, `QCOMPARE`, `QTRY_*` from QtTest
- [ ] CMakeLists.txt entry created with `add_test(NAME qt_* ...)`
- [ ] Test name starts with `qt_` prefix
- [ ] Shared fixtures placed in `tests/qt/fixtures/` if reusable

---

## 7. Test Execution

### Run all Qt tests:
```bash
cd build
ctest -R "^qt_" -V
```

### Run specific category:
```bash
ctest -R "^qt_state_" -V
ctest -R "^qt_websocket_" -V
```

### Run with verbose output and stop on failure:
```bash
ctest -R "^qt_" --output-on-failure --stop-on-failure
```

---

## 8. Example: Adding a New Test

### Scenario: Test ClientModel row insertion

**File:** `tests/qt/models/test_client_model_rows.cpp`

```cpp
#include <QtTest/QtTest>
#include <QSignalSpy>
#include "clientmodel.h"
#include "clientsession.h"

class TestClientModelRows : public QObject {
    Q_OBJECT

private slots:
    void testRowInsertOnClientConnect() {
        ClientModel model;
        QCOMPARE(model.rowCount(), 0);

        QSignalSpy spyRowsInserted(&model, &QAbstractListModel::rowsInserted);

        // Simulate client connection
        ClientSession session("client-123");
        session.setAlias("Alice");
        
        // (Assuming model has a slot that receives client sessions)
        model.addClient(&session);

        // Verify row count increased
        QCOMPARE(model.rowCount(), 1);

        // Verify rowsInserted signal emitted
        QTRY_COMPARE_WITH_TIMEOUT(spyRowsInserted.count(), 1, 500);

        // Verify data is accessible
        QVariant alias = model.data(model.index(0, 0), Qt::DisplayRole);
        QCOMPARE(alias.toString(), QString("Alice"));
    }

    void testRowRemoveOnClientDisconnect() {
        ClientModel model;
        
        ClientSession session("client-123");
        model.addClient(&session);
        QCOMPARE(model.rowCount(), 1);

        QSignalSpy spyRowsRemoved(&model, &QAbstractListModel::rowsRemoved);

        // Simulate disconnection
        model.removeClient("client-123");

        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(spyRowsRemoved.count(), 1);
    }
};

QTEST_MAIN(TestClientModelRows)
#include "test_client_model_rows.moc"
```

**Update CMakeLists.txt:**
```cmake
add_executable(qt_models_rows models/test_client_model_rows.cpp)
set_target_properties(qt_models_rows PROPERTIES AUTOMOC ON)
target_link_libraries(qt_models_rows PRIVATE 
    imagesocket Qt5::Core Qt5::Gui Qt5::Test)
add_test(NAME qt_models_rows COMMAND qt_models_rows)
```

---

## 9. What to Avoid

- ❌ Testing QML bindings or UI logic (use QML tests instead)
- ❌ Testing layout, styling, visual appearance
- ❌ Blocking calls like `sleep()` or `usleep()` (use QTRY_* instead)
- ❌ Global state or static variables shared between tests
- ❌ Direct socket I/O without event loop (use Qt classes instead)
- ❌ Assuming signal order across tests (signals can be queued/deferred)
- ❌ Memory leaks (ensure Qt object ownership is clear)
- ❌ Tests that hardcode port numbers (use `port == 0` for OS assignment)

---

## 10. Integration with Main Build

**In `tests/qt/CMakeLists.txt`:**
```cmake
find_package(Qt5 COMPONENTS Core Network WebSockets Test Gui REQUIRED)

# Set AUTOMOC globally for all targets in this directory
set(CMAKE_AUTOMOC ON)

# Add subdirectories with add_subdirectory(), or define tests inline

# State tests
add_executable(qt_state_server ... )
# ...

enable_testing()
```

**In top-level `tests/CMakeLists.txt`:**
```cmake
add_subdirectory(qt)
```

---

## 11. Debugging Failed Tests

### Verbose output:
```bash
ctest -R "^qt_" -VV  # Very verbose, shows all output
```

### Run single test with output:
```bash
./build/qt_state_server_transitions -v2  # Qt built-in verbose flag
```

### Attach debugger:
```bash
gdb ./build/qt_state_server_transitions
(gdb) run
```
