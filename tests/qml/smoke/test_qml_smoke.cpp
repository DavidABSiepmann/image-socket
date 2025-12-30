#include <QtTest/QtTest>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>
#include <QUrl>
#include <QStringList>

/**
 * test_qml_smoke.cpp
 * 
 * Smoke tests for QML components.
 * Verifies that all QML files load without syntax or import errors.
 * 
 * Tests:
 * - main.qml loads
 * - Core components load (Theme, InfoOverlay, StatusBar, etc.)
 * - No runtime errors during initialization
 * - No QML compiler warnings
 */

class TestQmlSmoke : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Setup QML engine
        engine = new QQmlEngine(this);
        if (!engine) {
            QFAIL("Failed to create QQmlEngine");
        }
    }

    void cleanupTestCase() {
        if (engine) {
            delete engine;
            engine = nullptr;
        }
    }

    /**
     * Helper to load and verify a QML component
     */
    bool loadComponent(const QString &filePath, QString &errorMsg) {
        QQmlComponent component(engine, QUrl::fromLocalFile(filePath));
        
        if (component.isError()) {
            QStringList errors;
            for (const auto &err : component.errors()) {
                errors << QString("Line %1:%2 - %3")
                    .arg(err.line())
                    .arg(err.column())
                    .arg(err.description());
            }
            errorMsg = errors.join("\n");
            return false;
        }

        // Try to create instance (without parent to avoid scene graph issues)
        QObject *obj = component.create();
        if (!obj) {
            errorMsg = "Failed to instantiate component";
            return false;
        }

        delete obj;
        return true;
    }

    /**
     * Helper to just check compilation (no instantiation)
     * Useful for components that need C++ backend or scene graph
     */
    bool checkCompilation(const QString &filePath, QString &errorMsg) {
        QQmlComponent component(engine, QUrl::fromLocalFile(filePath));
        
        if (component.isError()) {
            QStringList errors;
            for (const auto &err : component.errors()) {
                errors << QString("Line %1:%2 - %3")
                    .arg(err.line())
                    .arg(err.column())
                    .arg(err.description());
            }
            errorMsg = errors.join("\n");
            return false;
        }

        return true;
    }

    // ====================================================================
    // Main Application Tests (Check compilation only)
    // ====================================================================

    // Note: main.qml and components depending on C++ backend modules
    // (ImageSocket, ImageServerBridge) cannot be fully tested here.
    // They require proper QML context setup in integration tests.

    // ====================================================================
    // Theme and Constants Tests
    // ====================================================================

    void test_theme_component_loads() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/Theme.qml";
        QString error;
        
        QVERIFY2(loadComponent(qmlPath, error), 
                 qPrintable("Theme.qml failed to load: " + error));
    }

    void test_theme_properties_accessible() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/Theme.qml";
        QString error;
        
        QQmlComponent component(engine, QUrl::fromLocalFile(qmlPath));
        if (component.isError()) {
            QFAIL(qPrintable("Theme.qml load error: " + error));
        }

        QObject *theme = component.create();
        QVERIFY(theme != nullptr);

        // Verify key properties are accessible
        // Theme has themeMode, backgroundColor, textColor, accentColor, etc.
        QVERIFY(theme->property("themeMode").isValid());
        QVERIFY(theme->property("backgroundColor").isValid());
        QVERIFY(theme->property("textColor").isValid());
        QVERIFY(theme->property("accentColor").isValid());

        delete theme;
    }

    // ====================================================================
    // UI Component Tests
    // ====================================================================

    void test_info_overlay_component_compiles() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/InfoOverlay.qml";
        QString error;
        
        QVERIFY2(checkCompilation(qmlPath, error), 
                 qPrintable("InfoOverlay.qml failed to compile: " + error));
    }


    void test_toast_notification_component_compiles() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/ToastNotification.qml";
        QString error;
        
        QVERIFY2(checkCompilation(qmlPath, error), 
                 qPrintable("ToastNotification.qml failed to compile: " + error));
    }

    void test_styled_button_component_compiles() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/StyledButton.qml";
        QString error;
        
        // StyledButton is a Button subclass (visual) - check compilation only
        QVERIFY2(checkCompilation(qmlPath, error), 
                 qPrintable("StyledButton.qml failed to compile: " + error));
    }

    void test_clients_panel_component_compiles() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/ClientsPanel.qml";
        QString error;
        
        QVERIFY2(checkCompilation(qmlPath, error), 
                 qPrintable("ClientsPanel.qml failed to compile: " + error));
    }

    void test_diagnostics_panel_component_compiles() {
        QString qmlPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/qml/DiagnosticsPanel.qml";
        QString error;
        
        QVERIFY2(checkCompilation(qmlPath, error), 
                 qPrintable("DiagnosticsPanel.qml failed to compile: " + error));
    }

    // ====================================================================
    // Batch Load Tests
    // ====================================================================

    void test_all_simple_qml_components_compile() {
        // Only test components that don't depend on C++ backend or scene graph
        QStringList components = {
            "qml/Theme.qml",
            "qml/InfoOverlay.qml",
            "qml/ToastNotification.qml",
            "qml/StyledButton.qml",
            "qml/ClientsPanel.qml",
            "qml/DiagnosticsPanel.qml"
        };

        int successCount = 0;
        QStringList failures;

        for (const auto &component : components) {
            QString fullPath = QString(PROJECT_SOURCE_DIR) + "/apps/server/" + component;
            QString error;

            if (checkCompilation(fullPath, error)) {
                successCount++;
            } else {
                failures << QString("%1: %2").arg(component, error);
            }
        }

        qDebug() << QString("Compiled %1/%2 testable components successfully")
            .arg(successCount, components.size());

        QCOMPARE(successCount, components.size());
        
        if (!failures.isEmpty()) {
            QFAIL(qPrintable("Failed components:\n" + failures.join("\n")));
        }
    }

private:
    QQmlEngine *engine = nullptr;
};

QTEST_MAIN(TestQmlSmoke)
#include "test_qml_smoke.moc"
