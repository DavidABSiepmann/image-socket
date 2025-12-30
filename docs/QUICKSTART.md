# Quick Start Guide

Get the project built and running in ~5 minutes.

---

## Prerequisites

**Operating System:** Ubuntu 20.04+ or Debian (Linux required for now)

**Tools:**
```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build \
  qt5-qmake qt5-qmake qtbase5-dev qtdeclarative5-dev qtwebsockets5-dev qttools5-dev \
  libopencv-dev libboost-system-dev \
  protobuf-compiler libprotobuf-dev \
  xvfb  # For headless QML testing
```

**Check versions:**
```bash
cmake --version       # 3.5+
protoc --version      # 3.6+
qmake --version       # Qt 5.x
```

---

## Build

```bash
cd /home/david/Projetos/image-socket

# Configure
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..

# Compile (8 threads, adjust as needed)
ninja -j 8

# Done
echo "Build successful: $(ls bin/server 2>/dev/null && echo 'server binary found')"
```

**Expected time:** 2–5 minutes (first build), 30 seconds (incremental)

---

## Run Server

```bash
cd build
./bin/server
```

**Expected output:**
```
Server running on port 5000
Waiting for clients...
```

**In another terminal, connect a client:**
```bash
# With qwebsocket (example, if available)
# Or use any WebSocket client library (Python, Node.js, etc.)
```

Stop server: `Ctrl+C`

---

## Run Tests

### All tests (recommended first time)
```bash
cd build
ctest --output-on-failure
```

### By category

**C++ unit tests only** (fast, ~5 seconds):
```bash
ctest -R "^unit_" --output-on-failure
```

**Qt backend tests** (medium, ~10 seconds):
```bash
ctest -R "^qt_" --output-on-failure
```

**QML UI tests** (slower, ~15 seconds, requires xvfb):
```bash
ctest -R "^qml_" --output-on-failure
```

**Run with verbose output:**
```bash
ctest -VV --output-on-failure
```

---

## Verify Installation

```bash
# Check Protobuf setup
protoc --version

# Check Qt
qmake --version

# Check build artifacts
ls -la build/bin/server
ls -la build/lib/libimagesocket.a  # (or .so)
```

---

## Next Steps

- **Architecture:** Read [`docs/ARCHITECTURE.md`](ARCHITECTURE.md) for system design
- **Deeper setup:** [`protobuf_installation.md`](protobuf_installation.md), [`testing_infrastructure_std.md`](testing_infrastructure_std.md)
- **Design decisions:** [`docs/DECISIONS.md`](DECISIONS.md)
- **Protocol spec:** [`docs/protobuf_control_protocol.md`](protobuf_control_protocol.md)

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `cmake: not found` | `sudo apt-get install cmake` |
| `Qt5 headers not found` | `sudo apt-get install qtbase5-dev qtdeclarative5-dev` |
| `protoc: not found` | `sudo apt-get install protobuf-compiler` |
| `ninja: not found` | `sudo apt-get install ninja-build` (or use `cmake --build .`) |
| `Tests fail on QML` | Install xvfb: `sudo apt-get install xvfb` |
| `Slow build` | Use `ninja -j $(nproc)` for parallel build |

---

## Common Commands Reference

```bash
# Clean build
rm -rf build && mkdir build && cd build && cmake -G Ninja .. && ninja

# Rebuild single target
ninja server   # or: ninja qml_component_info_overlay

# Run single test
ctest -R "test_name" -VV

# Debug test with gdb
gdb ./bin/unit_protocol_serialization

# Check dependencies
ldd ./bin/server | grep -E 'Qt|boost|protobuf'
```

---

## Environment Variables (optional)

```bash
# Speed up compilation with ccache
export CMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -G Ninja ..

# Parallel jobs
export NINJA_STATUS="[%f/%t %es] "  # Fancy progress
ninja -j $(nproc)
```

---

**Questions?** Check [`docs/`](../) directory for detailed documentation.
