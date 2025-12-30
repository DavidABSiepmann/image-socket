# Design Decisions

This document records key architectural and technical decisions made during the development of this project. Each decision includes the context, alternatives considered, and consequences.

## Project Context

**This project is a Proof of Concept (PoC)** designed to validate:
- Real-time image streaming from multiple cameras (scalability)
- UI responsiveness for complex dashboards (QML performance)
- Backend ↔ QML communication patterns (architecture feasibility)

**Out of scope:**
- Full-featured video player
- Production-grade streaming optimization
- Commercial deployment

Design decisions reflect PoC goals: explore technology combinations, validate approaches, learn from integration challenges — not to build polished production systems.

**Related documents:**
- [`ARCHITECTURE.md`](ARCHITECTURE.md) — System architecture overview
- [`protobuf_control_protocol.md`](protobuf_control_protocol.md) — Protocol specification
- Testing infrastructure docs — [`testing_infrastructure_std.md`](testing_infrastructure_std.md), [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md), [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md)

---

## 1. WebSocket vs Raw TCP for Communication

### Context
The system needs bidirectional communication between server and clients for control messages (pause, resume, set FPS, etc.) and image streaming. The protocol must support:
- Multiple concurrent clients
- Binary data (JPEG frames)
- Low latency
- Browser compatibility (potential future web clients)

### Decision
**Use WebSocket** (Qt5::WebSockets) as the transport layer, not raw TCP sockets.

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Raw TCP** | Lower overhead, full control over protocol | No browser support, manual framing required, no standard handshake |
| **HTTP/REST** | Simple, widespread tooling | Polling overhead, no push notifications, inefficient for streaming |
| **gRPC** | Built-in Protobuf, streaming support | Heavier stack, HTTP/2 dependency, overkill for this use case |
| **MQTT** | Pub/sub pattern, lightweight | Requires broker, topic-based (not session-based), less suited for control |
| **WebSocket** | Browser-compatible, standard framing, bidirectional, Qt support | Slight overhead vs raw TCP |

### Consequences

**Positive:**
- ✅ Native browser support — future web-based clients can connect without plugins
- ✅ Qt provides `QWebSocket` and `QWebSocketServer` — no need to implement framing manually
- ✅ Standard handshake and upgrade from HTTP simplifies debugging (can inspect with browser DevTools)
- ✅ Binary frames supported out-of-the-box
- ✅ Automatic keep-alive and connection management

**Negative:**
- ❌ Slightly higher overhead than raw TCP (WebSocket frame headers)
- ❌ Requires HTTP handshake before upgrade (adds initial latency)

**Mitigation:**
- WebSocket overhead is negligible for this use case (streaming images > 100KB per frame)
- Initial handshake latency (~10–50 ms) is acceptable for client connection

---

## 2. Protocol Buffers (Protobuf) for Control Messages

### Context
Control messages (pause, resume, set FPS, quality, client ID, alias) need serialization. Requirements:
- Compact encoding (bandwidth efficiency)
- Type safety (enums, required fields)
- Versioning and backward compatibility
- Cross-language support (C++, potentially JavaScript, Python)

### Decision
**Use Protocol Buffers (proto3)** with **lite runtime** (`optimize_for = LITE_RUNTIME`) for control message serialization.

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **JSON** | Human-readable, widespread tooling, easy debugging | Verbose, no schema enforcement, type-unsafe, larger payload |
| **Binary JSON (BSON)** | Compact, typed | Less common, still larger than Protobuf, limited tooling |
| **MessagePack** | Compact, schema-less | No versioning, no type safety, less common in Qt/C++ |
| **Cap'n Proto** | Zero-copy, fast | Less mature, smaller ecosystem, higher learning curve |
| **Protobuf (full)** | Schema, versioning, compact | Larger runtime (200KB+), reflection overhead |
| **Protobuf (lite)** | Schema, versioning, compact, small runtime | No reflection (acceptable for this use case) |

### Consequences

**Positive:**
- ✅ **Compact encoding** — control messages are ~20–100 bytes (vs 200–500 bytes for JSON)
- ✅ **Type safety** — enum validation, required field checks at compile time
- ✅ **Versioning** — forward/backward compatible (new fields ignored by old clients, old fields have defaults)
- ✅ **Code generation** — `protoc` generates C++ classes with type-safe APIs
- ✅ **Cross-language** — same `.proto` file can generate code for Python, JavaScript, Java, etc.
- ✅ **Lite runtime** — small footprint (~50KB) suitable for embedded systems

**Negative:**
- ❌ **Not human-readable** — requires tools to inspect binary messages (e.g., `protoc --decode`)
- ❌ **Build-time dependency** — requires `protoc` compiler in build pipeline
- ❌ **Schema coupling** — server and client must agree on schema version

**Mitigation:**
- Use message prefix (`0x01`) to distinguish control messages from images (allows fallback inspection)
- Version field in schema for future protocol upgrades
- Committed generated `.pb.cc/.pb.h` files as backup when `protoc` is unavailable in CI

---

## 3. Testing Strategy — Layered Tests

### Context
The project has multiple layers (C++ pure logic, Qt backend, QML UI) with different dependencies. Testing all layers in a single test suite would be slow, flaky, and hard to maintain.

### Decision
**Adopt a layered testing strategy** with three distinct test suites:
1. **Pure C++ tests** (`tests/unit/`) — GoogleTest, no Qt dependencies
2. **Qt backend tests** (`tests/qt/`) — QtTest, no QML dependencies
3. **QML UI tests** (`tests/qml/`) — Qt Quick Test, MockBackend fixtures

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Single monolithic test suite** | Simple structure | Slow, flaky, hard to isolate failures |
| **Manual testing only** | No test infrastructure needed | Not repeatable, high regression risk |
| **Integration tests only** | Tests real behavior | Slow, hard to debug, flaky |
| **Unit tests only** | Fast, isolated | Misses integration issues |
| **Layered tests (chosen)** | Fast unit tests, targeted integration tests | More test infrastructure |

### Consequences

**Positive:**
- ✅ **Fast feedback** — C++ tests run in <5 seconds, entire suite in <30 seconds
- ✅ **Isolation** — protocol logic tested without Qt event loop, Qt backend tested without QML rendering
- ✅ **Parallelization** — tests can run concurrently without interference
- ✅ **Focused failures** — easier to identify which layer broke
- ✅ **CI efficiency** — can run C++ tests first (fail fast), then Qt, then QML

**Negative:**
- ❌ **More boilerplate** — three separate test directories, CMake targets
- ❌ **Fixture duplication** — MockBackend, mock sockets, etc.
- ❌ **Integration gaps** — need separate integration tests for end-to-end flows

**Mitigation:**
- Shared fixtures in `tests/*/fixtures/` directories
- Integration tests added for critical paths (server start → client connect → message exchange)
- Documentation for each test layer (`testing_infrastructure_*.md`)

---

## 4. MockBackend for QML Tests

### Context
QML tests need to verify UI behavior (bindings, visibility, animations) without initializing the full backend stack (WebSocket server, image capture, etc.). Real backend setup is slow and introduces non-determinism.

### Decision
**Use MockBackend pattern** — lightweight C++ mock object with Q_PROPERTYs and signals, injected into QML engine via `setContextProperty()`.

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Real backend in tests** | Tests real behavior | Slow (100–500 ms setup), flaky (network, timing), hard to inject states |
| **No C++ backend (QML-only tests)** | Simple | Can't test bindings or signal connections |
| **QML TestCase with inline JS mocks** | Pure QML, no C++ | Hard to maintain, limited control over signals |
| **MockBackend (chosen)** | Fast, deterministic, isolated | Requires maintaining mock class |

### Consequences

**Positive:**
- ✅ **Fast** — QML tests initialize in ~10–50 ms (vs 500+ ms with real backend)
- ✅ **Deterministic** — no network, no threads, no timing dependencies
- ✅ **State injection** — tests can directly set properties (e.g., `backend.m_alias = "Alice"`)
- ✅ **Signal control** — tests trigger signals manually (`emit backend.aliasChanged()`)
- ✅ **Isolated** — QML tests don't depend on backend implementation details

**Negative:**
- ❌ **Maintenance overhead** — MockBackend must be kept in sync with real backend interface
- ❌ **Limited coverage** — tests verify QML behavior, not backend correctness
- ❌ **Potential drift** — mock may diverge from real behavior

**Mitigation:**
- Qt backend tests verify real backend behavior (state machine, signals)
- Integration tests verify QML + real backend interaction
- MockBackend kept minimal (only properties needed for QML tests)

---

## 5. QML Architecture — Data Bindings vs Imperative Updates

### Context
QML UI needs to react to backend state changes (client connected, FPS changed, status message updated). Two main approaches:
1. **Declarative bindings** — QML properties bind directly to C++ properties
2. **Imperative updates** — C++ calls QML methods to update UI

### Decision
**Use declarative data bindings** (one-way, from C++ to QML) with signal connections for events.

**Pattern:**
- C++ properties exposed via Q_PROPERTY (e.g., `serverState`, `activeClientAlias`)
- QML binds to properties: `visible: backend.activeClientAlias !== ""`
- C++ emits signals for events (e.g., `statusMessageChanged`)
- QML connects to signals: `Connections { target: backend; onStatusMessageChanged: ... }`

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Imperative updates (C++ → QML methods)** | Explicit control | Tight coupling, hard to test, brittle (method name changes break) |
| **QML calls C++ for every update** | Simple | Polling overhead, inefficient, not reactive |
| **Two-way bindings** | Automatic sync | Complex, hard to debug, can cause binding loops |
| **Declarative bindings (chosen)** | Reactive, testable, Qt-idiomatic | Initial learning curve |

### Consequences

**Positive:**
- ✅ **Reactive** — UI automatically updates when C++ properties change
- ✅ **Decoupled** — QML doesn't know how backend implements state
- ✅ **Testable** — can verify bindings in QML tests (set property → check QML element)
- ✅ **Qt-idiomatic** — follows Qt best practices
- ✅ **Maintainable** — adding new properties doesn't require QML method calls

**Negative:**
- ❌ **Debugging** — binding errors can be cryptic (e.g., "Cannot read property 'foo' of null")
- ❌ **Learning curve** — developers must understand QML binding system
- ❌ **Property explosion** — many C++ properties needed for granular UI updates

**Mitigation:**
- Use `console.log()` in QML for debugging
- Document binding patterns in `testing_infrastructure_qt_qml.md`
- Keep properties coarse-grained when possible (e.g., single `statusMessage` vs separate `statusType`, `statusText`, `statusColor`)

---

## 6. CI Strategy — GitHub Actions with Matrix Builds

### Context
The project needs automated testing to catch regressions. Requirements:
- Test on Linux (Ubuntu) — primary target platform
- Test with multiple compilers (gcc, clang) — catch compiler-specific issues
- Run all test layers (C++, Qt, QML)
- Fast feedback (<10 minutes)
- Reproducible builds

### Decision
**Use GitHub Actions** with matrix builds (ubuntu-latest × [gcc, clang]).

**Pipeline:**
1. Install dependencies (Qt5, OpenCV, Protobuf, Boost, build tools)
2. Cache ccache to speed up rebuilds
3. Build with CMake + Ninja
4. Run all tests with ctest (xvfb-run for QML tests)

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Travis CI** | Established, good Ubuntu support | Free tier deprecated, slower builds |
| **CircleCI** | Fast, good Docker support | Complex config, paid for private repos |
| **GitLab CI** | Integrated with GitLab, powerful | Would require GitLab migration |
| **Jenkins** | Self-hosted, full control | Requires server maintenance, complex setup |
| **GitHub Actions (chosen)** | Integrated with GitHub, free for public repos, good Ubuntu support | Limited customization vs self-hosted |

### Consequences

**Positive:**
- ✅ **Integrated** — runs automatically on push/PR, status checks in GitHub
- ✅ **Free** — unlimited minutes for public repos
- ✅ **Matrix builds** — tests gcc and clang in parallel
- ✅ **Caching** — ccache reduces rebuild time (10 min → 3 min)
- ✅ **Reproducible** — same environment every run (ubuntu-latest)
- ✅ **xvfb support** — headless QML tests work out-of-the-box

**Negative:**
- ❌ **Ubuntu-only** — no Windows/macOS testing (acceptable for current scope)
- ❌ **No GUI testing** — can't test actual rendering (only behavioral tests)
- ❌ **Dependency bloat** — full Qt5 install adds ~500 MB to cache

**Mitigation:**
- Matrix can be extended to macOS/Windows if needed
- Use ccache to minimize rebuild overhead
- Document CI setup in `.github/workflows/ci.yml` with comments

---

## 7. Protobuf Lite vs Full Runtime

### Context
Protobuf offers two C++ runtimes:
- **Full runtime** — includes reflection, dynamic message parsing, text format
- **Lite runtime** — minimal API, no reflection, smaller binary

The project uses Protobuf for control messages (not large data structures), so reflection is not needed.

### Decision
**Use Protobuf lite runtime** (`optimize_for = LITE_RUNTIME` in `.proto` file).

### Alternatives Considered

| Alternative | Binary Size | Runtime Overhead | Features |
|-------------|-------------|------------------|----------|
| **Full runtime** | ~200 KB | Higher (reflection) | Text format, dynamic messages, reflection |
| **Lite runtime (chosen)** | ~50 KB | Minimal | Serialization only, no reflection |
| **Nanopb (C)** | ~10 KB | Minimal | C implementation, limited C++ ergonomics |

### Consequences

**Positive:**
- ✅ **Smaller binary** — 50 KB vs 200 KB (4× reduction)
- ✅ **Faster** — no reflection overhead
- ✅ **Sufficient** — serialization/deserialization is all we need

**Negative:**
- ❌ **No reflection** — can't inspect messages dynamically (acceptable, not needed)
- ❌ **No text format** — can't use `DebugString()` for logging (use `protoc --decode` instead)

**Mitigation:**
- Use `protoc --decode` for debugging binary messages
- Document message format in `protobuf_control_protocol.md`

---

## 8. CMake vs QMake for Build System

### Context
Qt traditionally uses QMake, but CMake has become the industry standard for cross-platform C++ projects. The project needs:
- Cross-platform builds (Linux, potentially Windows/macOS)
- Integration with Protobuf, OpenCV, Boost
- Modern CMake features (imported targets, generator expressions)

### Decision
**Use CMake 3.5+** as the build system, not QMake.

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **QMake** | Qt's native tool, simpler for Qt-only projects | Limited 3rd-party library support, less flexible |
| **CMake (chosen)** | Industry standard, excellent 3rd-party integration, powerful | Steeper learning curve for Qt beginners |
| **Meson** | Fast, modern | Less common, smaller ecosystem |

### Consequences

**Positive:**
- ✅ **Industry standard** — most C++ developers know CMake
- ✅ **Excellent Qt support** — `find_package(Qt5)` works well
- ✅ **Protobuf integration** — `find_package(Protobuf)` + `protobuf_generate_cpp()`
- ✅ **Modern features** — imported targets (`Qt5::Core`), generator expressions
- ✅ **CI-friendly** — works well with GitHub Actions, Docker

**Negative:**
- ❌ **Verbose** — more boilerplate than QMake for simple Qt apps
- ❌ **Learning curve** — Qt developers may prefer QMake

**Mitigation:**
- Document CMake patterns in `protobuf_installation.md`
- Use modern CMake practices (imported targets, not legacy variables)

---

## 9. Fixture-Based Test Architecture

### Context
Multiple tests need similar setup (mock backend, Qt event loop, WebSocket environment). Repeating setup code in every test is error-prone and hard to maintain.

### Decision
**Use fixture base classes** in `tests/*/fixtures/` to encapsulate common setup/teardown logic.

**Examples:**
- `QtTestBase` — initializes Qt event loop, ensures cleanup with `processEvents()`
- `MockBackend` — lightweight backend mock for QML tests
- `WebSocketTestEnvironment` — sets up WebSocket server for integration tests

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Copy-paste setup in every test** | Simple, explicit | Duplication, hard to maintain |
| **Global setup/teardown** | Shared across all tests | Tests become coupled, hard to isolate |
| **Fixture classes (chosen)** | Reusable, isolated, testable | Initial overhead to create fixtures |

### Consequences

**Positive:**
- ✅ **DRY** — setup code written once, reused
- ✅ **Consistency** — all tests using a fixture have same setup
- ✅ **Testable fixtures** — fixtures themselves can be tested

**Negative:**
- ❌ **Indirection** — developers must learn fixture API
- ❌ **Maintenance** — fixtures must be kept in sync with test needs

**Mitigation:**
- Document fixtures in testing infrastructure docs
- Keep fixtures minimal and focused

---

## 10. Testing Strategy — What NOT to Test

### Context
Tests have a cost (maintenance, CI time). Not everything needs automated testing. Some aspects are better validated manually or not at all.

### Decision
**Do NOT test:**
- Visual appearance (colors, fonts, exact pixel positions)
- Performance (FPS, latency) in unit tests
- Platform-specific rendering (Linux vs Windows)
- OpenCV correctness (assume library works)

**DO test:**
- Behavioral logic (state transitions, signal emissions)
- Bindings and event handlers
- Protocol correctness (serialization, framing)

### Consequences

**Positive:**
- ✅ **Focus** — tests verify logic, not appearance
- ✅ **Fast** — no slow visual regression tests
- ✅ **Stable** — behavioral tests are deterministic

**Negative:**
- ❌ **Limited visual coverage** — regressions in appearance won't be caught
- ❌ **No performance monitoring** — performance regressions possible

**Mitigation:**
- Manual QA for visual appearance
- Benchmarks (future) for performance-critical paths
- Document testing scope in `testing_infrastructure_*.md`

---

## 11. Qt Version Choice — Qt 5.15 for Native Embedded Compilation

### Context
The project targets Raspberry Pi and similar embedded platforms. Qt deployment options:
- **Qt 6.x** — Modern, but requires cross-compilation or commercial licenses for embedded pre-built binaries
- **Qt 5.15 LTS** — Mature, widely available pre-compiled for ARM, no licensing barriers

Goal: validate UI scalability and streaming on real hardware. Cross-compilation complexity and licensing constraints are blockers for PoC validation.

### Decision
**Use Qt 5.15 LTS** for native compilation on Raspberry Pi (apt-get install qt5-default).

### Alternatives Considered

| Alternative | Pros | Cons |
|-------------|------|------|
| **Qt 6.x with official binaries** | Latest features, better performance | Limited embedded pre-builds, may require commercial license |
| **Qt 6.x cross-compiled** | Works on RPi | High complexity, build time 2–4 hours, hard to debug |
| **Qt 5.15 native (chosen)** | Pre-compiled, free, LTS support, simple setup | Older (EOL 2023, patches available), features stable enough for PoC |

### Consequences

**Positive:**
- ✅ **Simple setup** — `apt-get install qt5-*` on RPi, no compilation needed
- ✅ **No licensing barriers** — free for all use cases
- ✅ **Mature ecosystem** — proven on millions of embedded devices
- ✅ **LTS support** — critical bug fixes available (Qt 5.15.x)
- ✅ **Development parity** — desktop tests on Ubuntu match RPi behavior

**Negative:**
- ❌ **EOL status** — Qt 5.15 reached end-of-life in 2023 (not critical for PoC)
- ❌ **No modern Qt 6 features** — some newer APIs unavailable

**Mitigation:**
- If Qt 6 migration needed later: abstractions exist (signals/slots haven't changed fundamentally)
- Security patches available from Qt/KDE projects
- PoC proves concept; production migration to Qt 6 is separate decision

---

## 12. Technology Exploration — Protobuf as Learning Tool

### Context
Control message serialization requires a format. Protobuf was chosen not solely for its technical merit, but as an exploration vehicle:
- Learn protocol design (what fields matter, versioning, compatibility)
- Evaluate Protobuf integration complexity
- Explore code generation workflows
- Prototype cross-language communication

### Decision
**Use Protobuf proto3** for control messages as both a communication layer AND a learning exercise.

**Explicitly not primary goals:**
- Optimize protocol to theoretical minimum
- Achieve peak serialization performance
- Design for future third-party integrations

### Alternatives Considered

| Alternative | Learning Value | Simplicity |
|-------------|-----------------|------------|
| **JSON** | Low (standard, known) | High |
| **Custom binary format** | High | Low |
| **Protobuf (chosen)** | High | Medium |

### Consequences

**Positive:**
- ✅ **Experience gained** — learned Protobuf workflow, code generation, versioning
- ✅ **Practical schema** — control messages are well-specified, testable
- ✅ **Cross-platform ready** — protocol can be reused if project continues
- ✅ **Build integration** — learning CMake + Protobuf is valuable for future projects

**Negative:**
- ❌ **Overkill for PoC scope** — could have used JSON for simplicity
- ❌ **Not optimized** — protocol could be more compact if that were a goal
- ❌ **Maintenance overhead** — schema and generated code must be managed

**Mitigation:**
- Protocol is intentionally simple (11 command types, max ~100 bytes per message)
- Documentation (`protobuf_control_protocol.md`) explains schema and design
- If protocol becomes unnecessary: can be replaced without affecting core architecture

---

## 13. Testing Infrastructure — Exploration, Not Exhaustive Coverage

### Context
A PoC doesn't strictly require comprehensive testing. Extensive test infrastructure (layered tests, fixtures, mocks, Qt Quick Test) was built intentionally for a different purpose: **technology exploration**.

Goals of testing infrastructure:
- Learn GoogleTest patterns and C++ unit testing practices
- Explore QtTest framework for backend testing
- Explore Qt Quick Test for QML testing
- Validate Boost usage in tests
- Demonstrate testing architecture that could scale to larger projects

### Decision
**Invest in layered, well-architected testing infrastructure** as a learning exercise and architectural validation, not as a requirement for shipping.

**Explicit non-goals:**
- 100% code coverage
- Exhaustive edge-case testing
- Performance benchmarking

### Alternatives Considered

| Approach | Testing Coverage | Learning Value | Code Quality |
|----------|-----------------|-----------------|---------------|
| **Manual testing only** | Low | None | Low |
| **Minimal tests** | Medium | Low | Medium |
| **Comprehensive testing (chosen)** | High | High | High |

### Consequences

**Positive:**
- ✅ **Learning platform** — tested C++ unit testing, Qt backend patterns, QML testing approaches
- ✅ **Architecture validation** — layered testing proves separation of concerns works
- ✅ **Reusable patterns** — fixture base classes, mock architectures transferable to other projects
- ✅ **Confidence in code** — tests catch regressions quickly
- ✅ **CI/CD practice** — GitHub Actions setup is production-ready

**Negative:**
- ❌ **Maintenance burden** — tests require upkeep as code evolves
- ❌ **Time investment** — building 3 test layers takes more initial effort
- ❌ **False sense of completeness** — tests are thorough but not exhaustive

**Mitigation:**
- Tests focus on public APIs and critical paths (not every line)
- Fixtures are shared to reduce duplication
- Testing docs guide new developers on what to test and why
- Tests are optional for PoC validation (can be skipped if timeline is tight)

---

---

## Summary Table

| Decision | Chosen Approach | Key Rationale |
|----------|-----------------|---------------|
| **Communication** | WebSocket | Browser compatibility, Qt support, standard framing |
| **Serialization** | Protobuf Lite | Compact, versioned, type-safe; exploration tool |
| **Testing** | Layered (C++, Qt, QML) | Fast, isolated, focused failures; learning platform |
| **QML Tests** | MockBackend | Fast, deterministic, state injection |
| **QML Architecture** | Data bindings | Reactive, testable, Qt-idiomatic |
| **CI** | GitHub Actions (matrix) | Integrated, free, reproducible |
| **Qt Version** | Qt 5.15 LTS | Native RPi compilation, no licensing barriers, simple setup |
| **Build System** | CMake 3.5+ | Industry standard, good Qt/Protobuf support |
| **Protobuf Goal** | Protocol exploration + serialization | Learn design, versioning, integration; not optimization |
| **Fixtures** | Base classes in fixtures/ | DRY, consistency, reusability |

---

## References

- **Architecture:** [`ARCHITECTURE.md`](ARCHITECTURE.md)
- **Protobuf:** [`protobuf_control_protocol.md`](protobuf_control_protocol.md), [`protobuf_installation.md`](protobuf_installation.md)
- **Testing:** [`testing_infrastructure_std.md`](testing_infrastructure_std.md), [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md), [`testing_infrastructure_qt_qml.md`](testing_infrastructure_qt_qml.md)
