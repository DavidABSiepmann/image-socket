# Qt Test Fixtures

Reusable header-only test fixtures and utilities for Qt + C++ component testing following TESTS_QT_CPP.md guidelines.

**Total: 5 fixture files, ~1500 lines of helper code**

## Fixture Files Overview

| File | Lines | Purpose | Used By |
|------|-------|---------|---------|
| qt_test_base.h | 290 | Core Qt utilities (app, event loop, signals) | All categories |
| test_websocket_server.h | 296 | Test WebSocket server with multi-client support | websocket, models, smoke |
| test_websocket_client.h | 249 | Test WebSocket client for server simulation | websocket, state, smoke |
| test_model_helper.h | 248 | QAbstractListModel testing utilities | models, smoke |
| test_signal_helper.h | 422 | Signal introspection and waiting | signals, state, websocket |

---

## 1. qt_test_base.h

**Core utilities for QtTest-based tests**

### Classes:
- **QtTestApplicationGuard** - RAII for QCoreApplication initialization
- **EventLoopSpinner** - Non-blocking event loop control with timeout
- **SignalWaiter** - Wait for signals without blocking threads
- **TimerGuard** - RAII wrapper for QTimer cleanup

### Key Functions:
- `initializeQtTestApp()` - One-time Qt application setup
- `EventLoopSpinner::processEvents()` - Deterministic event processing
- `EventLoopSpinner::processEventsWithTimeout(ms)` - Time-limited event loop

### Usage Example:
```cpp
qt_test::initializeQtTestApp();

qt_test::SignalWaiter waiter(&obj, &Class::signal, 1);
obj.triggerSignal();
if (waiter.wait(5000)) {
    // Signal emitted within 5 seconds
}
```

---

## 2. test_websocket_server.h

**Lightweight WebSocket server for testing clients**

### Class: WebSocketServerFixture

**Signals:**
- `clientConnected(QString clientId)` - New client connected
- `clientDisconnected(QString clientId)` - Client disconnected
- `messageReceived(QString clientId, QByteArray data)` - Binary message received
- `textMessageReceived(QString clientId, QString text)` - Text message received

**Key Methods:**
- `start()` - Start listening on specified port
- `stop()` - Stop server and close all clients
- `port()` - Get actual listening port (useful after port 0 auto-assignment)
- `serverUrl()` - Get full WS URL for clients
- `isListening()` - Check if server is running
- `clientCount()` - Get number of connected clients
- `sendToClient(id, data)` - Send message to specific client
- `broadcast(data)` - Broadcast to all clients
- `connectedClients()` - Get list of client IDs

**Features:**
- Automatic port selection (pass 0 for OS assignment)
- Multiple concurrent client support
- Binary and text message handling
- Signal-based client notifications
- RAII cleanup on destruction

### Usage Example:
```cpp
WebSocketServerFixture server(0);  // Auto-assign port
server.start();
QVERIFY(server.isListening());

// Access via: server.serverUrl(), server.port()
// Listen for: server.clientConnected, server.messageReceived
```

---

## 3. test_websocket_client.h

**Lightweight WebSocket client for server interaction testing**

### Class: WebSocketClientFixture

**Signals:**
- `connected()` - Connected to server
- `disconnected()` - Disconnected from server
- `binaryMessageReceived(QByteArray data)` - Binary message received
- `textMessageReceived(QString text)` - Text message received
- `errorOccurred(QString error)` - Connection error

**Key Methods:**
- `connect(url)` - Connect to server (non-blocking)
- `disconnect()` - Close connection
- `sendBinaryMessage(data)` - Send binary data
- `sendTextMessage(text)` - Send text data
- `isConnected()` - Check connection status
- `url()` - Get current server URL
- `lastError()` - Get error message

**Features:**
- Non-blocking connection (uses Qt signals, not threads)
- Binary and text message support
- Error reporting
- RAII cleanup

### Usage Example:
```cpp
WebSocketClientFixture client;
qt_test::SignalWaiter waiter(&client, &WebSocketClientFixture::connected, 1);
client.connect("ws://127.0.0.1:5000");
QVERIFY(waiter.wait(2000));
```

---

## 4. test_model_helper.h

**Utilities for QAbstractListModel testing**

### Class: ModelTestHelper

**Data Access:**
- `rowCount()` - Get current row count
- `roleValue(row, role)` - Get single role value
- `roleValues(row, roles)` - Get multiple role values
- `isValidRow(row)` - Check row validity
- `index(row)` - Get QModelIndex

**Signal Parsing (static methods):**
- `rowsInsertedCount(signal_args)` - Count rows from signal
- `rowsRemovedCount(signal_args)` - Count removed rows
- `getFirstRowFromSignal(args)` - Extract first row index
- `getLastRowFromSignal(args)` - Extract last row index
- `inspectDataChangedSignal(args, ...)` - Parse dataChanged signal

**Spy Factory Methods:**
- `createRowsInsertedSpy()` - Create spy for rowsInserted
- `createRowsRemovedSpy()` - Create spy for rowsRemoved
- `createDataChangedSpy()` - Create spy for dataChanged

**Features:**
- Non-assertion helper (test code adds assertions)
- Role extraction with validation
- Signal argument parsing
- Convenient spy creation

### Usage Example:
```cpp
ModelTestHelper helper(model);
QCOMPARE(helper.rowCount(), 0);

// After adding item...
QCOMPARE(helper.rowCount(), 1);

QVariant alias = helper.roleValue(0, AliasRole);
QCOMPARE(alias.toString(), "Alice");
```

---

## 5. test_signal_helper.h

**Utilities for signal-based testing**

### Class: SignalHelper

**Emission Checks:**
- `emissionCount()` - Total number of emissions
- `neverEmitted()` - Check if never emitted
- `wasEmitted()` - Check if emitted at least once
- `emittedExactly(n)` - Check for exact count
- `emittedAtLeast(n)` - Check minimum count
- `clear()` - Reset spy

**Argument Access:**
- `argumentAt(emission, arg)` - Get specific argument
- `argumentsAt(emission)` - Get all args for emission
- `lastArguments()` / `lastArgument()` - Last emission args
- `firstArgument()` - First emission's first arg
- `argumentAs<T>(e, a)` - Type-safe argument access

**Waiting (Non-blocking):**
- `waitForEmission(timeout, min_count)` - Wait for N emissions
- `waitForAdditionalEmissions(count, timeout)` - Wait for more

**History:**
- `allEmissions()` - Get all emissions as list
- `consistentArgumentCount()` - Check arg consistency
- `argumentCount()` - Expected argument count

### Class: MultiSignalHelper

**Track multiple signals:**
- `add(sender, signal, name)` - Register signal to track
- `count(name)` - Get emission count
- `argumentsAt(name, emission)` - Get signal arguments
- `clearAll()` / `clear(name)` - Reset spies
- `names()` - List tracked signal names

**Features:**
- Non-assertion helper (test code adds assertions)
- Type-safe argument conversion
- Non-blocking wait (QEventLoop-based)
- Signal ordering inspection
- Full history tracking

### Usage Example:
```cpp
qt_test::SignalHelper helper(&obj, &Class::valueChanged);
obj.setValue(42);
QCOMPARE(helper.emissionCount(), 1);
QCOMPARE(helper.firstArgumentAs<int>(), 42);
```

---

## Design Principles

### ✓ What Fixtures Do
- Manage Qt object lifetimes (RAII)
- Provide non-blocking event loop control
- Extract and format signal data
- Handle WebSocket communication safely
- Avoid hardcoded values (auto port assignment)
- Compose helpers instead of inheritance
- Expose signal/data for test assertions

### ✗ What Fixtures Don't Do
- Make assertions (test code does that)
- Hide test intent with complex abstractions
- Use sleep() or busy-wait loops
- Modify production code
- Hardcode port numbers
- Maintain global/static state
- Contain test-specific logic

---

## Including in Tests

```cpp
#include <QtTest/QtTest>
#include "qt_test_base.h"
#include "test_signal_helper.h"
#include "test_websocket_server.h"

class TestMyComponent : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }
    
    void test_signal_emission() {
        qt_test::SignalWaiter waiter(&obj, &Class::signal, 1);
        // ...
    }
};

QTEST_MAIN(TestMyComponent)
#include "test_my_component.moc"
```

---

## CMake Integration

Fixtures are header-only - include them directly in test files:

```cmake
# tests/qt/websocket/CMakeLists.txt

add_executable(qt_websocket_basic websocket/test_websocket_server_basic.cpp)
set_target_properties(qt_websocket_basic PROPERTIES AUTOMOC ON)
target_include_directories(qt_websocket_basic PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../fixtures)
target_link_libraries(qt_websocket_basic PRIVATE 
    imagesocket Qt5::Core Qt5::WebSockets Qt5::Test)
add_test(NAME qt_websocket_basic COMMAND qt_websocket_basic)
```

---

## Key Features Summary

| Feature | qt_test_base | websocket | websocket | model | signal |
|---------|---|---|---|---|---|
| App lifecycle | ✓ | | | | |
| Event loop | ✓ | | | | |
| Signal waiting | ✓ | | | | |
| Server fixture | | ✓ | | | |
| Client fixture | | | ✓ | | |
| Model utilities | | | | ✓ | |
| Signal helpers | | | | | ✓ |
| Multi-signal | | | | | ✓ |

---

## Testing the Fixtures

All fixtures:
- ✓ Compile with Qt5 (Core, Network, WebSockets, Test, Gui)
- ✓ Header-only (zero compilation artifacts)
- ✓ No external dependencies beyond Qt
- ✓ ~1500 lines total code
- ✓ No sleep/busy-wait/timing hacks
- ✓ Deterministic behavior

---

## Used By Test Categories

- **qt/smoke/** - qt_test_base, test_websocket_server, test_websocket_client
- **qt/state/** - qt_test_base, test_signal_helper, test_websocket_client
- **qt/signals/** - qt_test_base, test_signal_helper, test_websocket_server
- **qt/models/** - qt_test_base, test_signal_helper, test_model_helper, test_websocket_server
- **qt/websocket/** - All fixtures
