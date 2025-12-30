#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <qqml.h>

#include "diagnosticsmanager.h"
#include "qmlimageprovider.h"
#include "imageserverbridge.h"
#include "config.h"

/// Server application: Qt-based image streaming server for QML.
///
/// This application creates a QML GUI that:
/// - Hosts an ImageSocketServer listening for TCP connections.
/// - Displays live images received from clients via QML Image element.
/// - Provides UI controls to start/stop the server and view connection status.
///
/// Architecture:
/// - Qt event loop handles all socket I/O and QML updates.
/// - ImageSocketServer manages TCP connections and image decoding.
/// - QML Engine handles presentation (Image element uses QQuickImageProvider).
///
/// Command-line options:
///   --host <address>  Listening address (default: all interfaces)
///   --port <port>     Listening port (default: 5000)
///
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Parse optional command-line arguments for server configuration.
    quint16 port = kDefaultServerPort;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--port" && i + 1 < argc) {
            bool ok = false;
            quint16 parsed = QString::fromLocal8Bit(argv[++i]).toUShort(&ok);
            if (ok) port = parsed;
        }
    }

    // Create the QML application engine (manages the event loop and QML context).
    QQmlApplicationEngine engine;

    // Add resource paths so the QML engine can find the "ImageSocket" module
    // at qrc:/ImageSocket/qmldir
    engine.addImportPath("qrc:/");

    // Create the ImageServerBridge and expose it to QML.
    // We expose it as `imageSocket` to minimize QML changes; it replaces the previous TCP-based server.
    ImageServerBridge *imageBridge = new ImageServerBridge(&engine);

    // Make enums available via ImageServerBridge.<Enum> in QML
    qmlRegisterUncreatableType<ImageServerBridge>("ImageServerBridge", 1, 0, "ImageServerBridge", "Enums only");

    // Set the initial host and port from command-line arguments.
    imageBridge->setPort(port);

    // Create DiagnosticsManager and expose to QML
    DiagnosticsManager *diagnostics = new DiagnosticsManager(&engine);
    engine.rootContext()->setContextProperty("diagnostics", diagnostics);

    // Register the bridge object in the QML context so QML code can access its properties and slots.
    engine.rootContext()->setContextProperty("imageSocket", imageBridge);

    // Register an image provider that reads frames from the bridge.
    engine.addImageProvider("live", new QmlImageProvider(imageBridge));

    // Forward ImageServerBridge events to DiagnosticsManager so logs are captured centrally
    QObject::connect(imageBridge, &ImageServerBridge::eventOccurred, diagnostics,
                     [diagnostics](imagesocket::EventCode code, const QVariantMap &details){
        // Build a friendly message from details when possible
        QString msg = QString("Event %1").arg(static_cast<int>(code));
        if (details.contains("alias")) msg += ": " + details.value("alias").toString();
        else if (details.contains("reason")) msg += ": " + details.value("reason").toString();
        else if (details.contains("client")) msg += ": " + details.value("client").toString();

        // Choose severity based on event code ranges (heuristic)
        DiagnosticsManager::ErrorSeverity severity = DiagnosticsManager::Info;
        int c = static_cast<int>(code);
        if (c >= 1000 && c < 2000) severity = DiagnosticsManager::Info;        // server lifecycle
        else if (c >= 2000 && c < 3000) severity = DiagnosticsManager::Info;   // client connect/disconnect
        else if (c >= 3000 && c < 4000) severity = DiagnosticsManager::Warning; // fps related
        else if (c >= 4000) severity = DiagnosticsManager::Warning;           // frames/errors

        diagnostics->postError(static_cast<int>(code), msg, severity, "ImageServerBridge", details);
    });

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    
    // Connect the objectCreated signal to detect QML loading errors.
    // If the QML file fails to load, objectCreated will be called with nullptr.
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    // Load and compile the main QML file from resources.
    engine.load(url);

    // Verify that the QML file was loaded successfully.
    // rootObjects() returns an empty list if loading failed.
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // Start the Qt event loop. This blocks until QCoreApplication::quit() is called.
    return app.exec();
}
