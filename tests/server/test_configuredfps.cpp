#include <QtTest/QtTest>
#include <QSettings>
#include "imageserverbridge.h"

class TestConfiguredFps : public QObject {
    Q_OBJECT

private slots:
    void testPersistenceAcrossInstances() {
        // Ensure we start clean
        QSettings settings("ImageSocket", "Server");
        settings.remove("fps");
        settings.sync();

        {
            ImageServerBridge bridge;
            bridge.setConfiguredFps(30);
            QCOMPARE(bridge.configuredFps(), 30);
        }

        // Construct a new instance and verify it picks up the saved value
        ImageServerBridge bridge2;
        QCOMPARE(bridge2.configuredFps(), 30);

        // cleanup
        settings.remove("fps");
        settings.sync();
    }
};

QTEST_MAIN(TestConfiguredFps)
#include "test_configuredfps.moc"
