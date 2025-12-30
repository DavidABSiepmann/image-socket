# Smoke Tests

Lightweight sanity check tests verifying the build environment, library integration, and basic functionality.
**Total: 19 tests, 100% passing**

## Test Files

### test_smoke_linkage.cpp (2 tests)
Verifies library linkage and symbol availability:
- Confirms imagesocket library is properly linked
- Verifies protobuf library integration
- Tests basic symbol resolution

### test_smoke_enums.cpp (4 tests)
Validates Protobuf enum constant availability:
- CommandType enum values (PAUSE, RESUME, SET_FPS, SET_QUALITY, ID, REQUEST_RESUME, SUBSCRIBE, UNSUBSCRIBE, REQUEST_ALIAS, ALIAS, UNKNOWN)
- Enum default values
- Enum comparison operators
- Multiple enum constant access

### test_smoke_construction.cpp (4 tests)
Validates basic object construction:
- ControlMessage default construction
- ControlMessage field access
- Message field initialization
- Destructor execution (RAII)

### test_smoke_fixtures.cpp (7 tests)
Validates test fixture infrastructure:
- GoogleTest fixture inheritance
- GoogleMock mock creation
- Fixture setup/teardown execution
- Multiple fixture instances
- Fixture state isolation
- Test isolation verification

## Framework
- GoogleTest (gtest) v1.14.0
- GoogleMock (gmock)
- Protobuf v3 (libprotobuf-lite)
- C++ Standard Library only

## Build Configuration
- CMake targets: smoke_linkage, smoke_enums, smoke_construction, smoke_fixtures
- Linked with: imagesocket, GTest::gtest, GTest::gtest_main, gmock (fixtures only)
- Dependencies: Generated protobuf headers
- Test discovery: `ctest -R "^smoke_"`

## Naming Convention
test_smoke_*.cpp

## Purpose in Testing Pipeline

Smoke tests run first to:
1. Verify compilation succeeded
2. Check linker found all symbols
3. Confirm basic object functionality
4. Validate testing framework setup

If smoke tests fail, more complex unit tests won't run.
