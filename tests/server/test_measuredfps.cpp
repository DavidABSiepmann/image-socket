#include <QtTest/QtTest>
#include "imageserverbridge.h"
#include "clientmodel.h"

class TestMeasuredFps : public QObject
{
    Q_OBJECT

private slots:
    void testMeasuredFps()
    {
        ImageServerBridge bridge;
        bridge.setPort(0);
        QVERIFY(bridge.start());

        QObject* cmObj = bridge.clientModel();
        QVERIFY(cmObj != nullptr);
        ClientModel* cm = qobject_cast<ClientModel*>(cmObj);
        QVERIFY(cm != nullptr);

        QString clientId = "test-client-1";
        cm->addClient(clientId, QStringLiteral("Connected"));

        // Make client active
        bridge.setActiveClient(clientId);

        // Simulate frames for ~1s at ~30 FPS
        for (int i = 0; i < 31; ++i) {
            bridge.recordFrameReceived(clientId);
            QTest::qSleep(33); // ~30fps (31 * 33ms ~ 1023ms)
        }

        // Give a short time for accumulation window to close and measurement to update
        QTest::qSleep(200);

        // Trigger one more record to force measurement calculation if it was pending
        bridge.recordFrameReceived(clientId);

        int measured = bridge.activeClientMeasuredFps();
        qInfo() << "Measured FPS after simulated frames:" << measured;
        QVERIFY(measured >= 20 && measured <= 35); // expected around 30fps

        bridge.stop();
    }
};

QTEST_MAIN(TestMeasuredFps)
#include "test_measuredfps.moc"
