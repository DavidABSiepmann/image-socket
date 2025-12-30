# Control Protocol — Protobuf (imagesocket.control)

## Overview

This document describes the control message schema used between client ↔ server in the POC.

Control messages are sent as binary WebSocket frames with a 1-byte prefix (0x01) followed by the bytes of the Protobuf message serialized using the lite runtime.

**Schema location:** [`apps/proto/control.proto`](../apps/proto/control.proto)

**Related documents:**
- [Protobuf installation](protobuf_installation.md) — Installation and CMake integration guide
- [Testing Infrastructure (Qt/Backend)](testing_infrastructure_qt_backend.md) — Serialization and parsing tests

## Schema file
Location: `apps/proto/control.proto`

```proto
syntax = "proto3";
option optimize_for = LITE_RUNTIME;
package imagesocket.control;

enum CommandType {
  UNKNOWN = 0;
  PAUSE = 1;
  RESUME = 2;
  ID = 3;
  REQUEST_RESUME = 4;
  SET_FPS = 5;
  SET_QUALITY = 6;
  SUBSCRIBE = 7;
  UNSUBSCRIBE = 8;
  REQUEST_ALIAS = 9;
  ALIAS = 10;
}

message ControlMessage {
  CommandType type = 1;
  int32 client_id = 2;
  int32 fps = 3;
  int32 quality = 4;
  string reason = 5;
  int64 timestamp_ms = 6;
  string alias = 7;
}
```

## Types definition

### CommandType — Command types

| Value | Name | Direction | Description |
|-------|------|-----------|-------------|
| 0 | `UNKNOWN` | — | Unknown or invalid command |
| 1 | `PAUSE` | Client → Server | Client requests to pause streaming |
| 2 | `RESUME` | Client → Server | Client requests to resume streaming |
| 3 | `ID` | Server → Client | Server sends a unique client ID |
| 4 | `REQUEST_RESUME` | Server → Client | Server asks client to request RESUME |
| 5 | `SET_FPS` | Client → Server | Client adjusts frame rate (value in `fps`) |
| 6 | `SET_QUALITY` | Client → Server | Client adjusts JPEG quality (0–100 in `quality`) |
| 7 | `SUBSCRIBE` | Client → Server | Client starts a streaming subscription |
| 8 | `UNSUBSCRIBE` | Client → Server | Client cancels a streaming subscription |
| 9 | `REQUEST_ALIAS` | Server → Client | Server requests a user-friendly alias from the client |
| 10 | `ALIAS` | Client → Server | Client replies with alias (string in `alias`) |

### ControlMessage — Message fields

| Field | Type | Tag | Required | Description |
|-------|------|-----|----------|-------------|
| `type` | `CommandType` | 1 | ✅ Yes | Command type (enum) |
| `client_id` | `int32` | 2 | ❌ No | Numeric client ID (used in `ID`, `SUBSCRIBE`, etc.) |
| `fps` | `int32` | 3 | ❌ No | Desired frame rate (used with `SET_FPS`), typically 1–60 |
| `quality` | `int32` | 4 | ❌ No | JPEG quality (used with `SET_QUALITY`), range 0–100 |
| `reason` | `string` | 5 | ❌ No | Optional reason text (e.g., error/debug info) |
| `timestamp_ms` | `int64` | 6 | ❌ No | Unix timestamp in milliseconds (optional, for logging/telemetry) |
| `alias` | `string` | 7 | ❌ No | User-friendly client alias (used with `ALIAS`), e.g. "Main Dashboard" |

## WebSocket format

The WebSocket protocol uses a 1-byte prefix to distinguish message types:

- **Control (0x01)**: Binary frame starting with `0x01`, followed by the serialized `ControlMessage` (protobuf-lite)
- **Images (0x00 or no prefix)**: Binary frame containing a JPEG payload

**Implementation note (POC):** Server and client use `0x01` as the control prefix; messages without this prefix are treated as image JPEGs.

### Alias handshake flow

When a client connects, the following flow occurs:

1. **WebSocket connection established**
2. **Server → Client**: `REQUEST_ALIAS` (requests a user-friendly identifier)
3. **Client → Server**: `ALIAS` with the `alias` field set (e.g., "Main Dashboard")
4. Server registers the alias in the `ClientModel` for UI display

This allows the server UI to display friendly names instead of numeric IDs.

## Usage examples

### Example 1: Client sends `RESUME` (C++)

```cpp
// Build the message (control.pb.h generated)
imagesocket::control::ControlMessage cm;
cm.set_type(imagesocket::control::RESUME);
std::string out;
cm.SerializeToString(&out);

QByteArray ba;
ba.append(static_cast<char>(0x01));  // Control prefix
ba.append(out.data(), static_cast<int>(out.size()));
websocket.sendBinaryMessage(ba);
```

### Example 2: Server receives and parses control (C++)

```cpp
void onBinaryMessageReceived(const QByteArray &message) {
  const unsigned char* data = (const unsigned char*)message.constData();
  if (data[0] == 0x01) {
    // Control message
    imagesocket::control::ControlMessage cm;
    if (!cm.ParseFromArray(data + 1, message.size() - 1)) {
      qWarning() << "Failed to parse control message";
      return;
    }
    switch (cm.type()) {
      case imagesocket::control::PAUSE:
        handlePause();
        break;
      case imagesocket::control::RESUME:
        handleResume();
        break;
      case imagesocket::control::SET_FPS:
        setFps(cm.fps());
        break;
      case imagesocket::control::ALIAS:
        setClientAlias(QString::number(cm.client_id()), QString::fromStdString(cm.alias()));
        break;
      // ...
    }
  } else {
    // Handle as JPEG image payload
    handleImageFrame(message);
  }
}
```

### Example 3: Server requests alias (C++)

```cpp
// On client connect
void onClientConnected(QWebSocket* client) {
  imagesocket::control::ControlMessage req;
  req.set_type(imagesocket::control::REQUEST_ALIAS);
  
  std::string serialized;
  if (!req.SerializeToString(&serialized)) {
    qWarning() << "Failed to serialize alias request";
    return;
  }
  
  QByteArray ba;
  ba.append(static_cast<char>(0x01));
  ba.append(serialized.data(), static_cast<int>(serialized.size()));
  client->sendBinaryMessage(ba);
}
```

## Versioning and compatibility

### Schema evolution rules

- Use `proto3` and **reserve numbers** for future fields when removing or repurposing tags
- Protobuf lite maintains binary compatibility with the full runtime
- When adding fields:
  - Use sequential tag numbers (next available)
  - Keep them optional (default in proto3)
  - Unknown fields are ignored by older parsers
- When adding enum values:
  - Use sequential values (next available)
  - Older handlers will treat unknown enum values as `UNKNOWN` (0)

### Compatibility example

**Scenario:** Server updated (knows `REQUEST_ALIAS`/`ALIAS`) + older client (does not know)

- ✅ Server sends `REQUEST_ALIAS` → Older client parses as `UNKNOWN` and ignores it
- ✅ Older client sends `SUBSCRIBE` → New server understands it
- ✅ Additional fields (e.g., `alias`) are ignored by older implementations

**Recommendation:** There is no JSON fallback in this rollout — both sides must implement protobuf-lite.

## Observability and debugging

### Control message logging

Log structured entries to ease debugging and analysis:

```cpp
// On receive
qInfo() << "CONTROL RX id=" << cm.client_id() 
        << "type=" << cm.type() 
        << "ts=" << cm.timestamp_ms()
        << "alias=" << QString::fromStdString(cm.alias());

// On send
qInfo() << "CONTROL TX id=" << client_id 
        << "type=" << type 
        << "ts=" << QDateTime::currentMSecsSinceEpoch();
```

### Latency measurement

Use `timestamp_ms` for measuring command RTT:

1. **Client → Server**: Client sends a command with `timestamp_ms = now()`
2. **Server processes**: Latency = `now() - timestamp_ms`
3. **Server → Client**: Server replies with new `timestamp_ms = now()`
4. **Client computes**: RTT = `now() - timestamp_ms`

### Useful metrics

- **Commands per second** (by type)
- **Average latency** of critical commands (SUBSCRIBE, SET_FPS)
- **Parse error rate** (malformed messages)
- **Distribution of command types** (to identify usage patterns)

## Tests

For protocol validation, see:
- [`tests/unit/protocol/test_protobuf_serialization.cpp`](../tests/unit/protocol/test_protobuf_serialization.cpp) — Unit tests for serialization/deserialization
- [`testing_infrastructure_qt_backend.md`](testing_infrastructure_qt_backend.md) — Backend Qt test coverage

### Important test cases

- ✅ Serialization of all CommandType values
- ✅ Parsing messages with optional fields missing
- ✅ Handling unknown enum values
- ✅ Messages with the correct prefix (0x01)
- ✅ Rejection of malformed messages
- ✅ Boundary fields (quality=0, quality=100, fps=1, fps=60)

## Technical references

- **Protobuf Language Guide (proto3)**: https://protobuf.dev/programming-guides/proto3/
- **C++ Lite Runtime**: https://protobuf.dev/reference/cpp/api-docs/
- **CMake integration**: See [`protobuf_installation.md`](protobuf_installation.md)
- **Schema source**: [`apps/proto/control.proto`](../apps/proto/control.proto)
