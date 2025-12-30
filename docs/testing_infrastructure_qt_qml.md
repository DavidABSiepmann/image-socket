# Testing Infrastructure: Qt Quick (QML) Component Tests

## 1. Purpose

This document defines the testing strategy for **QML UI components and logic** including Qt Quick items, bindings, animations, and interactions with C++ backend objects.

These tests ensure that the QML layer correctly reflects backend state, responds to user interactions, and properly implements data bindings and event handlers. **Note:** This is *behavioral* testing, not visual/pixel-perfect rendering validation.

**Architecture reference:** See [`ARCHITECTURE.md`](ARCHITECTURE.md) and [`QUICKSTART.md`](QUICKSTART.md) for setup guidance.

**See also:**
- [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) — Qt C++ components (signals, state machines, models)
- [`testing_infrastructure_std.md`](testing_infrastructure_std.md) — Pure C++ / Protobuf unit tests

---

## 2. Scope

### What to test:
- **QML component visibility/state**: Visible, enabled, opacity based on backend properties
- **Bindings**: Data flows correctly from C++ to QML (one-way and two-way bindings)
- **Animations**: Transitions complete and end states are correct
- **Signal connections**: QML receives C++ signals and updates UI accordingly
- **Button/interaction handling**: Click responses, key presses
- **Custom components**: ToastNotification, Theme
- **Component initialization**: Components load without errors, properties initialized

### What NOT to test:
- **Visual appearance or pixel-perfect layout** (use manual QA or screenshot testing, not here)
- **Styling details** (colors, fonts, exact padding) — assume CSS/stylesheet correctness
- **Performance optimization** (use dedicated benchmarks, not unit tests)
- **C++ business logic** (test in [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) instead)
- **Platform-specific behavior** (desktop vs. mobile rendering) — test cross-platform logic in C++ layer
- **Actual image display quality** (test image provider in C++ tests; assume rendering works)
- **Animations' exact timing or smoothness** — test start/end values, not frame-by-frame rendering

**Important:** QML tests are **not visual validation tests**. They verify behavior: that properties bind correctly, that event handlers fire, that state transitions trigger the right UI updates. Manual testing or visual regression tools handle pixel-perfect layout validation.

---

## 3. Test Categories

### 3.1 Component State & Visibility Tests (`qml_component_*`)
**Location:** `tests/qml/components/`

**What to test:**
- Component visibility toggled by backend properties
- Component enabled state based on conditions
- Opacity animations (fadeIn, fadeOut)
- Text content updated from backend
- Property defaults are correct

**Example test cases:**
```
test_info_overlay_visible_when_client_connected
  → Load InfoOverlay QML component
  → Set backend.activeClientAlias = "" → Verify component invisible
  → Set backend.activeClientAlias = "Alice" → Verify component visible

test_status_bar_message_binding
  → Load StatusBar component
  → Update backend.statusMessage = "New message"
  → Verify QML text property reflects new message

test_toast_notification_opacity_animation
  → Load ToastNotification → Call show("message", 100)
  → Verify opacity animates from 0 → 1 within timeout window
  → Call hide() → Verify opacity animates from 1 → 0

test_control_panel_button_enabled_state
  → Load ControlPanel with backend in Idle state
  → Verify "Start" button is enabled, "Stop" button is disabled
  → Transition backend to Running state
  → Verify "Stop" button becomes enabled, "Start" disabled
```

**Dependencies:**
- Qt5::Quick, Qt5::Qml, Qt5::Test
- Qt Quick Test framework (qmltest)
- Mocked or real C++ backend objects exposed to QML

**Framework:**
- Qt Quick Test (qmltest) — built on QtTest

---

### 3.2 Data Binding Tests (`qml_binding_*`)
**Location:** `tests/qml/bindings/`

**What to test:**
- One-way bindings from C++ properties to QML
- Two-way bindings (if any) with property synchronization
- Binding updates propagate correctly
- Null/undefined values handled gracefully
- Type conversions (int, string, bool)

**Example test cases:**
```
test_video_display_image_binding
  → Load VideoDisplayArea → Backend provides image via provider
  → Set backend.currentImage = <new image>
  → Verify QML <Image> source updates
  → Verify imageChanged signal received

test_fps_display_binding
  → Set backend.currentFps = 24
  → Verify QML text label shows "24 FPS"
  → Set backend.currentFps = 30
  → Verify label updates to "30 FPS"

test_two_way_binding_fps_slider
  → Load ControlPanel with FPS slider
  → User drags slider → newFps = 60
  → Verify backend.configuredFps updated to 60
  → Backend emits fpsChanged(60)
  → Verify slider position reflects 60

test_null_alias_handled_gracefully
  → Set backend.activeClientAlias = null (or undefined)
  → Load component that displays alias
  → Verify component doesn't crash
  → Verify fallback text displayed (e.g., "No client")
```

**Dependencies:**
- Qt5::Quick, Qt5::Qml, Qt5::Test
- C++ backend objects (real or mocked) registered with QML

**Framework:**
- Qt Quick Test

---

### 3.3 QML Event Handling Tests (`qml_events_*`)
**Location:** `tests/qml/events/`

**What to test:**
- Button click handlers execute correct C++ slots
- Key presses trigger expected behaviors
- Timer signals trigger animations
- Focus handling
- Mouse event dispatch

**Example test cases:**
```
test_start_button_click_handler
  → Load ControlPanel → Click "Start" button
  → Verify backend.start() slot called
  → Verify state transitions to Running

test_theme_toggle_button
  → Load AppWindow → Click theme toggle
  → Verify themeMode property changes
  → Verify all components re-render with new theme

test_key_press_shortcut
  → Load AppWindow → Press Ctrl+Q
  → Verify quit signal emitted
  → Verify application shutdown initiated

test_mouse_drag_slider
  → Load ControlPanel with FPS slider
  → Simulate mouse drag to position X
  → Verify onValueChanged signal fired
  → Verify backend.configuredFps updated
```

**Dependencies:**
- Qt5::Quick, Qt5::Test
- TestUtil helpers for mouse/key simulation

**Framework:**
- Qt Quick Test with TouchEventSequence / mouseClick()

---

### 3.4 Custom Component Integration Tests (`qml_integration_*`)
**Location:** `tests/qml/integration/`

**What to test:**
- Complex multi-component interactions
- Theme changes propagate to all components
- Client list updates trigger view refresh
- Error messages display correctly

**Example test cases:**
```
test_state_manager_coordinates_components
  → Verify VideoDisplayArea enabled (Running state)
  → Verify ControlPanel buttons update
  → Verify StatusBar shows "Running" message

test_theme_propagates_to_all_components
  → Load AppWindow → Change theme to "dark"
  → Verify StatusBar backgroundColor reflects dark theme
  → Verify ControlPanel button colors updated
  → Verify VideoDisplayArea background updated

test_client_list_updates_video_display
  → Load MainWindow with client list
  → Add client "Alice" → Verify InfoOverlay shows "Alice"
  → Add client "Bob" → Verify display switches to Bob
  → Remove Bob → Verify display reverts to Alice

test_error_message_display
  → Simulate backend error (e.g., port in use)
  → Verify error message displayed in StatusBar
  → Verify error toast notification shown
  → Click "Dismiss" → Verify toast hidden
```

**Dependencies:**
- Qt5::Quick, Qt5::Qml, Qt5::Test
- Multiple QML components and C++ backend

**Framework:**
- Qt Quick Test with custom helpers

---

### 3.5 Animation Tests (`qml_animation_*`)
**Location:** `tests/qml/animations/`

**What to test:**
- Animations start and end at correct values
- Animation duration is correct
- Animations can be paused/resumed
- Multiple animations don't conflict

**Example test cases:**
```
test_toast_fade_in_animation
  → Load ToastNotification
  → Call show(text, duration=100)
  → At t=50ms, verify opacity is between 0 and 1
  → At t=100ms, verify opacity == 1

test_toast_fade_out_animation
  → Toast visible with opacity=1
  → Call hide()
  → Verify opacity animates from 1 → 0 over duration
  → Verify visible=false at end

test_status_message_fade_animation
  → Update statusMessage (triggers fade)
  → Old message fades out while new fades in
  → Verify both animations run concurrently
  → Verify new message visible at end

test_button_hover_animation
  → Hover over styled button
  → Verify color animates to highlight state
  → Un-hover → Verify color returns to normal
```

**Dependencies:**
- Qt5::Quick, Qt5::Test

**Framework:**
- Qt Quick Test with timing helpers

---

### 3.6 Smoke Tests (`qml_smoke_*`)
**Location:** `tests/qml/smoke/`

**Purpose:** Quick verification that QML components load without errors.

**What to test:**
- Each QML file can be loaded
- No QML syntax errors
- No import errors
- Components initialize without crashing

**Example test cases:**
```
test_main_window_loads
  → Load main.qml
  → Verify no QML errors
  → Verify ApplicationWindow created
  → Verify children initialized

test_all_components_loadable
  → For each component in qml/ directory:
    - Load component
    - Verify status == QQmlComponent::Ready
    - Verify no errors

test_theme_constants_accessible
  → Load Theme component
  → Verify all color/size properties accessible
  → Verify no undefined values
```

**Framework:**
- Qt Quick Test

---

## 4. QML Test Structure & Naming

### File organization:
```
tests/qml/
├── CMakeLists.txt
├── components/
│   ├── test_info_overlay_visibility.cpp
│   ├── test_toast_notification_behavior.cpp
│   └── test_status_bar_updates.cpp
├── bindings/
│   ├── test_video_display_image_binding.cpp
│   ├── test_fps_display_binding.cpp
│   └── test_alias_binding.cpp
├── events/
│   ├── test_button_click_handlers.cpp
│   └── test_keyboard_shortcuts.cpp
├── integration/
│   ├── test_state_manager_coordination.cpp
│   ├── test_theme_propagation.cpp
│   └── test_toast_integration.cpp
├── animations/
│   ├── test_toast_animations.cpp
│   └── test_message_fade_animation.cpp
├── smoke/
│   ├── test_components_load.cpp
│   └── test_no_qml_errors.cpp
└── fixtures/
    ├── mock_backend.h
    ├── qml_test_utils.h
    └── test_data/
```

### Naming conventions:
- File: `test_<component>_<aspect>.cpp`
- Test class: `Test<Component><Aspect>` (inherits from `QObject`)
- Test method: `test<behavior>()` (decorated with `private slots:`)

### QML Test File Pattern
```cpp
#include <QtTest/QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QSignalSpy>

class TestComponentName : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Global setup
    }

    void cleanupTestCase() {
        // Global cleanup
    }

    void init() {
        // Per-test setup
    }

    void cleanup() {
        // Per-test cleanup
    }

    void testBehavior() {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile("path/to/component.qml"));
        
        if (component.status() != QQmlComponent::Ready) {
            QFAIL(qPrintable(component.errorString()));
        }
        
        QObject* obj = component.create();
        QVERIFY(obj != nullptr);
        
        // Test behavior...
        
        delete obj;
    }
};

QTEST_MAIN(TestComponentName)
#include "test_component_name.moc"
```

### CMake integration:
```cmake
# tests/qml/CMakeLists.txt
find_package(Qt5 COMPONENTS Core Qml Quick Test REQUIRED)

# Component state tests
add_executable(qml_component_info_overlay components/test_info_overlay_visibility.cpp)
set_target_properties(qml_component_info_overlay PROPERTIES AUTOMOC ON)
target_compile_definitions(qml_component_info_overlay PRIVATE 
    PROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")
target_link_libraries(qml_component_info_overlay PRIVATE 
    imagesocket Qt5::Core Qt5::Qml Qt5::Quick Qt5::Test)
add_test(NAME qml_component_info_overlay COMMAND qml_component_info_overlay)

# Similar for other tests...
```

---

## 5. QML Testing Best Practices

### 5.0 MockBackend Architecture

QML tests require C++ objects to be exposed to the QML engine. This project uses a **MockBackend** pattern to avoid tight coupling to the real application backend during testing:

```cpp
// tests/qml/fixtures/mock_backend.h
class MockBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeClientAlias MEMBER m_alias NOTIFY aliasChanged)
    Q_PROPERTY(int currentFps MEMBER m_fps NOTIFY fpsChanged)
    Q_PROPERTY(ServerState serverState MEMBER m_state NOTIFY stateChanged)

public:
    MockBackend() : m_alias(""), m_fps(0), m_state(Idle) {}

    Q_INVOKABLE void start() { m_state = Running; emit stateChanged(); }
    Q_INVOKABLE void stop() { m_state = Idle; emit stateChanged(); }

    // Properties for binding
    QString m_alias;
    int m_fps;
    ServerState m_state;

signals:
    void aliasChanged();
    void fpsChanged();
    void stateChanged();
};
```

**Benefits:**
- Tests do not depend on real application initialization
- Tests run fast (no network, no actual image I/O)
- Tests can inject specific states/signals to verify QML behavior
- Easy to reset state between tests

**Usage in tests:**
```cpp
QQmlEngine engine;
MockBackend backend;
engine.rootContext()->setContextProperty("backend", &backend);
// Now QML components can bind to backend.activeClientAlias, etc.
```

### 5.1 Loading QML Components
```cpp
QQmlEngine engine;
QQmlComponent component(&engine, QUrl::fromLocalFile(
    QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/MyComponent.qml"));

if (component.isError()) {
    qWarning() << component.errors();
    QFAIL("QML component failed to load");
}

QObject* obj = component.create();
QVERIFY(obj != nullptr);

// (Component loaded and ready for testing)
// ...test behavior here...

delete obj;  // Clean up
```

**Common errors:**
- Missing `PROJECT_SOURCE_DIR` compile definition → component file not found
- QML import errors → missing modules or incorrect paths
- Component creation returns nullptr → syntax errors in QML

### 5.2 Accessing QML Properties from C++
```cpp
// Read property
QVariant value = obj->property("propertyName");
QCOMPARE(value.toString(), QString("expected"));

// Set property
bool success = obj->setProperty("propertyName", QVariant("newValue"));
QVERIFY(success);

// Watch for property changes
QSignalSpy spy(obj, SIGNAL(propertyNameChanged()));
obj->setProperty("propertyName", QVariant("changed"));
QCOMPARE(spy.count(), 1);
```

### 5.3 Invoking QML Methods from C++
```cpp
QVariant result;
bool success = QMetaObject::invokeMethod(
    obj, "methodName",
    Q_RETURN_ARG(QVariant, result),
    Q_ARG(QVariant, arg1),
    Q_ARG(QVariant, arg2));

QVERIFY(success);
```

### 5.4 Mouse & Keyboard Simulation
```cpp
#include <QTest>

// Click button at position
QTest::mouseClick(qobject_cast<QQuickItem*>(buttonObj), Qt::LeftButton);

// Type text
QTest::keyClicks(window, "hello", Qt::NoModifier);

// Press key
QTest::keyPress(window, Qt::Key_Escape);
QTest::keyRelease(window, Qt::Key_Escape);
```

### 5.5 Mocking C++ Backend for QML

The **MockBackend** class (see section 5.0) provides test doubles for the real application backend. Each test creates an instance and injects it into the QML engine:

```cpp
class TestInfoOverlayVisibility : public QObject {
    Q_OBJECT

private slots:
    void testVisibilityBinding() {
        QQmlEngine engine;
        MockBackend backend;
        engine.rootContext()->setContextProperty("backend", &backend);

        QQmlComponent component(&engine, 
            QUrl::fromLocalFile(PROJECT_SOURCE_DIR + "/apps/server/qml/InfoOverlay.qml"));
        QObject* overlay = component.create();

        // Test: overlay invisible when alias is empty
        backend.m_alias = "";
        QCOMPARE(overlay->property("visible").toBool(), false);

        // Test: overlay visible when alias is set
        backend.m_alias = "Alice";
        emit backend.aliasChanged();  // Trigger binding update
        QCOMPARE(overlay->property("visible").toBool(), true);

        delete overlay;
    }
};
```

**Key points:**
- MockBackend properties can be directly set (no real I/O)
- Signals are emitted to trigger QML bindings
- Tests can verify reactive behavior without complex setup
- Avoids coupling to real application initialization

### 5.6 Waiting for Animations to Complete
```cpp
// Wait for opacity to reach 1.0
QObject* toastRect = obj->findChild<QObject*>("toastRect");
QTRY_COMPARE_WITH_TIMEOUT(toastRect->property("opacity").toDouble(), 1.0, 1000);

// Or use explicit event loop spin
QEventLoop loop;
QTimer timer;
QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
timer.start(100);
loop.exec();
```

---

## 6. Minimal Test Checklist

For each new QML test:

- [ ] QML file(s) referenced are syntactically valid (no QML errors on load)
- [ ] Test class inherits from `QObject` (decorated with `Q_OBJECT`)
- [ ] Test methods are `private slots:`
- [ ] `AUTOMOC ON` set in CMake
- [ ] `PROJECT_SOURCE_DIR` defined in `target_compile_definitions()` for QML file path resolution
- [ ] Components loaded with `QQmlComponent` and errors explicitly checked
- [ ] MockBackend injected into QML engine via `setContextProperty()`
- [ ] All QML objects properly cleaned up (`delete` or explicit cleanup)
- [ ] Signals captured with `QSignalSpy` where testing event propagation
- [ ] Event loop spinned with `QTRY_*` macros for async operations (not `qWait()`)
- [ ] Test is deterministic (no timing dependencies, no flakiness)
- [ ] Test runs in < 1000 ms (QML component loading can be slower than C++ tests)
- [ ] Assertions use `QVERIFY`, `QCOMPARE`, `QTRY_*` from QtTest
- [ ] CMakeLists.txt entry created with `add_test(NAME qml_* ...)`
- [ ] Test name starts with `qml_` prefix
- [ ] Shared fixtures and MockBackend in `tests/qml/fixtures/`
- [ ] **NOT testing visual/pixel appearance** — test behavior only

---

## 7. Test Execution

### Run all QML tests:
```bash
cd build
ctest -R "^qml_" -V
```

### Run specific category:
```bash
ctest -R "^qml_component_" -V
ctest -R "^qml_animation_" -V
```

### Run with verbose output and stop on failure:
```bash
ctest -R "^qml_" --output-on-failure --stop-on-failure
```

---

## 8. Example: Adding a New Test

### Scenario: Test InfoOverlay visibility

**File:** `tests/qml/components/test_info_overlay_visibility.cpp`

```cpp
#include <QtTest/QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QSignalSpy>

class TestInfoOverlayVisibility : public QObject {
    Q_OBJECT

private slots:
    void testInvisibleWhenNoClient() {
        QQmlEngine engine;
        
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/InfoOverlay.qml";
        QQmlComponent component(&engine, QUrl::fromLocalFile(qmlPath));
        
        QVERIFY2(component.status() == QQmlComponent::Ready, 
                 qPrintable(component.errorString()));
        
        QObject* overlay = component.create();
        QVERIFY(overlay != nullptr);
        
        // Initially, activeClientAlias is empty → overlay should be invisible
        overlay->setProperty("activeClientAlias", QVariant::fromValue(QString("")));
        bool visible = overlay->property("visible").toBool();
        QVERIFY(!visible);
        
        delete overlay;
    }

    void testVisibleWhenClientConnected() {
        QQmlEngine engine;
        
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/InfoOverlay.qml";
        QQmlComponent component(&engine, QUrl::fromLocalFile(qmlPath));
        
        QVERIFY2(component.status() == QQmlComponent::Ready, 
                 qPrintable(component.errorString()));
        
        QObject* overlay = component.create();
        QVERIFY(overlay != nullptr);
        
        // Set activeClientAlias → overlay should become visible
        overlay->setProperty("activeClientAlias", QVariant::fromValue(QString("Alice")));
        
        // Wait for binding to propagate (with timeout)
        QTRY_VERIFY_WITH_TIMEOUT(overlay->property("visible").toBool(), 500);
        
        delete overlay;
    }

    void testAliasTextUpdates() {
        QQmlEngine engine;
        
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/InfoOverlay.qml";
        QQmlComponent component(&engine, QUrl::fromLocalFile(qmlPath));
        
        QVERIFY2(component.status() == QQmlComponent::Ready, 
                 qPrintable(component.errorString()));
        
        QObject* overlay = component.create();
        QVERIFY(overlay != nullptr);
        
        // Set alias to "Bob"
        overlay->setProperty("activeClientAlias", QVariant::fromValue(QString("Bob")));
        
        // Find the text element (assuming it has objectName "aliasText")
        QObject* textElement = overlay->findChild<QObject*>("aliasText");
        QVERIFY(textElement != nullptr);
        
        // Verify text updated
        QString displayedAlias = textElement->property("text").toString();
        QCOMPARE(displayedAlias, QString("Bob"));
        
        delete overlay;
    }
};

QTEST_MAIN(TestInfoOverlayVisibility)
#include "test_info_overlay_visibility.moc"
```

**Update CMakeLists.txt:**
```cmake
add_executable(qml_component_info_overlay components/test_info_overlay_visibility.cpp)
set_target_properties(qml_component_info_overlay PROPERTIES AUTOMOC ON)
target_compile_definitions(qml_component_info_overlay PRIVATE 
    PROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")
target_link_libraries(qml_component_info_overlay PRIVATE 
    imagesocket Qt5::Core Qt5::Qml Qt5::Quick Qt5::Test)
add_test(NAME qml_component_info_overlay COMMAND qml_component_info_overlay)
```

---

## 9. What to Avoid

- ❌ Testing visual appearance or pixel-perfect layout
- ❌ Testing C++ business logic (test in Qt + C++ tests instead)
- ❌ Hardcoded absolute paths (use `PROJECT_SOURCE_DIR`)
- ❌ Tests that depend on specific window focus or desktop state
- ❌ Excessive timing assumptions (use QTRY_* instead of fixed delays)
- ❌ Global state shared between tests
- ❌ Memory leaks (ensure components are deleted or parentage is clear)
- ❌ Tests that assume specific animation durations
- ❌ Complex QML property bindings that are untestable in unit context

---

## 10. Integration with Main Build

**In `tests/qml/CMakeLists.txt`:**
```cmake
find_package(Qt5 COMPONENTS Core Qml Quick Test REQUIRED)

set(CMAKE_AUTOMOC ON)

# Define QML_ROOT_PATH for test data (if needed)
file(GLOB_RECURSE QML_SOURCES "${CMAKE_SOURCE_DIR}/apps/server/qml/*.qml")

# Add test subdirectories or inline definitions
add_subdirectory(components)
# ... etc
```

**In top-level `tests/CMakeLists.txt`:**
```cmake
add_subdirectory(qml)
```

---

## 11. Debugging Failed QML Tests

### View QML errors:
```cpp
if (component.isError()) {
    for (const auto& err : component.errors()) {
        qWarning() << err.description();
    }
}
```

### Enable QML debugging:
```bash
export QT_DEBUG_QML_MODULE_SYNTAX=1
./build/qml_component_info_overlay -v2
```

### Inspect properties at runtime:
```cpp
// Print all properties of an object
const QMetaObject* meta = obj->metaObject();
for (int i = 0; i < meta->propertyCount(); ++i) {
    QMetaProperty prop = meta->property(i);
    qInfo() << prop.name() << "=" << obj->property(prop.name());
}
```
