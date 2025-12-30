#include <QtTest/QtTest>
#include <QWebSocket>
#include <QEventLoop>
#include <QTimer>
#include "imageserverbridge.h"
#include "control.pb.h"

class TestReapplyFps : public QObject {
    Q_OBJECT

private slots:
    void testReapplyFpsOnReconnect() {
        ImageServerBridge bridge;
        bridge.setPort(0);
        QVERIFY(bridge.start());

        // Set server-wide configured FPS before clients connect
        bridge.setConfiguredFps(30);

        // Connect first client (register spy before opening so messages sent during handshake are captured)
        QWebSocket client;
        QSignalSpy spy(&client, &QWebSocket::binaryMessageReceived);
        QEventLoop loop;
        connect(&client, &QWebSocket::connected, &loop, &QEventLoop::quit);
        client.open(QUrl(QString("ws://127.0.0.1:%1").arg(bridge.serverPort())));
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
        QVERIFY(client.state() == QAbstractSocket::ConnectedState);
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 3000);

        bool foundSetFps = false;
        for (int i = 0; i < spy.count(); ++i) {
            QList<QVariant> args = spy.at(i);
            QByteArray msg = args.at(0).toByteArray();
            if ((unsigned char)msg.at(0) == 0x01) {
                imagesocket::control::ControlMessage cm;
                QVERIFY(cm.ParseFromArray(msg.mid(1).constData(), msg.mid(1).size()));
                if (cm.type() == imagesocket::control::SET_FPS && cm.fps() == 30) {
                    foundSetFps = true;
                    break;
                }
            }
        }
        QVERIFY(foundSetFps);

        // Clear spy and disconnect
        spy.clear();
        QEventLoop loop2;
        connect(&client, &QWebSocket::disconnected, &loop2, &QEventLoop::quit);
        client.close();
        QTimer::singleShot(2000, &loop2, &QEventLoop::quit);
        loop2.exec();

        // Reconnect with a new client (new session/id) - register spy before opening
        QWebSocket client2;
        QSignalSpy spy2(&client2, &QWebSocket::binaryMessageReceived);
        QEventLoop loop3;
        connect(&client2, &QWebSocket::connected, &loop3, &QEventLoop::quit);
        client2.open(QUrl(QString("ws://127.0.0.1:%1").arg(bridge.serverPort())));
        QTimer::singleShot(2000, &loop3, &QEventLoop::quit);
        loop3.exec();
        QVERIFY(client2.state() == QAbstractSocket::ConnectedState);
        QTRY_VERIFY_WITH_TIMEOUT(spy2.count() >= 1, 3000);

        bool foundSetFps2 = false;
        for (int i = 0; i < spy2.count(); ++i) {
            QList<QVariant> args = spy2.at(i);
            QByteArray msg = args.at(0).toByteArray();
            if ((unsigned char)msg.at(0) == 0x01) {
                imagesocket::control::ControlMessage cm;
                QVERIFY(cm.ParseFromArray(msg.mid(1).constData(), msg.mid(1).size()));
                if (cm.type() == imagesocket::control::SET_FPS && cm.fps() == 30) {
                    foundSetFps2 = true;
                    break;
                }
            }
        }
        QVERIFY(foundSetFps2);

        bridge.stop();
    }
};

QTEST_MAIN(TestReapplyFps)
#include "test_reapplyfps.moc"