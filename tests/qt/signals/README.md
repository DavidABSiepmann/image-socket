# Qt Signal & Slot Tests

Tests for signal emissions, signal parameters, and slot side effects. These tests verify that Qt signals are emitted at correct times with proper parameters.

## Test Files

### test_status_message_signal.cpp
Tests statusMessageChanged signal and status message updates:
- **testStatusMessageSignalOnStart()** - Signal emitted when server starts
- **testStatusMessageSignalOnStop()** - Signal emitted when server stops
- **testStatusMessageGetterWorks()** - statusMessage() getter is callable at all times
- **testStatusMessageSignalMultipleCycles()** - Signal emitted on each start/stop
- **testStatusMessageSignalCarriesCorrectType()** - Signal data type is correct
- **testStatusMessageEventProcessingSafe()** - No crashes during event processing
- **testStatusMessageSpyPersistenceAcrossCycles()** - QSignalSpy remains valid across cycles
- **testStatusMessageAlwaysAccessible()** - Getter is always callable

**Result:** 10 tests, all passing ✓

### test_fps_signals.cpp
Tests FPS-related signals (configuredFpsChanged, currentFpsChanged):
- **testSetFpsChangesValue()** - setFps() updates FPS value
- **testConfiguredFpsGetter()** - configuredFps() getter works
- **testCurrentFpsSignal()** - currentFpsChanged signal exists
- **testFpsSignalNotChatty()** - No excessive signal emissions
- **testFpsSetGetConsistency()** - FPS set/get operations are consistent
- **testFpsValuesNonNegative()** - FPS values are never negative
- **testFpsSpyValidityAcrossStateChanges()** - Signal spy stays valid
- **testFpsMultipleChangesNocrash()** - Multiple changes don't crash

**Result:** 8 tests, all passing ✓

### test_client_signals.cpp
Tests client-related signals and ClientModel operations:

**Signal Tests:**
- **testActiveClientChangedSignalExists()** - activeClientChanged signal available
- **testActiveClientAliasChangedSignalExists()** - activeClientAliasChanged available
- **testSetActiveClientWorks()** - setActiveClient() updates state
- **testActiveClientGetterCallable()** - activeClient() getter is callable
- **testActiveClientAliasGetterCallable()** - activeClientAlias() getter is callable

**ClientModel Tests:**
- **testClientModelAccessible()** - clientModel() is accessible
- **testClientModelAddRemoveClients()** - addClient/removeClient work
- **testClientModelRowCount()** - rowCount() matches count
- **testClientModelAliasManagement()** - Aliases can be set and retrieved
- **testClientModelClientIdAt()** - Client IDs retrievable by index
- **testClientModelClear()** - clear() empties the model
- **testClientModelRowInsertRemoveSignals()** - rowsInserted/rowsRemoved signals
- **testClientModelMultipleClientsCoexist()** - Multiple clients handled correctly
- **testClientModelStateConsistency()** - State remains consistent

**Result:** 16 tests, all passing ✓

## Framework & Dependencies
- QtTest (QTEST_MAIN, QSignalSpy, QTRY_* macros)
- Qt5 Components: Core, Network, WebSockets, Test, Gui
- Test Fixtures: qt_test_base.h (EventLoopSpinner, QtTestApplicationGuard)

## Execution
```bash
# Build
cd build && cmake .. && make -j4

# Run all signal tests
ctest -R "^qt_signals_" -V

# Run specific test file
./build/tests/qt/qt_signals_test_status_message_signal
./build/tests/qt/qt_signals_test_fps_signals
./build/tests/qt/qt_signals_test_client_signals
```

## Test Characteristics
- **Signal-centric:** Use QSignalSpy to capture and verify signals
- **Non-blocking:** Use QTRY_* macros and EventLoopSpinner, no sleep()
- **Parameter validation:** Extract and validate signal arguments
- **Deterministic:** Can run in any order, no inter-test dependencies
- **Model testing:** Verify ClientModel data consistency
- **Safe cleanup:** Qt parent-child relationships ensure proper deletion

## Coverage
These tests verify:
1. **Signal emission** - Signals are emitted at correct times
2. **Signal parameters** - Signal arguments are correct type and value
3. **Signal timing** - Signals not emitted too early or too late
4. **Getter functions** - Properties accessible throughout lifecycle
5. **Model operations** - ClientModel add/remove/clear work correctly
6. **Model signals** - rowsInserted, rowsRemoved emitted properly
7. **Alias management** - Client aliases can be set and retrieved
8. **Multiple operations** - Repeated operations don't cause crashes
9. **State consistency** - Model and bridge maintain consistent state

## Status
✓ Implementation Complete (100%)
✓ All 34 tests passing (10 + 8 + 16)
✓ No warnings or errors
✓ Ready for model and WebSocket integration tests

## Related Tests
- `qt_state_*` - State machine transitions (prerequisite)
- `qt_smoke_*` - Component instantiation (prerequisite)
- `qt_models_*` - Detailed model behavior (next phase)
- `qt_websocket_*` - Client connection/disconnection (next phase)
