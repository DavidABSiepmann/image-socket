# QML Smoke Tests

## Overview

Smoke tests are lightweight compilation and instantiation checks that verify QML components are syntactically correct and can be loaded without errors. These tests focus on **isolatable pure QML components** that don't depend on C++ backend modules.

## Current Status

✅ **All 8 tests passing** (100%)

```
Test Results:
  qml_smoke_theme_component_loads                    PASSED ✅
  qml_smoke_theme_properties_accessible              PASSED ✅
  qml_smoke_info_overlay_component_compiles          PASSED ✅
  qml_smoke_toast_notification_component_compiles    PASSED ✅
  qml_smoke_styled_button_component_compiles         PASSED ✅
  qml_smoke_clients_panel_component_compiles         PASSED ✅
  qml_smoke_diagnostics_panel_component_compiles     PASSED ✅
  qml_smoke_all_simple_qml_components_compile        PASSED ✅

Total: 8 passed, 0 failed (0.12 sec)
```

## Test Categories

### 1. Fully Instantiable Components

These components can be fully loaded and instantiated in the test QML engine:

- **Theme.qml** - Centralized styling definitions
  - ✅ Loads successfully
  - ✅ Properties accessible: `themeMode`, `backgroundColor`, `textColor`, `accentColor`
  - No backend dependencies
  - No visual scene graph requirements

### 2. Compilation-Only Components

These components can be compiled and parsed but should not be instantiated (may require scene graph or visual context):

- **InfoOverlay.qml** - Information display overlay
- **ToastNotification.qml** - Toast notification component
- **StyledButton.qml** - Styled button component
- **ClientsPanel.qml** - Clients panel view
- **DiagnosticsPanel.qml** - Diagnostics panel view

These use `checkCompilation()` helper which verifies syntax without instantiation.

### 3. Batch Test

- **test_all_simple_qml_components_compile** - Tests all simple QML files for compilation

## Skipped Tests

The following components are intentionally **skipped** from smoke tests:

### Backend-Dependent Components
- **StatusBar.qml** - Requires `ImageServerBridge` C++ module
- **ControlPanel.qml** - Requires `ImageServerBridge` C++ module
- **VideoDisplayArea.qml** - Requires scene graph and visual context

### Singleton Components
- **EventMessages.qml** - Uses `pragma Singleton` (cannot be instantiated via QQmlComponent)

**Rationale**: Qt design prevents direct instantiation of Singleton components. They must be imported at module level, not created dynamically.

## Test Implementation

### Two Testing Strategies

```cpp
// Strategy 1: Full instantiation test
bool loadComponent(const QString &filePath, QString &errorMsg) {
    QQmlComponent component(engine, QUrl::fromLocalFile(filePath));
    if (component.isError()) {
        // Capture errors
        return false;
    }
    QObject *obj = component.create();
    if (!obj) {
        // Component failed to instantiate
        return false;
    }
    delete obj;
    return true;  // ✅ Fully loaded and functional
}

// Strategy 2: Compilation-only test
bool checkCompilation(const QString &filePath, QString &errorMsg) {
    QQmlComponent component(engine, QUrl::fromLocalFile(filePath));
    if (component.isError()) {
        // Capture errors
        return false;
    }
    return true;  // ✅ Syntax valid, don't instantiate
}
```

## Testing Pattern

Each test follows this pattern:

```cpp
void test_component_name() {
    // 1. Define component path
    QString qmlPath = componentPath("ComponentName.qml");
    QString errorMsg;

    // 2. Load or compile component
    if (!loadComponent(qmlPath, errorMsg)) {
        QFAIL(qPrintable("Load failed: " + errorMsg));
    }

    // 3. Verify specific properties/behavior (optional)
    // QVERIFY(obj->property("propName").isValid());
}
```

## Running Tests

### Run all smoke tests:
```bash
cd build
ctest -R "^qml_smoke_"
```

### Run single test:
```bash
./tests/qml/smoke/qml_smoke_test_all_components test_theme_component_loads
```

### Run with verbose output:
```bash
ctest -R "^qml_smoke_" --output-on-failure
```

## Extending Smoke Tests

To add a new smoke test for a component:

1. **Verify component dependencies**:
   - No C++ backend imports? → Can fully instantiate
   - Has `pragma Singleton`? → Must skip
   - Requires scene graph? → Use `checkCompilation()`

2. **Add test method** to `test_qml_smoke.cpp`:
   ```cpp
   void test_mycomponent_loads() {
       QString qmlPath = componentPath("MyComponent.qml");
       QString errorMsg;
       if (!loadComponent(qmlPath, errorMsg)) {
           QFAIL(qPrintable("Load failed: " + errorMsg));
       }
   }
   ```

3. **Register test** in `CMakeLists.txt`:
   ```cmake
   add_test(NAME qml_smoke_mycomponent_loads
            COMMAND qml_smoke_test_all_components test_mycomponent_loads)
   ```

4. **Compile and run**:
   ```bash
   cmake --build .
   ctest -R "^qml_smoke_"
   ```

## Next Steps: Full Component Tests

Smoke tests verify basic loading. For comprehensive testing, use the **QML Test Fixtures**:

- [MockBackend.qml](../fixtures/MockBackend.qml) - Control component state in tests
- [SignalRecorder.qml](../fixtures/SignalRecorder.qml) - Record signal emissions
- [AnimationHelper.qml](../fixtures/AnimationHelper.qml) - Monitor animation state

See [fixtures README](../fixtures/README.md) for full fixture documentation.

## See Also

- [QML Fixtures Documentation](../fixtures/README.md) - Reusable test helpers
- [QML Test Quick Start](../fixtures/QUICK_START.md) - Copy-paste examples
- [Main test suite documentation](../../TESTS_QT_QML.md) - All QML testing strategy
