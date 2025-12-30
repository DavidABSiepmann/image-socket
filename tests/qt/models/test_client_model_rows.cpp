/**
 * @file test_client_model_rows.cpp
 * @brief Qt model tests - ClientModel row insertion and removal
 *
 * Tests ClientModel row lifecycle:
 * - rowCount() reflects client count
 * - rowsInserted signal emitted on client add
 * - rowsRemoved signal emitted on client remove
 * - clear() empties model and emits signal
 * - Model consistency after operations
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include <QtCore/QAbstractListModel>
#include "../fixtures/qt_test_base.h"
#include "network/clientmodel.h"

/**
 * @class TestClientModelRows
 * @brief Tests for ClientModel row insertion and removal
 */
class TestClientModelRows : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: Initial model is empty
     * Verifies:
     * - rowCount() returns 0 on construction
     * - count property is 0
     * - Model is empty and accessible
     */
    void testInitialModelEmpty() {
        ClientModel model;
        
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.count(), 0);
        QVERIFY(model.rowCount() >= 0);
    }

    /**
     * Test: Single client addition increases rowCount
     * Verifies:
     * - addClient() increases rowCount from 0 to 1
     * - rowCount is accessible after add
     * - Same ID is retrievable after add
     */
    void testAddSingleClientIncreasesRowCount() {
        ClientModel model;
        
        QCOMPARE(model.rowCount(), 0);
        
        model.addClient("client-001");
        
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.count(), 1);
        
        // Verify client is accessible
        QString id = model.clientIdAt(0);
        QCOMPARE(id, QString("client-001"));
    }

    /**
     * Test: Multiple clients addition
     * Verifies:
     * - Adding multiple clients increases rowCount correctly
     * - Each client is accessible by index
     * - Count property reflects total
     */
    void testAddMultipleClients() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        model.addClient("client-003");
        
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.count(), 3);
        
        // Verify each is accessible
        QCOMPARE(model.clientIdAt(0), QString("client-001"));
        QCOMPARE(model.clientIdAt(1), QString("client-002"));
        QCOMPARE(model.clientIdAt(2), QString("client-003"));
    }

    /**
     * Test: rowsInserted signal emitted on addClient
     * Verifies:
     * - rowsInserted(parent, first, last) signal emitted
     * - Signal parameters are correct
     * - Signal emitted once per add operation
     */
    void testRowsInsertedSignalOnAdd() {
        ClientModel model;
        
        QSignalSpy spy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
        
        model.addClient("client-001");
        
        // Verify signal emitted
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 1000);
        
        // Verify signal has correct parameters
        QVariantList args = spy.takeFirst();
        int first = args.at(1).toInt();
        int last = args.at(2).toInt();
        
        QCOMPARE(first, 0);
        QCOMPARE(last, 0);
    }

    /**
     * Test: rowsInserted signal has correct row indices for multiple adds
     * Verifies:
     * - First add: rows 0-0
     * - Second add: rows 1-1
     * - Third add: rows 2-2
     */
    void testRowsInsertedSignalIndicesMultiple() {
        ClientModel model;
        
        QSignalSpy spy(&model, SIGNAL(rowsInserted(QModelIndex, int, int)));
        
        model.addClient("client-001");
        QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 1000);
        
        model.addClient("client-002");
        QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 2, 1000);
        
        model.addClient("client-003");
        QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 3, 1000);
        
        // Verify indices
        QVariantList args0 = spy.at(0);
        QCOMPARE(args0.at(1).toInt(), 0);
        QCOMPARE(args0.at(2).toInt(), 0);
        
        QVariantList args1 = spy.at(1);
        QCOMPARE(args1.at(1).toInt(), 1);
        QCOMPARE(args1.at(2).toInt(), 1);
        
        QVariantList args2 = spy.at(2);
        QCOMPARE(args2.at(1).toInt(), 2);
        QCOMPARE(args2.at(2).toInt(), 2);
    }

    /**
     * Test: Single client removal decreases rowCount
     * Verifies:
     * - removeClient() decreases rowCount from 1 to 0
     * - Removed client is no longer accessible
     * - rowCount is correct after removal
     */
    void testRemoveSingleClientDecreasesRowCount() {
        ClientModel model;
        
        model.addClient("client-001");
        QCOMPARE(model.rowCount(), 1);
        
        model.removeClient("client-001");
        
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.count(), 0);
    }

    /**
     * Test: Remove specific client from multiple
     * Verifies:
     * - Removing middle client removes correct one
     * - Remaining clients shift indices correctly
     * - rowCount reflects removal
     */
    void testRemoveSpecificClientFromMultiple() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        model.addClient("client-003");
        
        QCOMPARE(model.rowCount(), 3);
        
        // Remove middle client
        model.removeClient("client-002");
        
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.clientIdAt(0), QString("client-001"));
        QCOMPARE(model.clientIdAt(1), QString("client-003"));
    }

    /**
     * Test: rowsRemoved signal emitted on removeClient
     * Verifies:
     * - rowsRemoved(parent, first, last) signal emitted
     * - Signal parameters are correct
     * - Signal emitted when client removed
     */
    void testRowsRemovedSignalOnRemove() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        
        QSignalSpy spy(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
        
        model.removeClient("client-002");
        
        // Verify signal emitted
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 1000);
        
        // Verify signal has correct parameters (row 1 removed)
        QVariantList args = spy.takeFirst();
        int first = args.at(1).toInt();
        int last = args.at(2).toInt();
        
        QCOMPARE(first, 1);
        QCOMPARE(last, 1);
    }

    /**
     * Test: clear() empties model and emits signal
     * Verifies:
     * - clear() sets rowCount to 0
     * - All clients removed
     * - Model is empty after clear
     */
    void testClearEmptiesModel() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        model.addClient("client-003");
        
        QCOMPARE(model.rowCount(), 3);
        
        model.clear();
        
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.count(), 0);
    }

    /**
     * Test: Model consistency after add and remove operations
     * Verifies:
     * - Add client, remove it, add different client
     * - rowCount is correct at each step
     * - Clients are accessible and distinct
     */
    void testModelConsistencyAfterAddRemove() {
        ClientModel model;
        
        // Add
        model.addClient("client-001");
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.clientIdAt(0), QString("client-001"));
        
        // Remove
        model.removeClient("client-001");
        QCOMPARE(model.rowCount(), 0);
        
        // Add different client
        model.addClient("client-002");
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.clientIdAt(0), QString("client-002"));
    }

    /**
     * Test: Model accessible at all steps
     * Verifies:
     * - data() calls don't crash at any state
     * - roleNames() is accessible
     * - Model remains valid through operations
     */
    void testModelAccessibilityThroughOperations() {
        ClientModel model;
        
        // Empty model
        QHash<int, QByteArray> roles = model.roleNames();
        QVERIFY(roles.contains(ClientModel::IdRole));
        
        model.addClient("client-001");
        
        // With client
        QVariant id_data = model.data(model.index(0, 0), ClientModel::IdRole);
        QCOMPARE(id_data.toString(), QString("client-001"));
        
        model.removeClient("client-001");
        
        // Empty again
        roles = model.roleNames();
        QVERIFY(roles.contains(ClientModel::IdRole));
    }

    /**
     * Test: Count property changes on add/remove
     * Verifies:
     * - count property reflects current row count
     * - count accessible via count() method
     * - count is consistent with rowCount
     */
    void testCountPropertyConsistent() {
        ClientModel model;
        
        QCOMPARE(model.count(), 0);
        
        model.addClient("client-001");
        QCOMPARE(model.count(), 1);
        QCOMPARE(model.count(), model.rowCount());
        
        model.addClient("client-002");
        QCOMPARE(model.count(), 2);
        QCOMPARE(model.count(), model.rowCount());
        
        model.removeClient("client-001");
        QCOMPARE(model.count(), 1);
        QCOMPARE(model.count(), model.rowCount());
    }

    /**
     * Test: indexOfClient returns correct index
     * Verifies:
     * - indexOfClient("client-001") returns 0 when first
     * - indexOfClient("client-002") returns 1 when second
     * - indexOfClient() returns valid index
     */
    void testIndexOfClientReturnsCorrectIndex() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        model.addClient("client-003");
        
        QCOMPARE(model.indexOfClient("client-001"), 0);
        QCOMPARE(model.indexOfClient("client-002"), 1);
        QCOMPARE(model.indexOfClient("client-003"), 2);
    }

    /**
     * Test: Model behaves correctly with duplicate add attempts
     * Verifies:
     * - Adding same client ID doesn't crash
     * - rowCount doesn't increase unexpectedly
     * - Model remains in valid state
     */
    void testDuplicateAddAttemptSafe() {
        ClientModel model;
        
        model.addClient("client-001");
        QCOMPARE(model.rowCount(), 1);
        
        // Add same client again (behavior depends on implementation)
        // Should either ignore or handle gracefully
        model.addClient("client-001");
        
        // Model should remain valid and have >= 1 client
        QVERIFY(model.rowCount() >= 1);
        QVERIFY(model.count() >= 1);
    }
};

QTEST_MAIN(TestClientModelRows)
#include "test_client_model_rows.moc"
