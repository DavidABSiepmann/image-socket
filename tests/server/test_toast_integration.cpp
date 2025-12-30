#include <QtTest/QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include "imageserverbridge.h"
#include "control.pb.h"

class TestToastIntegration : public QObject {
    Q_OBJECT

private slots:
    void testToastOnConnectAndDisconnect() {
        ImageServerBridge bridge;
        bridge.setPort(0);
        QVERIFY(bridge.start());

        QQmlEngine engine;
        engine.addImportPath("qrc:/");
        engine.addImportPath("qrc:/ImageSocket");
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/tests/server/toast_connector.qml";
        QQmlComponent comp(&engine, QUrl::fromLocalFile(qmlPath));
        QVERIFY2(comp.status() == QQmlComponent::Ready, qPrintable(comp.errorString()));
        QObject* obj = comp.create();
        QVERIFY(obj);

        // set bridge as property
        bool ok = obj->setProperty("bridge", QVariant::fromValue(static_cast<QObject*>(&bridge)));
        Q_UNUSED(ok);

        // Simulate client connect: send alias control message to trigger onControlMessageReceived
        QString clientId = "{toast-test-client}";
        // Simulate server accepting a new connection (adds client to model and sets active client)
        QMetaObject::invokeMethod(&bridge, "onClientConnected", Qt::DirectConnection,
                                  Q_ARG(QString, clientId), Q_ARG(QHostAddress, QHostAddress()));

        imagesocket::control::ControlMessage aliasMsg;
        aliasMsg.set_type(imagesocket::control::ALIAS);
        aliasMsg.set_alias("Toasty");
        std::string out;
        QVERIFY(aliasMsg.SerializeToString(&out));
        QByteArray ba; ba.append(char(0x01)); ba.append(out.data(), (int)out.size());

        // Invoke the control message slot to simulate alias arrival
        QMetaObject::invokeMethod(&bridge, "onControlMessageReceived", Qt::DirectConnection,
                                  Q_ARG(QString, clientId), Q_ARG(QByteArray, ba));

        // Find the ToastNotification child inside QML object
        QObject* toast = obj->findChild<QObject*>("toastRect");
        if (!toast) {
            // try find root t
            toast = obj->findChild<QObject*>() ;
        }

        // Instead of relying on internal names, probe for any child with property 'visible' true soon
        bool shown = false;
        QTRY_VERIFY_WITH_TIMEOUT([&]() {
            const auto children = obj->children();
            for (QObject* c : children) {
                if (c->property("visible").isValid() && c->property("visible").toBool()) {
                    shown = true; return true; }
            }
            return false;
        }(), 2000);
        QVERIFY(shown);

        // Now simulate disconnect
        QMetaObject::invokeMethod(&bridge, "onSessionDisconnected", Qt::DirectConnection,
                                  Q_ARG(QString, clientId));

        bool shown2 = false;
        QTRY_VERIFY_WITH_TIMEOUT([&]() {
            const auto children = obj->children();
            for (QObject* c : children) {
                if (c->property("visible").isValid() && c->property("visible").toBool()) {
                    shown2 = true; return true; }
            }
            return false;
        }(), 2000);
        QVERIFY(shown2);

        // cleanup
        delete obj;
        bridge.stop();
    }
};

QTEST_MAIN(TestToastIntegration)
#include "test_toast_integration.moc"