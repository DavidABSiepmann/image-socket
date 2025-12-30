# Documentation Index

Welcome to the documentation for the **image-socket** project — a real-time image streaming system built on Qt 5 with WebSocket communication and Protocol Buffers.

---

## 🚀 Getting Started

**New to the project?** Start here:

1. **[QUICKSTART.md](QUICKSTART.md)** — Build and run in 5 minutes
   - Prerequisites and environment setup
   - Build instructions (CMake + Ninja)
   - Running the server and tests

---

## 🏗️ Understanding the System

**Want to understand how it works?** Read these in order:

2. **[ARCHITECTURE.md](ARCHITECTURE.md)** — System design and components
   - High-level overview
   - Component diagram (textual)
   - Data flow through the system
   - Key design patterns

3. **[DECISIONS.md](DECISIONS.md)** — Why design choices were made
   - WebSocket vs TCP
   - Protobuf serialization
   - Testing strategy (layered)
   - QML architecture
   - CI/CD approach
   - Trade-offs and consequences

---

## 🔌 Protocol & Communication

**Working with control messages or WebSocket?** See:

4. **[protobuf_control_protocol.md](protobuf_control_protocol.md)** — Control message specification
   - Protocol schema (Protobuf proto3)
   - CommandType enum (11 values: PAUSE, RESUME, SET_FPS, etc.)
   - ControlMessage fields
   - Message flow examples (C++)
   - Versioning and compatibility rules

5. **[protobuf_installation.md](protobuf_installation.md)** — Protobuf setup and integration
   - Installation options (apt, vcpkg, Conan, from source)
   - Lite vs Full runtime comparison
   - CMake integration patterns
   - Build-time vs runtime dependencies

---

## 🧪 Testing Infrastructure

**Writing or understanding tests?** See the testing docs:

6. **[testing_infrastructure_std.md](testing_infrastructure_std.md)** — Pure C++ unit tests
   - Protocol and serialization tests (Protobuf)
   - Data structure and parsing tests
   - Client logic tests (state machine, backoff)
   - Smoke tests (linkage, basic API)
   - GoogleTest framework usage
   - **No Qt dependencies**

7. **[testing_infrastructure_qt_backend.md](testing_infrastructure_qt_backend.md)** — Qt C++ component tests
   - State machine and transition tests
   - Signal emission and slot verification
   - Qt model tests (ClientModel)
   - WebSocket integration tests
   - Event loop and lifetime management
   - QtTest framework (QSignalSpy, QTRY_* macros)
   - **No QML, event-loop based**

8. **[testing_infrastructure_qt_qml.md](testing_infrastructure_qt_qml.md)** — QML UI component tests
   - Component visibility and state bindings
   - Data binding verification
   - Event handling (buttons, keyboard, mouse)
   - Custom component integration
   - Animation testing
   - MockBackend fixture architecture
   - Qt Quick Test framework
   - **No visual/pixel-perfect testing**

---

## 📋 Document Structure

| Document | Audience | Purpose | Length |
|----------|----------|---------|--------|
| QUICKSTART.md | All developers | Get running fast | 5 min |
| ARCHITECTURE.md | Architects, senior devs | Understand design | 15 min |
| DECISIONS.md | Design reviews, new devs | Know the "why" | 20 min |
| protobuf_control_protocol.md | Protocol devs, integrators | Message spec | 10 min |
| protobuf_installation.md | Build/infra engineers | Protobuf setup | 10 min |
| testing_infrastructure_std.md | C++ test writers | Unit test patterns | 15 min |
| testing_infrastructure_qt_backend.md | Qt test writers | Qt backend tests | 20 min |
| testing_infrastructure_qt_qml.md | QML developers | QML test patterns | 25 min |

---

## 🎯 By Role

### I'm a new developer, starting fresh
1. [`QUICKSTART.md`](QUICKSTART.md) — Get running
2. [`ARCHITECTURE.md`](ARCHITECTURE.md) — Understand structure
3. Role-specific docs below

### I'm working on C++ backend logic
- [`testing_infrastructure_std.md`](testing_infrastructure_std.md) — Write unit tests
- [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) — Test Qt integration
- [`ARCHITECTURE.md`](ARCHITECTURE.md) — Component responsibilities

### I'm working on QML UI
- [`ARCHITECTURE.md`](ARCHITECTURE.md#31-qml-frontend-qt-quick) — QML component overview
- [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md) — Test patterns
- [`protobuf_control_protocol.md`](protobuf_control_protocol.md) — Message definitions

### I'm setting up the build/CI pipeline
- [`QUICKSTART.md`](QUICKSTART.md#prerequisites) — Dependencies
- [`protobuf_installation.md`](protobuf_installation.md) — Protobuf integration
- [`DECISIONS.md`](DECISIONS.md#6-ci-strategy--github-actions-with-matrix-builds) — Why GitHub Actions

### I'm integrating an external client
- [`protobuf_control_protocol.md`](protobuf_control_protocol.md) — Protocol spec
- [`ARCHITECTURE.md`](ARCHITECTURE.md#3-component-responsibilities) — Backend design
- [`DECISIONS.md`](DECISIONS.md#2-protocol-buffers-protobuf-for-control-messages) — Why Protobuf

### I'm reviewing architecture decisions
- [`DECISIONS.md`](DECISIONS.md) — All decisions with trade-offs
- [`ARCHITECTURE.md`](ARCHITECTURE.md#8-key-design-decisions) — Design rationale

---

## 📚 Topic Cross-Reference

**Need info on a specific topic?**

### WebSocket Communication
- [`ARCHITECTURE.md`](ARCHITECTURE.md#33-websocket-protocol-layer) — Protocol layer
- [`DECISIONS.md`](DECISIONS.md#1-websocket-vs-raw-tcp-for-communication) — Why WebSocket

### Protocol Buffers & Serialization
- [`protobuf_control_protocol.md`](protobuf_control_protocol.md) — Message schema
- [`protobuf_installation.md`](protobuf_installation.md) — Setup
- [`DECISIONS.md`](DECISIONS.md#2-protocol-buffers-protobuf-for-control-messages) — Why Protobuf
- [`DECISIONS.md`](DECISIONS.md#7-protobuf-lite-vs-full-runtime) — Lite runtime choice
- [`testing_infrastructure_std.md`](testing_infrastructure_std.md#31-protocol-tests-unit_protocol_) — Protocol tests

### Client Session Management
- [`ARCHITECTURE.md`](ARCHITECTURE.md#323-clientsession) — Design
- [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md#31-state-transition-tests-qt_state_) — State tests

### Data Binding (QML ↔ C++)
- [`ARCHITECTURE.md`](ARCHITECTURE.md#32-c-backend-qt-core--qtnetwork) — Component design
- [`DECISIONS.md`](DECISIONS.md#5-qml-architecture--data-bindings-vs-imperative-updates) — Design choice
- [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md#32-data-binding-tests-qml_binding_) — Binding tests

### Testing & Quality
- [`testing_infrastructure_std.md`](testing_infrastructure_std.md) — Pure C++ tests
- [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) — Qt tests
- [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md) — QML tests
- [`DECISIONS.md`](DECISIONS.md#3-testing-strategy--layered-tests) — Testing strategy
- [`DECISIONS.md`](DECISIONS.md#10-testing-strategy--what-not-to-test) — Testing scope

### Build System & CI
- [`QUICKSTART.md`](QUICKSTART.md#build) — CMake build
- [`protobuf_installation.md`](protobuf_installation.md#cmake-integration-recommended-pattern) — CMake + Protobuf
- [`DECISIONS.md`](DECISIONS.md#8-cmake-vs-qmake-for-build-system) — Why CMake
- [`DECISIONS.md`](DECISIONS.md#6-ci-strategy--github-actions-with-matrix-builds) — CI strategy

---

## 🔗 Quick Links

**Related resources:**
- **Source code:** `src/` — Implementation
- **Build:** `CMakeLists.txt`, `src/CMakeLists.txt`, `tests/CMakeLists.txt`
- **Protocol:** `apps/proto/control.proto`
- **QML:** `apps/server/qml/` — UI components
- **Tests:** `tests/unit/`, `tests/qt/`, `tests/qml/`
- **CI:** `.github/workflows/ci.yml` — GitHub Actions

---

## 📖 Reading Recommendations

### 15-minute Overview
1. [`QUICKSTART.md`](QUICKSTART.md)
2. [`ARCHITECTURE.md`](ARCHITECTURE.md) (sections 1–3)

### 1-hour Deep Dive
1. [`ARCHITECTURE.md`](ARCHITECTURE.md)
2. [`DECISIONS.md`](DECISIONS.md)
3. [`protobuf_control_protocol.md`](protobuf_control_protocol.md)

### For a Code Review
1. [`ARCHITECTURE.md`](ARCHITECTURE.md#7-separation-of-concerns)
2. [`DECISIONS.md`](DECISIONS.md)
3. Role-specific testing docs

---

## ❓ FAQ

**Where do I start?**
→ Read [`QUICKSTART.md`](QUICKSTART.md), then [`ARCHITECTURE.md`](ARCHITECTURE.md)

**How do I add a new test?**
→ See the appropriate testing infrastructure doc (std/qt/qml)

**Why was decision X made?**
→ Check [`DECISIONS.md`](DECISIONS.md)

**How do I integrate an external client?**
→ Read [`protobuf_control_protocol.md`](protobuf_control_protocol.md)

**Why Protobuf instead of JSON?**
→ [`DECISIONS.md`](DECISIONS.md#2-protocol-buffers-protobuf-for-control-messages)

**Why WebSocket instead of raw TCP?**
→ [`DECISIONS.md`](DECISIONS.md#1-websocket-vs-raw-tcp-for-communication)

**What should I test and what shouldn't I?**
→ [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md#2-scope) (and other testing docs)

**How does data flow from C++ to QML?**
→ [`ARCHITECTURE.md`](ARCHITECTURE.md#4-data-flow)




