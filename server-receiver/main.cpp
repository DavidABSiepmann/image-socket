#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include "imageSocketServer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Create a QML application engine
    QQmlApplicationEngine engine;

    // Create an instance of LiveImageProvider
    ImageSocketServer imageSocket;


    // Set the image provider in the QML engine context
    engine.rootContext()->setContextProperty("imageSocket", &imageSocket);
    engine.addImageProvider("live", &imageSocket);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    // Load the main QML file
    engine.load(url);

    // Check if the QML file is loaded successfully
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
