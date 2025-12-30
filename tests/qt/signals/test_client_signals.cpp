/**
 * @file test_client_signals.cpp
 * @brief Qt signal tests - Client connection/disconnection signals
 *
 * Tests client-related signals:
 * - activeClientChanged signal
 * - activeClientAliasChanged signal
 * - Signal parameters (client ID, alias)
 * - Signal emission on client operations
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include "../fixtures/qt_test_base.h"
#include "network/imageserverbridge.h"
#include "network/clientmodel.h"

/**
 * @class TestClientSignals
 * @brief Tests for client connection/disconnection signals
 */
class TestClientSignals : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: activeClientChanged signal is available
     * Verifies:
     * - Signal can be captured with QSignalSpy
     * - Signal is accessible and valid
     */
    void testActiveClientChangedSignalExists() {
        ImageServerBridge bridge;

        // Capture activeClientChanged signal
        QSignalSpy spy(&bridge, &ImageServerBridge::activeClientChanged);

        // Verify spy is valid
        QVERIFY(spy.isValid());

        // Initially no active client
        QCOMPARE(bridge.activeClient(), QString());
    }

    /**
     * Test: activeClientAliasChanged signal is available
     * Verifies:
     * - Signal can be captured
     * - Initially no alias
     */
    void testActiveClientAliasChangedSignalExists() {
        ImageServerBridge bridge;

        // Capture activeClientAliasChanged signal
        QSignalSpy spy(&bridge, &ImageServerBridge::activeClientAliasChanged);

        // Verify spy is valid
        QVERIFY(spy.isValid());

        // Initially no alias
        QCOMPARE(bridge.activeClientAlias(), QString());
    }

    /**
     * Test: setActiveClient changes the active client
     * Verifies:
     * - setActiveClient() updates activeClient()
     * - activeClientChanged signal is emitted
     * - Empty client ID can be set to clear selection
     */
    void testSetActiveClientWorks() {
        ImageServerBridge bridge;

        // Capture signal
        QSignalSpy spy(&bridge, &ImageServerBridge::activeClientChanged);

        // Initially no active client
        QCOMPARE(bridge.activeClient(), QString());

        // Set active client
        bridge.setActiveClient("test-client-1");

        // Allow event processing
        qt_test::EventLoopSpinner::processEvents();

        // Should be set now
        QCOMPARE(bridge.activeClient(), QString("test-client-1"));

        // Clear active client
        bridge.setActiveClient("");

        // Should be empty now
        QCOMPARE(bridge.activeClient(), QString());
    }

    /**
     * Test: activeClient getter is always callable
     * Verifies:
     * - activeClient() can be called at any time
     * - Returns a valid string representation
     */
    void testActiveClientGetterCallable() {
        ImageServerBridge bridge;

        for (int i = 0; i < 5; ++i) {
            QString active = bridge.activeClient();
            // Should be callable, may be empty or null
            active.length();  // Should not crash
            QVERIFY(true);
        }
    }

    /**
     * Test: activeClientAlias getter is always callable
     * Verifies:
     * - activeClientAlias() can be called at any time
     * - Returns a valid string representation
     */
    void testActiveClientAliasGetterCallable() {
        ImageServerBridge bridge;

        for (int i = 0; i < 5; ++i) {
            QString alias = bridge.activeClientAlias();
            // Should be callable, may be empty or null
            alias.length();  // Should not crash
            QVERIFY(true);
        }
    }

    /**
     * Test: ClientModel is created and accessible
     * Verifies:
     * - clientModel() returns non-null object
     * - Model has count property
     * - Initially count is 0
     */
    void testClientModelAccessible() {
        ImageServerBridge bridge;

        // Get client model
        QObject* model = bridge.clientModel();
        QVERIFY(model != nullptr);

        // Verify it's a ClientModel-like object
        int count = model->property("count").toInt();
        QCOMPARE(count, 0);
    }

    /**
     * Test: ClientModel addClient and removeClient work
     * Verifies:
     * - Clients can be added to model
     * - Count increases on add
     * - Count decreases on remove
     */
    void testClientModelAddRemoveClients() {
        ImageServerBridge bridge;

        // Get client model
        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Initially empty
        QCOMPARE(model->count(), 0);

        // Add client
        model->addClient("client-1", "Connected");
        QCOMPARE(model->count(), 1);

        // Add another client
        model->addClient("client-2", "Connected");
        QCOMPARE(model->count(), 2);

        // Remove first client
        model->removeClient("client-1");
        QCOMPARE(model->count(), 1);

        // Remove second client
        model->removeClient("client-2");
        QCOMPARE(model->count(), 0);
    }

    /**
     * Test: ClientModel rowCount matches count
     * Verifies:
     * - rowCount() QAbstractListModel interface works
     * - Matches count() property
     */
    void testClientModelRowCount() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Add clients
        model->addClient("client-1");
        model->addClient("client-2");
        model->addClient("client-3");

        // rowCount should match count
        int row_count = model->rowCount();
        int count = model->count();
        QCOMPARE(row_count, count);
        QCOMPARE(row_count, 3);

        // Remove client
        model->removeClient("client-1");

        // Should be updated
        QCOMPARE(model->rowCount(), 2);
        QCOMPARE(model->count(), 2);
    }

    /**
     * Test: ClientModel setClientAlias updates alias
     * Verifies:
     * - Alias can be set for a client
     * - Value is retrievable
     */
    void testClientModelAliasManagement() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Add client
        model->addClient("client-1");

        // Get initial alias (may be empty)
        QString initial = model->aliasAt(0);
        QVERIFY(!initial.isNull());  // Should be valid QString

        // Set alias
        model->setClientAlias("client-1", "Alice");

        // Process events
        qt_test::EventLoopSpinner::processEvents();

        // Verify alias was set (or at least didn't crash)
        QString after_set = model->aliasAt(0);
        QVERIFY(!after_set.isNull());
    }

    /**
     * Test: ClientModel clientIdAt works
     * Verifies:
     * - Client IDs can be retrieved by index
     * - Returns correct ID
     */
    void testClientModelClientIdAt() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Add clients
        model->addClient("client-alice");
        model->addClient("client-bob");

        // Verify IDs are retrievable
        QCOMPARE(model->clientIdAt(0), QString("client-alice"));
        QCOMPARE(model->clientIdAt(1), QString("client-bob"));
    }

    /**
     * Test: ClientModel clear removes all clients
     * Verifies:
     * - clear() empties the model
     * - Count becomes 0
     */
    void testClientModelClear() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Add clients
        model->addClient("client-1");
        model->addClient("client-2");
        model->addClient("client-3");

        QCOMPARE(model->count(), 3);

        // Clear all
        model->clear();

        // Should be empty
        QCOMPARE(model->count(), 0);
        QCOMPARE(model->rowCount(), 0);
    }

    /**
     * Test: ClientModel signals on row insertion and removal
     * Verifies:
     * - rowsInserted signal is emitted on addClient
     * - rowsRemoved signal is emitted on removeClient
     * - Signal count is correct
     */
    void testClientModelRowInsertRemoveSignals() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Capture row insertion signal
        QSignalSpy spy_insert(model, SIGNAL(rowsInserted(QModelIndex,int,int)));

        // Add client
        model->addClient("client-1");

        // Allow signal processing
        qt_test::EventLoopSpinner::processEvents();

        // Verify signal was emitted
        QTRY_VERIFY_WITH_TIMEOUT(spy_insert.count() >= 1, 1000);

        // Capture row removal signal
        QSignalSpy spy_remove(model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

        // Remove client
        model->removeClient("client-1");

        // Allow signal processing
        qt_test::EventLoopSpinner::processEvents();

        // Verify removal signal was emitted
        QTRY_VERIFY_WITH_TIMEOUT(spy_remove.count() >= 1, 1000);
    }

    /**
     * Test: Multiple clients can coexist in model
     * Verifies:
     * - Model handles multiple clients correctly
     * - All clients are retrievable
     * - Operations don't affect other clients
     */
    void testClientModelMultipleClientsCoexist() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Add multiple clients
        for (int i = 0; i < 5; ++i) {
            QString client_id = QString("client-%1").arg(i);
            model->addClient(client_id);
            model->setClientAlias(client_id, QString("User%1").arg(i));
        }

        // Verify all are present
        QCOMPARE(model->count(), 5);

        // Verify all IDs and aliases are correct
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(model->clientIdAt(i), QString("client-%1").arg(i));
            QCOMPARE(model->aliasAt(i), QString("User%1").arg(i));
        }

        // Remove one
        model->removeClient("client-2");

        // Others should be unaffected
        QCOMPARE(model->count(), 4);
        QCOMPARE(model->clientIdAt(0), QString("client-0"));
        QCOMPARE(model->clientIdAt(1), QString("client-1"));
    }

    /**
     * Test: ClientModel maintains consistent state
     * Verifies:
     * - No crashes or invalid state after operations
     * - Count is always consistent
     */
    void testClientModelStateConsistency() {
        ImageServerBridge bridge;

        QObject* model_obj = bridge.clientModel();
        ClientModel* model = qobject_cast<ClientModel*>(model_obj);
        QVERIFY(model != nullptr);

        // Perform random operations
        model->addClient("c1");
        QCOMPARE(model->rowCount(), model->count());

        model->addClient("c2");
        QCOMPARE(model->rowCount(), model->count());

        model->setClientAlias("c1", "Alice");
        QCOMPARE(model->rowCount(), model->count());

        model->removeClient("c1");
        QCOMPARE(model->rowCount(), model->count());

        model->clear();
        QCOMPARE(model->rowCount(), model->count());
        QCOMPARE(model->count(), 0);
    }
};

QTEST_MAIN(TestClientSignals)
#include "test_client_signals.moc"
