# Architecture — System Design

## Project Context

**This is a Proof of Concept (PoC)** designed to validate:
- Real-time image streaming from multiple camera sources (scalability)
- UI responsiveness for complex dashboards (QML performance)
- Backend ↔ QML communication patterns (architecture feasibility)

**Scope:** Visualization and control layer for real-time image streams. **Not a video player** or complete media framework.

**Platform constraint:** Targets **Qt 5.15 LTS** for native compilation on Raspberry Pi (avoids cross-compilation, licensing complexity, custom Qt builds).

**Design philosophy:** Prioritizes clarity, feasibility, and learning over completeness or production optimization.

---

## 1. Overview

This project is a real-time image streaming system built on **Qt 5.15** (C++ backend and QML frontend) with **WebSocket** communication and **Protocol Buffers** for control messages.

**Core architecture:** A server manages multiple client connections over WebSocket, tracks per-client state (FPS, quality, alias), and provides a QML UI for monitoring and control.

**Key layers:**
1. **C++ Backend** (QtCore, QtNetwork, QtWebSockets) — state machine, session management, WebSocket server
2. **WebSocket Protocol** — binary frames with Protobuf control messages
3. **QML Frontend** (Qt Quick) — UI layer with data bindings to backend
4. **Testing Infrastructure** — layered tests (C++, Qt, QML) as architectural validation

---

## 2. High-Level Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     QML UI Layer (Qt Quick)                     │
│  ┌─────────────────┬──────────────────┬──────────────────────┐  │
│  │  MainWindow     │  ControlPanel    │  VideoDisplayArea    │  │
│  │  StatusBar      │  InfoOverlay     │  ClientModel (list)  │  │
│  │  ToastNotify    │  Theme           │  ...                 │  │
│  └────────┬────────┴────────┬─────────┴──────────────┬───────┘  │
│           │ Data Bindings   │ Signal Connections     │          │
│           │ (one-way)       │ (slot invocation)      │          │
└───────────┼─────────────────┼────────────────────────┼──────────┘
            │                 │                        │
┌───────────┼─────────────────┼────────────────────────┼─────────┐
│ C++ Backend Layer (Qt Core + Network)                          │
│                     ↓                                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │          ImageServerBridge (State Machine)               │  │
│  │  - Manages server state (Idle, Starting, Running)        │  │
│  │  - Emits stateChanged, statusMessage signals             │  │
│  └──────────────────┬───────────────────────────────────────┘  │
│                     │                                          │
│  ┌──────────────────┴───────────────────────────────────────┐  │
│  │          WebSocketServer (QtWebSockets)                  │  │
│  │  - Accepts WebSocket connections on port 5000            │  │
│  │  - Manages ClientSession instances                       │  │
│  │  - Routes control/image messages                         │  │
│  └──────────────────┬───────────────────────────────────────┘  │
│                     │                                          │
│  ┌──────────────────┴───────────────────────────────────────┐  │
│  │          ClientSession (per-client state)                │  │
│  │  - Tracks connection, FPS, quality, alias                │  │
│  │  - Sends control messages (REQUEST_ALIAS, SET_FPS, etc)  │  │
│  │  - Image streaming (via image provider callback)         │  │
│  └──────────────────────────────────────────────────────────┘  │
│                     ↓                                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │          ClientModel (Qt model for QML)                  │  │
│  │  - Wraps list of connected clients                       │  │
│  │  - Provides roles: alias, clientId, currentFps, etc.     │  │
│  │  - Emits rowsInserted, dataChanged, rowsRemoved          │  │
│  └──────────────────────────────────────────────────────────┘  │
│                     ↓                                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │          LiveImageProvider (QML image provider)          │  │
│  │  - Fetches current frame for QML <Image> display         │  │
│  │  - Caching layer for performance                         │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                │
└────────────────────────────────────────────────────────────────┘
                             ↓
┌──────────────────────────────────────────────────────────────────┐
│             WebSocket Protocol Layer                             │
│                                                                  │
│  Binary frame format:                                            │
│  ┌─────────┬──────────────────────────────────────────────────┐  │
│  │ 0x01    │ Protobuf-serialized ControlMessage               │  │
│  │ (prefix)│ (type, client_id, fps, quality, alias, ...)      │  │
│  └─────────┴──────────────────────────────────────────────────┘  │
│                                                                  │
│  Messages: PAUSE, RESUME, ID, SET_FPS, SET_QUALITY, SUBSCRIBE,   │
│            REQUEST_ALIAS, ALIAS, ...                             │
│            (see protobuf_control_protocol.md)                    │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                             ↓
┌──────────────────────────────────────────────────────────────────┐
│                    Network Transport                             │
│                                                                  │
│  TCP/WebSocket (port 5000)                                       │
│  - Full-duplex communication                                     │
│  - Binary frames (not JSON)                                      │
│  - Multiple concurrent clients                                   │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                             ↓
┌──────────────────────────────────────────────────────────────────┐
│                    Connected Clients                             │
│                   (QWebSocket, native)                           │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 3. Component Responsibilities

### 3.1 QML Frontend (Qt Quick)

**Responsibility:** Visualization layer for application state and user control input.

**Purpose:** Monitor server status, view live image streams, control per-client FPS/quality. **Not a media player framework** — rendering is pragmatic, not media-optimized.

**Components:**
- **MainWindow** — Top-level application window
- **ControlPanel** — Start/stop buttons, FPS/quality sliders
- **VideoDisplayArea** — Live image display via image provider
- **InfoOverlay** — Client alias, FPS, quality display
- **StatusBar** — Application status messages, error display
- **ToastNotification** — Temporary notifications (success, error)
- **Theme** — Centralized color/size constants
- **ClientModel** — List view of connected clients (Qt model from C++)

**Key patterns:**
- **Data binding** — QML properties bound to C++ object properties (one-way, read-only)
- **Signal connections** — QML slots connected to C++ signals (statusMessage, fpsChanged, etc.)
- **Slot invocation** — QML buttons invoke C++ methods (start(), stop(), setFps())
- **Image provider** — QML `<Image>` source resolves to C++ live frame provider

**Design constraints:**
- No business logic (all state in C++)
- No direct WebSocket manipulation
- No raw Protobuf usage
- Pragmatic rendering choices (not optimized for videography use cases)

---

### 3.2 C++ Backend (Qt Core + QtNetwork)

**Responsibility:** State management, session handling, WebSocket server, business logic.

#### 3.2.1 ImageServerBridge
**State machine** that orchestrates the entire server lifecycle.

**States:**
- `Idle` — server not running
- `Starting` — initialization phase
- `Running` — accepting clients and streaming
- `Stopping` — cleanup phase

**Signals:**
- `serverStateChanged(ServerState)` — state transitions
- `connectionStateChanged(ConnectionState)` — client availability change
- `statusMessage(QString)` — human-readable status for UI
- `eventOccurred(EventCode)` — structured event log

**Connections to QML:**
- QML binds to `serverState` property
- QML reacts to `statusMessage` signal (updates StatusBar)
- QML buttons call `start()` and `stop()` slots

#### 3.2.2 WebSocketServer
**Manages WebSocket connections and message routing.**

**Responsibilities:**
- Accept and manage WebSocket connections (Qt5::WebSockets)
- Parse incoming control messages (Protobuf)
- Create/destroy ClientSession instances
- Route image frames to appropriate clients
- Handle client disconnection cleanup

**Signals:**
- `clientConnected(ClientSession*)` — new client connected
- `clientDisconnected(QString clientId)` — client left
- `controlMessageReceived(ControlMessage)` — control message parsed

---

#### 3.2.3 ClientSession
**Per-client state and session management.**

**Responsibilities:**
- Maintain connection to a single client
- Track client properties: alias, FPS, quality, client_id
- Handle REQUEST_ALIAS/ALIAS handshake (ask client for friendly name)
- Send control messages (SET_FPS, SET_QUALITY) to client
- Stream frames to this client only

**Signals:**
- `aliasChanged(QString)` — client provided or updated alias
- `fpsChanged(int)` — client's desired FPS updated
- `connectionLost()` — client disconnected

**Lifetime:**
- Created when client connects
- Destroyed when client disconnects (deleteLater)
- Tracks reference in ClientModel during lifetime

---

#### 3.2.4 ClientModel
**Qt list model exposing connected clients to QML.**

**Responsibility:**
- Maintain list of ClientSession instances
- Provide Qt model interface for QML ListView
- Expose roles: client_id, alias, currentFps, currentQuality, status
- Emit standard model signals (rowsInserted, dataChanged, rowsRemoved)

**Usage in QML:**
```qml
ListView {
    model: clientModel
    delegate: Text { text: model.alias + " (" + model.currentFps + " FPS)" }
}
```

---

#### 3.2.5 LiveImageProvider
**QML image provider for live frame display.**

**Responsibility:**
- Cache current frame in memory
- Respond to QML `image://imageprovider/` URL requests
- Return latest available frame without blocking

**Usage in QML:**
```qml
Image {
    source: "image://liveimageprovider/frame"
}
```

---

### 3.3 WebSocket Protocol Layer

**Responsibility:** Message framing and control message serialization.

**Design intent:** Validate real-time protocol patterns; explore Protobuf integration and binary protocol design. **Not optimized for streaming performance** — design prioritizes clarity and feasibility.

**Protocol:**
- Binary frames over TCP (WebSocket subprotocol)
- **Prefix byte `0x01`** for control messages (Protobuf)
- **Prefix byte `0x00`** for image frames (JPEG)

**Control messages:** Defined in `apps/proto/control.proto` using Protocol Buffers (proto3, lite runtime).

- **Server → Client:** ID, REQUEST_ALIAS, SET_FPS, SET_QUALITY, REQUEST_RESUME
- **Client → Server:** PAUSE, RESUME, SET_FPS, SET_QUALITY, SUBSCRIBE, UNSUBSCRIBE, ALIAS

(See [`protobuf_control_protocol.md`](protobuf_control_protocol.md) for detailed specification.)

**Rationale for Protobuf:**
- **Exploration tool** — learn protocol design, versioning, code generation workflows
- **Type safety** — enums and required fields catch errors early
- **Forward compatibility** — supports future client variations
- **Learning value** — CMake + Protobuf integration experience transferable to other projects

**Not in scope:**
- Performance optimization
- Protocol completeness for production clients
- Advanced serialization features

---

## 4. Data Flow

### 4.1 Initialization Flow

```
User clicks "Start" button (QML)
    ↓
QML calls: backend.start()
    ↓
ImageServerBridge::start() slot
    ↓
    1. Emit stateChanged(Starting)
    2. Create WebSocketServer, bind to port 5000
    3. Emit stateChanged(Running)
    4. Emit statusMessage("Server running")
    ↓
QML binds to serverState property → ControlPanel buttons update
QML reacts to statusMessage signal → StatusBar displays "Server running"
```

### 4.2 Client Connection Flow

```
Client connects to ws://server:5000
    ↓
WebSocketServer accepts connection
    ↓
    1. Create ClientSession(client_id)
    2. Add to ClientModel
    3. Emit clientConnected(ClientSession*)
    ↓
    4. Send ControlMessage(type=REQUEST_ALIAS)
    5. WebSocket sends: 0x01 + protobuf bytes
    ↓
Client receives, parses control message
Client sends ALIAS message back with friendly name
    ↓
ClientSession::onAliasReceived() → emit aliasChanged()
    ↓
ClientModel updates role data
    ↓
QML ListView re-renders (client appears in list with alias)
QML InfoOverlay shows active client alias
```

### 4.3 Control Message Flow (Client changes FPS)

```
User drags FPS slider in QML (e.g., to 24)
    ↓
QML slots: onFpsChanged(24)
    ↓
QML calls: backend.setFps(clientId, 24) C++ slot
    ↓
ClientSession::setFps(24)
    ↓
    1. Store fps = 24
    2. Emit fpsChanged(24)
    3. Send ControlMessage(type=SET_FPS, fps=24) to client
    4. WebSocket sends: 0x01 + protobuf bytes
    ↓
ClientModel::dataChanged() signal
    ↓
QML ListView updates (client row shows new FPS)
QML InfoOverlay refreshes display
```

### 4.4 Image Streaming Flow

```
Backend (native code or OpenCV) captures frame
    ↓
Frame cached in LiveImageProvider
    ↓
QML Image element requests: image://liveimageprovider/frame
    ↓
LiveImageProvider::requestPixmap() returns cached frame
    ↓
    (Separately, each ClientSession sends frame via binary WebSocket frame with 0x00 prefix)
```

---

## 5. Architectural Patterns

### 5.1 State Machine Pattern
**ImageServerBridge** uses explicit state transitions (Idle → Starting → Running → Stopping → Idle) with signal emission at each transition. Enables:
- Clear lifecycle management
- Predictable state combinations
- Prevents invalid operations (e.g., starting twice)

### 5.2 Qt Signals & Slots
**Decouples components** — C++ backend emits signals that QML listens to. Enables:
- Asynchronous communication (no blocking)
- Event-driven UI updates
- Testability via QSignalSpy

### 5.3 Qt Model/View
**ClientModel** wraps business logic into a Qt model interface. Enables:
- Automatic QML ListView synchronization
- Standard signals (rowsInserted, dataChanged, rowsRemoved)
- Consistent Qt programming patterns

### 5.4 Image Provider
**LiveImageProvider** decouples frame source from QML display. Enables:
- Asynchronous frame fetching
- Caching without blocking
- Multiple simultaneous display requests

### 5.5 Protobuf Serialization
**Binary protocol** with schema versioning. Enables:
- Compact encoding (efficient bandwidth)
- Forward/backward compatibility
- Type safety (enum validation, required fields)
- Extensibility (new enum values, new fields with defaults)

### 5.6 Per-Client Session Objects
**ClientSession** encapsulates per-client state. Enables:
- Multiple concurrent clients with isolated state
- Easy cleanup on disconnect (deleteLater)
- Per-client control logic (FPS, quality, alias)

---

## 6. Testing Architecture

**Design intent:** Testing infrastructure serves as a **learning and validation platform**, not a production requirement. Three-layer architecture demonstrates testability at each tier and explores C++/Qt testing practices.

Tests are layered by dependency:

### 6.1 Pure C++ Tests (`tests/unit/`)
**Scope:** Algorithm, protocol, data structures.
- Protobuf serialization/roundtrip
- Byte order conversion, frame parsing
- Backoff calculation, state machine logic

**Purpose:** Learn GoogleTest patterns, C++ unit testing best practices, Boost usage.

**Framework:** GoogleTest (gtest) + GoogleMock (gmock)
**Constraints:** No Qt, no network, no file I/O

(See [`testing_infrastructure_std.md`](testing_infrastructure_std.md))

### 6.2 Qt Backend Tests (`tests/qt/`)
**Scope:** Signals, state transitions, event-loop behavior, models.
- ServerBridge state machine (Idle → Running)
- ClientSession creation/destruction
- ClientModel row insertion/removal
- WebSocket server accepting connections
- Control message parsing and dispatch

**Purpose:** Explore QtTest framework, signal/slot verification patterns, Qt event-loop behavior.

**Framework:** QtTest + QSignalSpy
**Constraints:** No QML, no visual testing

(See [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md))

### 6.3 QML Tests (`tests/qml/`)
**Scope:** QML bindings, animations, event handlers.
- Component visibility based on backend properties
- Property binding from C++ to QML
- Button click handlers invoke C++ slots
- Animations start/end correctly

**Purpose:** Explore Qt Quick Test, QML testing patterns, fixture-based architecture (MockBackend).

**Framework:** Qt Quick Test + MockBackend fixture
**Constraints:** No visual/pixel-perfect testing, no real backend (use MockBackend)

(See [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md))

### 6.4 Integration Tests
**Scope:** Multi-component interactions, end-to-end flows.
- Server start → client connect → message exchange → disconnect
- Theme propagation across all components
- Error handling and recovery

**Framework:** Custom test framework or QTest with full backend

---

## 7. Separation of Concerns

| Layer | Responsibility | Tech Stack |
|-------|-----------------|------------|
| **QML UI** | User interaction, visual layout | Qt Quick, QML |
| **C++ Backend** | State, business logic, events | Qt5 Core, Network, WebSockets |
| **WebSocket Protocol** | Message framing, serialization | Protobuf, TCP/WebSocket |
| **Testing** | Validation at each layer | GoogleTest, QtTest, Qt Quick Test |

**Information flow:**
- **Top → Down:** User actions (QML) → C++ methods → state changes
- **Down → Top:** C++ signals → QML slots → UI updates
- **Bidirectional:** Property bindings (C++ property → QML property)

**Independence:**
- QML does not know about WebSocket details
- C++ backend does not depend on QML rendering
- Tests do not depend on layers above them (C++ tests don't need QML engine)

---

## 8. Architectural Boundaries and Non-Goals

### Scope and Intent

**This architecture is a PoC validation, not production-grade infrastructure.** Design reflects exploration and learning goals:

- ✅ **In scope:** Real-time image streaming validation, UI scalability exploration, communication pattern validation
- ❌ **Not in scope:** Performance optimization, production deployment guarantees, media framework completeness
- ❌ **Not a video player:** No audio sync, no advanced codecs, no playback controls beyond FPS adjustment
- ❌ **Not optimized for:** Large-scale deployments, complex streaming scenarios, cross-platform media rendering

### Key Design Choices

**Why Protobuf?**
- **Exploration:** Learn protocol design, versioning, code generation workflows
- **Type safety:** Enum validation, required fields
- **Cross-language:** Supports future client implementations (C++, Python, JavaScript)
- **Not chosen for:** Performance optimization or complete protocol design

**Why Qt 5.15?**
- **Practical:** Pre-compiled ARM binaries, no licensing barriers
- **Embedded:** Native compilation on Raspberry Pi (no cross-compilation)
- **Stable:** LTS support, proven on millions of devices
- **Not chosen for:** Modern Qt 6 features or latest APIs

**Why Qt Signals/Slots?**
- **Qt-idiomatic:** Follows established Qt patterns
- **Testable:** `QSignalSpy` captures and verifies signals
- **Decoupled:** Components don't call each other directly
- **Asynchronous:** Event-driven, no blocking

**Why MockBackend in QML Tests?**
- **Fast:** No network, no initialization overhead
- **Deterministic:** Easy to inject states for testing
- **Isolated:** QML tests don't depend on C++ backend complexity
- **Focused:** Tests verify QML behavior, not C++ logic

**Why Per-Client ClientSession?**
- **Isolation:** Each client's state is independent
- **Scalability:** Demonstrates multi-client architecture
- **Qt patterns:** Uses Qt object tree (`deleteLater`) for cleanup
- **Testability:** Can test a single session without full server

---

## 9. Future Extensibility

If this PoC proves successful, potential extensions include:

- **Multiple image sources:** Abstract frame source interface (OpenCV, camera, file, remote RTSP)
- **Authentication:** Client authentication (token, TLS, certificate)
- **Compression:** JPEG/H.264 compression options
- **Persistence:** Client session recovery, reconnection logic
- **Clustering:** Multiple servers with client load balancing
- **Mobile clients:** Android/iOS implementations using same Protobuf schema
- **Performance tuning:** Frame rate adaptation, bandwidth throttling
- **Qt 6 migration:** Future update to Qt 6 (architecture supports this)

**Note:** These are possibilities, not current design goals. PoC validates feasibility without attempting these features.

---

## 10. External References

- **Protobuf specification:** [`protobuf_control_protocol.md`](protobuf_control_protocol.md)
- **Protobuf installation:** [`protobuf_installation.md`](protobuf_installation.md)
- **Testing strategy:** [`testing_infrastructure_std.md`](testing_infrastructure_std.md), [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md), [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md)
- **Quick start:** [`QUICKSTART.md`](QUICKSTART.md)
