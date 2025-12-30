# Image Socket — Real-Time Streaming PoC

[![CI](https://github.com/username/image-socket/actions/workflows/ci.yml/badge.svg)](https://github.com/username/image-socket/actions/workflows/ci.yml)

A **Proof of Concept** validating real-time image streaming architecture with Qt 5.15, WebSocket, Protocol Buffers, and QML.

---

## Project Scope & Intent

**This is a PoC, not a production application.**

### What this project IS ✅

- **Feasibility test** for real-time image streaming from multiple camera sources
- **Architecture validation** for Qt backend ↔ QML communication patterns
- **Technology exploration:**
  - Qt 5.15 native compilation on Raspberry Pi
  - Protocol Buffers for message serialization
  - WebSocket for client-server communication
  - Layered testing (C++, Qt, QML)
- **Learning project** documenting design decisions and trade-offs

### What this project IS NOT ❌

- ❌ **Not a video player** — no audio sync, limited codec support, basic playback controls
- ❌ **Not production-ready** — not optimized for performance, bandwidth, or deployment
- ❌ **Not a media framework** — UI is pragmatic, not media-optimized
- ❌ **Not feature-complete** — intentionally scoped for validation, not completeness

### Why these technologies?

| Technology | Reason | Not chosen for |
|------------|--------|----------------|
| **Qt 5.15 LTS** | Native ARM binaries for Raspberry Pi, no cross-compilation | Latest Qt 6 features |
| **Protobuf** | Explore protocol design, versioning, code generation | Performance optimization |
| **WebSocket** | Browser compatibility, validate binary framing | Raw TCP performance |
| **Extensive testing** | Learn GoogleTest, QtTest, Qt Quick Test patterns | Production coverage requirements |

---

## Quick Start

**New to this project?** Get running in 5 minutes:

👉 **[QUICKSTART.md](docs/QUICKSTART.md)** — Build, run, and test

---

## Documentation

Comprehensive documentation is available in [`docs/`](docs/):

| Document | Purpose | Audience |
|----------|---------|----------|
| **[docs/README.md](docs/README.md)** | 📚 **Documentation hub** — navigation and index | All developers |
| **[QUICKSTART.md](docs/QUICKSTART.md)** | 🚀 **Get running fast** — build, test, verify | New developers |
| **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** | 🏗️ **System design** — components, data flows, patterns | Architects, reviewers |
| **[DECISIONS.md](docs/DECISIONS.md)** | 🎯 **Design rationale** — why choices were made | Design reviewers |
| **[protobuf_control_protocol.md](docs/protobuf_control_protocol.md)** | 🔌 **Protocol spec** — message schema, examples | Protocol integrators |

**Start here:**
1. Read [QUICKSTART.md](docs/QUICKSTART.md) to build and run
2. Read [ARCHITECTURE.md](docs/ARCHITECTURE.md) to understand design
3. Read [DECISIONS.md](docs/DECISIONS.md) to learn "why"

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                  QML UI (Qt Quick)                      │
│  • Visualization layer (not a video player)             │
│  • Data bindings from C++ backend                       │
│  • Client monitoring, FPS/quality control               │
└────────────────────┬────────────────────────────────────┘
                     │ Signals/Slots + Property Bindings
┌────────────────────┴────────────────────────────────────┐
│              C++ Backend (Qt 5.15)                      │
│  • ImageServerBridge (state machine)                    │
│  • WebSocketServer (multi-client sessions)              │
│  • ClientSession (per-client state: FPS, alias)         │
│  • ClientModel (Qt model for QML ListView)              │
└────────────────────┬────────────────────────────────────┘
                     │ Binary WebSocket frames
┌────────────────────┴────────────────────────────────────┐
│         WebSocket Protocol (Protobuf Lite)              │
│  • Control messages: SET_FPS, ALIAS, PAUSE, RESUME      │
│  • Image frames: JPEG binary data                       │
└─────────────────────────────────────────────────────────┘
                     │ TCP/WebSocket
┌────────────────────┴────────────────────────────────────┐
│              Connected Clients (Native)                 │
└─────────────────────────────────────────────────────────┘
```

**See [ARCHITECTURE.md](docs/ARCHITECTURE.md) for complete component breakdown.**

---

## Key Features

- ✅ **Multi-client support** — WebSocket server handles multiple concurrent clients
- ✅ **Per-client control** — FPS, quality, pause/resume per client
- ✅ **Client aliasing** — REQUEST_ALIAS handshake for friendly names
- ✅ **Protobuf control messages** — type-safe, versioned binary protocol
- ✅ **QML visualization** — responsive UI with data bindings
- ✅ **Layered testing** — Pure C++, Qt backend, QML tests (GoogleTest, QtTest, Qt Quick Test)
- ✅ **CI/CD** — GitHub Actions with matrix builds (gcc, clang)

**What's intentionally missing:**
- ❌ Audio streaming
- ❌ Advanced video codecs (H.264 encode/decode)
- ❌ Authentication/encryption
- ❌ Performance optimization
- ❌ Cross-platform builds (Windows, macOS)

---

## Building

### Prerequisites

- **OS:** Ubuntu 20.04+ or Debian-based Linux
- **Qt:** 5.15.x (qtbase5-dev, libqt5websockets5-dev)
- **Build tools:** CMake 3.5+, Ninja, gcc/g++ or clang
- **Libraries:** OpenCV 4.x, Boost 1.65+, Protobuf 3.6+

**Quick install (Ubuntu/Debian):**

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake ninja-build \
    qtbase5-dev libqt5websockets5-dev qt5-qmake \
    libopencv-dev libboost-all-dev \
    libprotobuf-dev protobuf-compiler
```

### Build Steps

```bash
# Clone repository
git clone https://github.com/username/image-socket.git
cd image-socket

# Configure with CMake + Ninja
mkdir build && cd build
cmake -G Ninja ..

# Build all targets
ninja

# Run server
./apps/server/server-receiver

# Run client (in another terminal)
./apps/client/send_image_client --video /path/to/video.mp4

# Run tests
ctest --output-on-failure
```

**See [QUICKSTART.md](docs/QUICKSTART.md) for detailed instructions and troubleshooting.**

---

## Running

### Server

Start the server (binds to `0.0.0.0:5000` by default):

```bash
./apps/server/server-receiver

# Or with custom host/port:
./apps/server/server-receiver --port 6000
```

Then click **Start** in the QML UI to begin accepting clients.

### Client

Send images from a video file:

```bash
./apps/client/send_image_client --video /path/to/video.mp4

# Or connect to remote server:
./apps/client/send_image_client --server 192.168.1.100 --port 6000 --video video.mp4
```

Client will:
1. Connect to server via WebSocket
2. Perform ALIAS handshake (server requests alias, client responds)
3. Stream JPEG frames at configured FPS
4. Respond to control messages (SET_FPS, PAUSE, RESUME)

---

## Testing

Three test layers validate architecture:

```bash
# Run all tests
ctest --output-on-failure

# Run specific layers
ctest -R unit_       # Pure C++ tests (GoogleTest)
ctest -R qt_         # Qt backend tests (QtTest)
ctest -R qml_        # QML UI tests (Qt Quick Test)
```

**Test coverage:**
- **C++ unit tests** — Protobuf serialization, protocol parsing, state machine logic
- **Qt backend tests** — Signal emissions, state transitions, ClientModel, WebSocket
- **QML tests** — Data bindings, component visibility, event handlers (uses MockBackend)

**See [testing_infrastructure_*.md](docs/) for patterns and best practices.**

---

## Technology Stack

| Layer | Technologies |
|-------|--------------|
| **UI** | Qt 5.15 QML (Qt Quick) |
| **Backend** | C++11, Qt5 Core/Network/WebSockets |
| **Protocol** | WebSocket, Protocol Buffers (proto3, lite runtime) |
| **Video** | OpenCV 4.x (frame capture, JPEG encoding) |
| **Networking** | Boost.Asio (client), Qt WebSockets (server) |
| **Testing** | GoogleTest, QtTest, Qt Quick Test |
| **Build** | CMake 3.5+, Ninja |
| **CI** | GitHub Actions (Ubuntu, gcc/clang matrix) |

---

## Project Structure

```
image-socket/
├── apps/
│   ├── client/          # Client application (Boost + OpenCV)
│   ├── server/          # Server application (Qt + QML)
│   └── proto/           # Protobuf schema (control.proto)
├── src/
│   ├── imagesocket/     # Core library (client/server)
│   └── network/         # Network utilities
├── tests/
│   ├── unit/            # Pure C++ tests (GoogleTest)
│   ├── qt/              # Qt backend tests (QtTest)
│   └── qml/             # QML UI tests (Qt Quick Test)
├── docs/                # Comprehensive documentation
│   ├── README.md        # Documentation hub
│   ├── QUICKSTART.md    # Get running fast
│   ├── ARCHITECTURE.md  # System design
│   ├── DECISIONS.md     # Design rationale
│   └── testing_*.md     # Testing guides
├── CMakeLists.txt       # Top-level build
└── README.MD            # This file
```

---

## Platform Support

**Primary target:** Raspberry Pi (ARMv7/ARMv8) with native Qt 5.15 compilation.

**Also tested on:**
- Ubuntu 20.04 / 22.04 (x86_64)
- Debian 11+ (x86_64)

**Not tested / not supported:**
- Windows (would require Qt 5.15 Windows build + MSVC adjustments)
- macOS (would require Qt 5.15 macOS build + Homebrew setup)
- Embedded Linux (other than Raspberry Pi OS)

---

## Intended Audience

This project is intended for:

- **Engineers** evaluating real-time streaming architectures
- **Qt developers** exploring C++ ↔ QML integration patterns
- **Students** learning Protocol Buffers, WebSocket, or testing practices
- **Portfolio reviewers** assessing design decisions and documentation quality

**Not intended for:**
- End users seeking a video player application
- Production deployments requiring performance or security guarantees
- Projects needing cross-platform or commercial Qt support

---

## Contributing

This is a personal learning project. Contributions are not expected, but feedback is welcome via Issues.

If you found this project useful for learning or reference:
- ⭐ Star the repo
- 📖 Reference the documentation in your own projects
- 💬 Open an issue to share what you learned or built

---

## License

MIT License — See [LICENSE](LICENSE) for details.

---

## References

**Documentation:**
- [Full documentation index](docs/README.md)
- [System architecture](docs/ARCHITECTURE.md)
- [Design decisions](docs/DECISIONS.md)
- [Protobuf protocol](docs/protobuf_control_protocol.md)

**External links:**
- [Qt 5.15 Documentation](https://doc.qt.io/qt-5.15/)
- [Protocol Buffers (proto3)](https://protobuf.dev/programming-guides/proto3/)
- [GoogleTest User Guide](https://google.github.io/googletest/)
- [Qt Test Framework](https://doc.qt.io/qt-5/qtest.html)
