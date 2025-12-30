#include <QtTest/QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

class TestToast : public QObject {
    Q_OBJECT

private slots:
    void testShowHide() {
        QQmlEngine engine;
        QString path = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/ToastNotification.qml";
        QQmlComponent comp(&engine, QUrl::fromLocalFile(path));
        QVERIFY2(comp.status() == QQmlComponent::Ready, qPrintable(comp.errorString()));
        QObject* obj = comp.create();
        QVERIFY(obj != nullptr);

        // Call show("hi", 100) and expect visible true and opacity 1
        QMetaObject::invokeMethod(obj, "show", Q_ARG(QVariant, QString("hello")), Q_ARG(QVariant, 100));
        QTRY_VERIFY_WITH_TIMEOUT(obj->property("visible").toBool(), 1000);
        // opacity is on the child toastRect; read via child property
        QVariant opacity = obj->property("opacity");
        // if opacity not exposed at root, check child
        if (!opacity.isValid()) {
            QObject* child = obj->findChild<QObject*>("toastRect");
            QVERIFY(child);
            QTRY_VERIFY_WITH_TIMEOUT(child->property("opacity").toDouble() > 0.9, 1000);
        }

        // Hide and ensure it becomes invisible
        QMetaObject::invokeMethod(obj, "hide");
        // Wait for animation to finish and toastRect to be hidden
        QObject* child = obj->findChild<QObject*>("toastRect");
        QVERIFY(child);
        QTRY_VERIFY_WITH_TIMEOUT(!child->property("visible").toBool(), 2000);

        delete obj;
    }
};

QTEST_MAIN(TestToast)
#include "test_toast.moc"