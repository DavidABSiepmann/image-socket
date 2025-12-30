#ifndef IMAGESOCKET_EVENTCODES_H
#define IMAGESOCKET_EVENTCODES_H

#include <QVariantMap>

namespace imagesocket {

// Centralized event codes used by backend and UI
enum EventCode {
    // Server (1000-1999)
    ServerStarted = 1000,
    ServerStopped = 1001,
    ServerStartFailed = 1002,
    ServerPortInUse = 1003,
    ServerError = 1099,

    // Clients (2000-2999)
    ClientConnected = 2000,
    ClientDisconnected = 2001,
    ClientBecameActive = 2002,
    NoClientsAvailable = 2003,

    // FPS (3000-3999)
    FpsApplied = 3000,
    FpsFailedNoClient = 3001,
    FpsSendError = 3002,

    // Frames (4000-4999)
    FrameReceived = 4000,
    FrameDropped = 4001,

    // Generic
    UnknownError = 9999
};

} // namespace imagesocket

Q_DECLARE_METATYPE(imagesocket::EventCode)

#endif // IMAGESOCKET_EVENTCODES_H
