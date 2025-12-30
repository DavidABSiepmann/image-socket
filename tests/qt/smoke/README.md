# Qt Smoke Tests

Lightweight sanity checks for Qt components and framework infrastructure. These tests verify that core components can be instantiated and basic operations work correctly.

## Test Files

### test_component_instantiation.cpp
Tests instantiation of production components:
- **testImageServerBridgeInstantiation()** - Verifies ImageServerBridge can be created, properties are accessible, initial state is correct
- **testClientModelInstantiation()** - Verifies ClientModel works, can add/remove clients, model operations are functional
- **testClientSessionInstantiation()** - Verifies ClientSession class is linkable and fixture setup works
- **testObjectTreeCleanup()** - Verifies Qt parent-child object relationships work correctly

**Result:** 6 tests, all passing ✓

### test_dependencies.cpp
Tests Qt framework and dependency availability:
- **testQtCoreApplicationAvailable()** - QCoreApplication instance exists
- **testEventLoopAvailable()** - QEventLoop can be created and used
- **testQtSignalsWork()** - Signal/slot mechanism is functional
- **testQSignalSpyWorks()** - QSignalSpy available for signal introspection
- **testQtTimersAvailable()** - QTimer creation and state management
- **testMetaObjectSystem()** - Qt meta-object system works
- **testQtTypeSystem()** - QVariant type conversions work
- **testEventLoopSpinnerWorks()** - Test fixture helper (EventLoopSpinner) is functional
- **testSignalWaiterWorks()** - Test fixture helper (SignalWaiter) is functional
- **testQtTestAssertionsWork()** - QVERIFY, QCOMPARE macros work
- **testQtFileSystemAvailable()** - QDir/QFile classes are linkable
- **testFixtureInitializationSafe()** - Multiple fixture initialization calls are safe
- **testQtMemoryManagement()** - deleteLater() and deferred deletion work

**Result:** 15 tests, all passing ✓

## Framework
- QtTest (QTEST_MAIN, assertions)
- Qt5 Components: Core, Test, WebSockets, Gui
- Test Fixtures: qt_test_base.h (utilities), test_websocket_server.h, test_websocket_client.h, test_model_helper.h, test_signal_helper.h

## Execution
```bash
# Build
cd build && cmake .. && make -j4

# Run smoke tests
ctest -R "^qt_smoke_" -V

# Run individual test
./build/tests/qt/qt_smoke_test_component_instantiation
./build/tests/qt/qt_smoke_test_dependencies
```

## Requirements
- Each test must complete in < 200ms
- No networking, no threading, no complex logic
- No file I/O or system calls
- Safe to run in parallel with other test suites
- No assertions/verification in fixtures (test code is responsible)

## Design Principles
- **Header-only fixtures**: Zero compilation overhead
- **RAII patterns**: Automatic cleanup on scope exit
- **Non-blocking waits**: QSignalSpy-based, no threads or sleep()
- **Namespace isolation**: qt_test:: prevents naming conflicts
- **QObject lifecycle management**: QtTestApplicationGuard ensures proper initialization

## Coverage
These are **entry-point tests** that verify:
1. Production components can be instantiated without crashing
2. Qt framework is correctly configured and linked
3. Test infrastructure (fixtures) is available for higher-level tests

More comprehensive tests are in:
- `qt_state_*` - State machine and transitions
- `qt_signals_*` - Signal emission and connections
- `qt_models_*` - Model behavior and data access
- `qt_websocket_*` - WebSocket integration

## Status
✓ Implementation Complete (100%)
✓ All 21 tests passing
✓ No warnings or errors
✓ Ready for CI/CD integration
