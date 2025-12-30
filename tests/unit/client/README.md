# Client Logic Tests

Unit tests for pure C++ client logic testing isolated business logic without I/O, networking, or threading.
**Total: 111 tests, 100% passing**

## Test Files

### test_reconnect_backoff.cpp (25 tests)
Validates exponential backoff calculation for reconnection attempts:
- Formula: `min(30, 1 << min(attempt, 6))`
- Individual attempt calculations (attempts 0-100)
- Backoff sequence generation
- Determinism verification
- Maximum backoff cap at 30 seconds
- Exponential growth validation
- Practical reconnection scenarios

### test_client_state_machine.cpp (26 tests)
Validates client connection state machine transitions:
- States: Disconnected → Connecting → Connected → Disconnecting → Disconnected
- Valid and invalid transition enforcement
- Error recovery flows
- State persistence across multiple getters
- Reset functionality
- Transition counter accuracy

### test_configuration_accumulation.cpp (33 tests)
Validates configuration persistence and override behavior:
- Configuration fields: fps (int), quality (int), alias (string), client_id (int)
- Initial unset state
- Sequential accumulation with persistence
- Override behavior (new values replace old)
- Completeness checking (IsFullyConfigured, IsPartiallyConfigured)
- Reset functionality
- Configuration equality/inequality

### test_error_callback.cpp (27 tests)
Validates error callback propagation using GoogleMock:
- Error codes: ConnectionFailed, ConnectionTimeout, ProtocolError, AuthenticationFailed, ServerRejected, SocketError
- Callback invocation on error conditions
- Error code and message propagation
- Error count tracking (incremental)
- Last error information tracking
- Error code to string conversion
- Callback registration and replacement

## Framework
- GoogleTest (gtest) v1.14.0
- GoogleMock (gmock) for callback verification
- C++ Standard Library only
- No Qt, threading, or real socket dependencies

## Build Configuration
- CMake targets: unit_client_backoff, unit_client_state_machine, unit_client_accumulation, unit_client_error_callback
- Linked with: GTest::gtest, GTest::gtest_main, gmock
- Test discovery: `ctest -R "^unit_client_"`

## Naming Convention
test_*.cpp
