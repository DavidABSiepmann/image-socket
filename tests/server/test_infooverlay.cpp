#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QWebSocket>
#include <QEventLoop>
#include <QTimer>
#include "imageserverbridge.h"
#include "control.pb.h"

class TestInfoOverlay : public QObject
{
    Q_OBJECT

private slots:
    void testCurrentFpsAndAlias()
    {
        ImageServerBridge bridge;
        bridge.setPort(0);

        QVERIFY(bridge.start());
        quint16 port = bridge.serverPort();
        QVERIFY(port > 0);

        QWebSocket client;
        QEventLoop loop;
        QObject::connect(&client, &QWebSocket::connected, &loop, &QEventLoop::quit);
        client.open(QUrl(QString("ws://127.0.0.1:%1").arg(port)));
        QTimer::singleShot(3000, &loop, &QEventLoop::quit); // timeout safety
        loop.exec();
        QVERIFY(client.state() == QAbstractSocket::ConnectedState);

        // Prepare alias control message
        imagesocket::control::ControlMessage aliasMsg;
        aliasMsg.set_type(imagesocket::control::ALIAS);
        aliasMsg.set_alias("Alice");
        std::string out;
        aliasMsg.SerializeToString(&out);
        QByteArray ba;
        ba.append(char(0x01));
        ba.append(out.data(), (int)out.size());

        // Spy for activeClientAlias change
        QSignalSpy spyAlias(&bridge, &ImageServerBridge::activeClientAliasChanged);

        // Send alias message and wait for bridge to pick it up
        client.sendBinaryMessage(ba);

        QTRY_VERIFY_WITH_TIMEOUT(spyAlias.count() >= 1, 3000);
        QCOMPARE(bridge.activeClientAlias(), QString("Alice"));

        // Now set FPS and verify currentFps changes and FpsApplied event emitted
        QSignalSpy spyFps(&bridge, &ImageServerBridge::currentFpsChanged);
        QSignalSpy spyEvent(&bridge, &ImageServerBridge::eventOccurred);

        bridge.setFps(30);

        QTRY_VERIFY_WITH_TIMEOUT(spyFps.count() >= 1, 3000);
        QCOMPARE(bridge.currentFps(), 30);

        // Ensure FpsApplied event was emitted
        bool foundFpsApplied = false;
        for (const auto &args : spyEvent) {
            imagesocket::EventCode code = args.at(0).value<imagesocket::EventCode>();
            if (code == imagesocket::FpsApplied) { foundFpsApplied = true; break; }
        }
        QVERIFY(foundFpsApplied);

        // Cleanup
        client.close();
        bridge.stop();
    }
};

QTEST_MAIN(TestInfoOverlay)
#include "test_infooverlay.moc"
