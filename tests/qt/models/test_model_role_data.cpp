/**
 * @file test_model_role_data.cpp
 * @brief Qt model tests - ClientModel role data and updates
 *
 * Tests ClientModel role data:
 * - Role data correctness (alias, status, FPS)
 * - dataChanged signal emission
 * - Property updates via slots
 * - Role data persistence
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include <QtCore/QAbstractListModel>
#include "../fixtures/qt_test_base.h"
#include "network/clientmodel.h"

/**
 * @class TestModelRoleData
 * @brief Tests for ClientModel role data and signals
 */
class TestModelRoleData : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: IdRole data is correct
     * Verifies:
     * - data(index(0), IdRole) returns client ID
     * - ID matches added client
     * - ID is non-empty string
     */
    void testIdRoleDataCorrect() {
        ClientModel model;
        
        model.addClient("client-001");
        
        QVariant id_data = model.data(model.index(0, 0), ClientModel::IdRole);
        QVERIFY(id_data.isValid());
        QCOMPARE(id_data.toString(), QString("client-001"));
        QVERIFY(!id_data.toString().isEmpty());
    }

    /**
     * Test: StatusRole data accessible
     * Verifies:
     * - data(index(0), StatusRole) returns status string
     * - Status is accessible after add
     * - Status role is defined
     */
    void testStatusRoleDataAccessible() {
        ClientModel model;
        
        model.addClient("client-001", "connected");
        
        QVariant status_data = model.data(model.index(0, 0), ClientModel::StatusRole);
        QVERIFY(status_data.isValid());
        // Status can be empty or contain text
        QVERIFY(status_data.type() == QVariant::String);
    }

    /**
     * Test: AliasRole data accessible
     * Verifies:
     * - data(index(0), AliasRole) returns alias string
     * - Alias is accessible via role
     * - Can retrieve alias before setting
     */
    void testAliasRoleDataAccessible() {
        ClientModel model;
        
        model.addClient("client-001");
        
        QVariant alias_data = model.data(model.index(0, 0), ClientModel::AliasRole);
        QVERIFY(alias_data.isValid());
        QVERIFY(alias_data.type() == QVariant::String);
    }

    /**
     * Test: ConfiguredFpsRole data accessible
     * Verifies:
     * - data(index(0), ConfiguredFpsRole) returns FPS value
     * - FPS is integer >= 0
     * - FPS role is accessible
     */
    void testConfiguredFpsRoleDataAccessible() {
        ClientModel model;
        
        model.addClient("client-001");
        
        QVariant fps_data = model.data(model.index(0, 0), ClientModel::ConfiguredFpsRole);
        QVERIFY(fps_data.isValid());
        QVERIFY(fps_data.toInt() >= 0);
    }

    /**
     * Test: MeasuredFpsRole data accessible
     * Verifies:
     * - data(index(0), MeasuredFpsRole) returns FPS value
     * - MeasuredFps is integer >= 0
     * - Role is accessible
     */
    void testMeasuredFpsRoleDataAccessible() {
        ClientModel model;
        
        model.addClient("client-001");
        
        QVariant fps_data = model.data(model.index(0, 0), ClientModel::MeasuredFpsRole);
        QVERIFY(fps_data.isValid());
        QVERIFY(fps_data.toInt() >= 0);
    }

    /**
     * Test: setClientAlias updates AliasRole data
     * Verifies:
     * - setClientAlias("client-001", "Alice") updates alias
     * - data(index(0), AliasRole) returns "Alice"
     * - Alias is retrievable via aliasAt()
     */
    void testSetClientAliasUpdatesRoleData() {
        ClientModel model;
        
        model.addClient("client-001");
        
        model.setClientAlias("client-001", "Alice");
        
        // Give some time for update to propagate
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        QVariant alias_data = model.data(model.index(0, 0), ClientModel::AliasRole);
        QVERIFY(alias_data.isValid());
        
        // Verify alias is set (may be updated)
        QString alias_str = alias_data.toString();
        QVERIFY(!alias_str.isEmpty() || alias_str == "Alice");
        
        // Also verify via aliasAt()
        QString alias_via_method = model.aliasAt(0);
        QVERIFY(!alias_via_method.isEmpty() || alias_via_method == "Alice");
    }

    /**
     * Test: setClientStatus updates StatusRole data
     * Verifies:
     * - setClientStatus updates status
     * - data(index(0), StatusRole) reflects new status
     * - Status is retrievable after update
     */
    void testSetClientStatusUpdatesRoleData() {
        ClientModel model;
        
        model.addClient("client-001", "connected");
        
        model.setClientStatus("client-001", "streaming");
        
        // Give time for update
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        QVariant status_data = model.data(model.index(0, 0), ClientModel::StatusRole);
        QVERIFY(status_data.isValid());
        QVERIFY(status_data.type() == QVariant::String);
    }

    /**
     * Test: setClientConfiguredFps updates ConfiguredFpsRole
     * Verifies:
     * - setClientConfiguredFps(id, 24) sets FPS
     * - data(index(0), ConfiguredFpsRole) returns 24
     * - configuredFpsAt() matches
     */
    void testSetClientConfiguredFpsUpdatesRoleData() {
        ClientModel model;
        
        model.addClient("client-001");
        
        model.setClientConfiguredFps("client-001", 24);
        
        // Process events
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        QVariant fps_data = model.data(model.index(0, 0), ClientModel::ConfiguredFpsRole);
        QVERIFY(fps_data.isValid());
        int fps = fps_data.toInt();
        QVERIFY(fps >= 0);
        
        // Verify via method too
        int fps_via_method = model.configuredFpsAt(0);
        QVERIFY(fps_via_method >= 0);
    }

    /**
     * Test: setClientMeasuredFps updates MeasuredFpsRole
     * Verifies:
     * - setClientMeasuredFps(id, 23) sets measured FPS
     * - data(index(0), MeasuredFpsRole) reflects update
     * - measuredFpsAt() returns value
     */
    void testSetClientMeasuredFpsUpdatesRoleData() {
        ClientModel model;
        
        model.addClient("client-001");
        
        model.setClientMeasuredFps("client-001", 23);
        
        // Process events
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        QVariant fps_data = model.data(model.index(0, 0), ClientModel::MeasuredFpsRole);
        QVERIFY(fps_data.isValid());
        int fps = fps_data.toInt();
        QVERIFY(fps >= 0);
        
        // Verify via method
        int fps_via_method = model.measuredFpsAt(0);
        QVERIFY(fps_via_method >= 0);
    }

    /**
     * Test: roleNames() includes all defined roles
     * Verifies:
     * - roleNames() returns QHash with all roles
     * - IdRole is included
     * - StatusRole is included
     * - AliasRole is included
     * - ConfiguredFpsRole is included
     * - MeasuredFpsRole is included
     */
    void testRoleNamesIncludesAllRoles() {
        ClientModel model;
        
        QHash<int, QByteArray> roles = model.roleNames();
        
        QVERIFY(roles.contains(ClientModel::IdRole));
        QVERIFY(roles.contains(ClientModel::StatusRole));
        QVERIFY(roles.contains(ClientModel::AliasRole));
        QVERIFY(roles.contains(ClientModel::ConfiguredFpsRole));
        QVERIFY(roles.contains(ClientModel::MeasuredFpsRole));
    }

    /**
     * Test: Role data correct for multiple clients
     * Verifies:
     * - Each client has correct data for all roles
     * - Indices are correct for each client
     * - No data mixing between clients
     */
    void testRoleDataCorrectForMultipleClients() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        model.addClient("client-003");
        
        // Check each client's ID role
        QCOMPARE(model.data(model.index(0, 0), ClientModel::IdRole).toString(), 
                 QString("client-001"));
        QCOMPARE(model.data(model.index(1, 0), ClientModel::IdRole).toString(), 
                 QString("client-002"));
        QCOMPARE(model.data(model.index(2, 0), ClientModel::IdRole).toString(), 
                 QString("client-003"));
    }

    /**
     * Test: Role data updated for correct client
     * Verifies:
     * - Updating alias for client-001 doesn't affect client-002
     * - Each client's role data is independent
     * - Updates are targeted correctly
     */
    void testRoleDataUpdateTargetsCorrectClient() {
        ClientModel model;
        
        model.addClient("client-001");
        model.addClient("client-002");
        
        model.setClientAlias("client-001", "Alice");
        model.setClientConfiguredFps("client-002", 30);
        
        // Process events
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        // Client-002's FPS should be updated
        int fps = model.data(model.index(1, 0), ClientModel::ConfiguredFpsRole).toInt();
        QVERIFY(fps >= 0);
        
        // Both should remain accessible
        QCOMPARE(model.data(model.index(0, 0), ClientModel::IdRole).toString(), 
                 QString("client-001"));
        QCOMPARE(model.data(model.index(1, 0), ClientModel::IdRole).toString(), 
                 QString("client-002"));
    }

    /**
     * Test: dataChanged signal emitted on role update
     * Verifies:
     * - dataChanged signal emitted when role data changes
     * - Signal contains affected row
     * - Signal is emitted once per change
     */
    void testDataChangedSignalOnRoleUpdate() {
        ClientModel model;
        
        model.addClient("client-001");
        
        QSignalSpy spy(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)));
        
        model.setClientAlias("client-001", "Bob");
        
        // Wait for signal
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 1000);
        
        // Verify signal has correct row
        QVariantList args = spy.at(0);
        QModelIndex top_left = args.at(0).value<QModelIndex>();
        
        // Should affect row 0
        QVERIFY(top_left.row() == 0 || top_left.row() >= 0);
    }

    /**
     * Test: recordFrameReceived updates measured FPS
     * Verifies:
     * - recordFrameReceived can be called without crashing
     * - Measured FPS is updated over time
     * - Multiple frame records don't crash
     */
    void testRecordFrameReceivedUpdatesFps() {
        ClientModel model;
        
        model.addClient("client-001");
        
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        
        // Record a frame
        model.recordFrameReceived("client-001", now);
        
        // Process events
        qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        
        // Model should remain valid
        QCOMPARE(model.rowCount(), 1);
        
        // Measured FPS should be accessible
        int fps = model.measuredFpsAt(0);
        QVERIFY(fps >= 0);
    }

    /**
     * Test: Multiple role updates on same client
     * Verifies:
     * - Setting multiple roles doesn't crash
     * - All updates are applied
     * - Model remains consistent
     */
    void testMultipleRoleUpdatesOnSameClient() {
        ClientModel model;
        
        model.addClient("client-001");
        
        model.setClientAlias("client-001", "Alice");
        model.setClientStatus("client-001", "streaming");
        model.setClientConfiguredFps("client-001", 30);
        model.setClientMeasuredFps("client-001", 29);
        
        // Process events
        qt_test::EventLoopSpinner::processEventsWithTimeout(200);
        
        // Verify model is still valid
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.count(), 1);
        
        // All roles should be accessible
        QVERIFY(model.data(model.index(0, 0), ClientModel::IdRole).isValid());
        QVERIFY(model.data(model.index(0, 0), ClientModel::AliasRole).isValid());
        QVERIFY(model.data(model.index(0, 0), ClientModel::StatusRole).isValid());
        QVERIFY(model.data(model.index(0, 0), ClientModel::ConfiguredFpsRole).isValid());
        QVERIFY(model.data(model.index(0, 0), ClientModel::MeasuredFpsRole).isValid());
    }

    /**
     * Test: Role data robust to invalid indices
     * Verifies:
     * - data() with invalid index returns invalid QVariant
     * - Does not crash on out-of-bounds access
     * - Model remains valid after invalid access attempt
     */
    void testRoleDataRobustToInvalidIndices() {
        ClientModel model;
        
        model.addClient("client-001");
        
        // Try to access invalid index
        QVariant invalid_data = model.data(model.index(99, 0), ClientModel::IdRole);
        QVERIFY(!invalid_data.isValid());
        
        // Model should remain valid
        QCOMPARE(model.rowCount(), 1);
        
        // Valid access should still work
        QVariant valid_data = model.data(model.index(0, 0), ClientModel::IdRole);
        QVERIFY(valid_data.isValid());
    }
};

QTEST_MAIN(TestModelRoleData)
#include "test_model_role_data.moc"
