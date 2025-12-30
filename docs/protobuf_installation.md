# Protocol Buffers (Protobuf) — Quick Install & Integration Guide

## Overview

This short guide explains how to install and integrate Protocol Buffers for development and CI in this repository. It focuses on the project's needs: `proto3` schemas and the **lite runtime** (`optimize_for = LITE_RUNTIME`) used by `apps/proto/control.proto`.

**Note:** This document assumes a Linux environment (commands are tested on Debian/Ubuntu). Adjust package manager commands for other distributions.

## Recommended Protobuf versions

- Minimum compatible: **protoc >= 3.6** (basic proto3 features).
- Recommended: **protoc >= 3.12** for broader bug fixes and improved C++ support.
- If you need newer language features, use the latest 3.x available.

## Install options (copy-paste safe)

### A) Debian / Ubuntu (example)

```bash
sudo apt-get update
sudo apt-get install -y protobuf-compiler libprotobuf-dev
protoc --version  # check installed version
```

> The distro packages may be older than the recommended version; if you need a newer `protoc`, use vcpkg, Conan or build from source.

### B) vcpkg (cross-platform)

```bash
./vcpkg install protobuf
# for the lite runtime variant (if supported):
./vcpkg install protobuf[core]
```

### C) Conan

Declare `protobuf/x.y` in your `conanfile` and let Conan manage versions for reproducible builds.

### D) Build from source (when you need a specific version)

```bash
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
mkdir build && cd build
cmake -Dprotobuf_BUILD_TESTS=OFF ..
cmake --build . -- -j$(nproc)
sudo cmake --install .
```

## Lite vs Full vs Nanopb

- **Full C++**: Full API and reflection, larger binaries.
- **Lite**: Smaller runtime (recommended for this project). Use `--cpp_out=lite` when generating sources.
- **Nanopb**: C implementation for microcontrollers (not used here).

This project uses `optimize_for = LITE_RUNTIME` in `apps/proto/control.proto`.

## Build-time vs Runtime

- **protoc (compiler)** is a build-time tool: it generates `.pb.cc`/`.pb.h` sources from `.proto` files.
- **libprotobuf / libprotobuf-lite** is a runtime dependency required when linking the generated code.
- If `protoc` is not available in CI, you may temporarily commit the generated `.pb.cc/.pb.h` files, but prefer generating them during the build for reproducibility.

## CMake integration (recommended pattern)

Recommended pattern for `CMakeLists.txt` inside the submodule (copy-paste safe):

```cmake
find_package(Protobuf REQUIRED)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_GEN_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
  OUTPUT ${PROTO_GEN_DIR}/control.pb.cc ${PROTO_GEN_DIR}/control.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE} --cpp_out=lite:${PROTO_GEN_DIR} -I ${PROTO_DIR} ${PROTO_DIR}/control.proto
  DEPENDS ${PROTO_DIR}/control.proto
)

add_custom_target(proto_imagesocket_generated DEPENDS ${PROTO_GEN_DIR}/control.pb.cc ${PROTO_GEN_DIR}/control.pb.h)

# Example: link generated sources into a target
add_executable(my_consumer src/main.cpp ${PROTO_GEN_DIR}/control.pb.cc)
add_dependencies(my_consumer proto_imagesocket_generated)
target_include_directories(my_consumer PRIVATE ${PROTO_GEN_DIR})
# Prefer imported target if available
if (TARGET protobuf::libprotobuf-lite)
  target_link_libraries(my_consumer PRIVATE protobuf::libprotobuf-lite)
else()
  target_link_libraries(my_consumer PRIVATE protobuf)
endif()
```

Project specifics: the generated sources for `imagesocket` are placed in `${CMAKE_BINARY_DIR}/src/imagesocket/generated` and the project defines a target `proto_imagesocket_generated` (see `src/imagesocket/CMakeLists.txt`). Make your test and consumer targets depend on that target and add the generated directory to their include paths.

## CI notes

- Ensure `protoc` is installed on CI runners (apt, vcpkg, conan, or build from source).
- Committing generated files (`control.pb.cc/h`) is allowed as a temporary workaround when `protoc` is not available, but prefer generating them in CI to keep builds reproducible.

## Quick verification

```bash
cmake -S . -B build -G Ninja
cmake --build build --target proto_imagesocket_generated
# Verify files exist (example):
ls build/src/imagesocket/generated/control.pb.*
```

## References

- Protobuf language guide: https://protobuf.dev/programming-guides/proto3/
- Protobuf C++ reference: https://protobuf.dev/reference/cpp/
