#include <QtTest/QtTest>
#include <QWebSocket>
#include <QEventLoop>
#include <QTimer>
#include "imageserverbridge.h"
#include "control.pb.h"

class TestClientSignals : public QObject {
    Q_OBJECT

private slots:
    void testClientConnectedAndDisconnectedSignals() {
        ImageServerBridge bridge;
        bridge.setPort(0);
        QVERIFY(bridge.start());

        QSignalSpy spyConnected(&bridge, &ImageServerBridge::clientConnectedWithAlias);
        QSignalSpy spyDisconnected(&bridge, &ImageServerBridge::clientDisconnectedWithAlias);

        quint16 port = bridge.serverPort();
        QVERIFY(port > 0);

        QWebSocket client;
        QEventLoop loop;
        connect(&client, &QWebSocket::connected, &loop, &QEventLoop::quit);
        client.open(QUrl(QString("ws://127.0.0.1:%1").arg(port)));
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
        QVERIFY(client.state() == QAbstractSocket::ConnectedState);

        // Send alias
        imagesocket::control::ControlMessage aliasMsg;
        aliasMsg.set_type(imagesocket::control::ALIAS);
        aliasMsg.set_alias("Bob");
        std::string out;
        aliasMsg.SerializeToString(&out);
        QByteArray ba; ba.append(char(0x01)); ba.append(out.data(), (int)out.size());
        client.sendBinaryMessage(ba);

        QTRY_VERIFY_WITH_TIMEOUT(spyConnected.count() >= 1, 3000);
        QList<QVariant> args = spyConnected.takeFirst();
        QString connectedClientId = args.at(0).toString();
        QCOMPARE(connectedClientId, bridge.activeClient());
        QCOMPARE(args.at(1).toString(), QString("Bob"));

        client.close();
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();

        QTRY_VERIFY_WITH_TIMEOUT(spyDisconnected.count() >= 1, 3000);
        QList<QVariant> args2 = spyDisconnected.takeFirst();
        // disconnected signal should contain the same client id that connected earlier
        QCOMPARE(args2.at(0).toString(), connectedClientId);
        QCOMPARE(args2.at(1).toString(), QString("Bob"));

        bridge.stop();
    }
};

QTEST_MAIN(TestClientSignals)
#include "test_client_signals.moc"
