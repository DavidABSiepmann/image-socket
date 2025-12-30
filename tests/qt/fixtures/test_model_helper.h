/**
 * @file test_model_helper.h
 * @brief Utilities for testing QAbstractListModel
 *
 * Provides:
 * - Helpers to verify row insertion/removal
 * - Role value extraction helper
 * - dataChanged signal validation helper
 *
 * Header-only, no assertions, no production logic
 *
 * Usage:
 *   ModelTestHelper helper(model);
 *   helper.verifyRowCount(1);
 *   QVariant alias = helper.roleValue(0, Qt::DisplayRole);
 */

#ifndef TEST_MODEL_HELPER_H
#define TEST_MODEL_HELPER_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QVariant>
#include <QtCore/QModelIndex>
#include <QtCore/QSignalSpy>
#include <QtCore/QList>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class ModelTestHelper
 * @brief Test utilities for QAbstractListModel
 *
 * Provides non-assertion helpers to work with models in tests.
 * Does not contain assertions - test code adds those.
 *
 * Features:
 * - Row count retrieval
 * - Role value extraction
 * - Signal argument inspection
 * - Index validation
 */
class ModelTestHelper {
public:
    /**
     * Create helper for given model
     * @param model Model to test (must not be null)
     */
    explicit ModelTestHelper(QAbstractListModel* model)
        : m_model(model) {
    }

    /**
     * Get current row count
     */
    int rowCount() const {
        if (!m_model) {
            return -1;
        }
        return m_model->rowCount();
    }

    /**
     * Get value for specific role at row
     * @param row Row index
     * @param role Qt role to query
     * @return Variant with role value (or null if invalid)
     */
    QVariant roleValue(int row, int role = Qt::DisplayRole) const {
        if (!m_model || !isValidRow(row)) {
            return QVariant();
        }

        QModelIndex index = m_model->index(row, 0);
        return m_model->data(index, role);
    }

    /**
     * Get multiple role values for a row
     * @param row Row index
     * @param roles List of roles to query
     * @return Map of role -> value (empty if invalid row)
     */
    QMap<int, QVariant> roleValues(int row, const QList<int>& roles) const {
        QMap<int, QVariant> result;

        if (!m_model || !isValidRow(row)) {
            return result;
        }

        for (int role : roles) {
            result[role] = roleValue(row, role);
        }

        return result;
    }

    /**
     * Check if row index is valid
     */
    bool isValidRow(int row) const {
        return m_model && row >= 0 && row < m_model->rowCount();
    }

    /**
     * Get index for row
     * @param row Row index
     * @return QModelIndex (invalid if row out of bounds)
     */
    QModelIndex index(int row) const {
        if (!m_model || !isValidRow(row)) {
            return QModelIndex();
        }
        return m_model->index(row, 0);
    }

    /**
     * Get row count from rowsInserted signal
     * Useful for verifying insertion arguments
     *
     * @param signal_args QSignalSpy arguments from rowsInserted
     * @return Number of rows inserted (parent, first, last)
     */
    static int rowsInsertedCount(const QVariantList& signal_args) {
        // rowsInserted(QModelIndex parent, int first, int last)
        if (signal_args.size() < 3) {
            return 0;
        }

        int first = signal_args.at(1).toInt();
        int last = signal_args.at(2).toInt();

        // Count includes both endpoints
        return (last - first + 1);
    }

    /**
     * Get row count from rowsRemoved signal
     * @param signal_args QSignalSpy arguments from rowsRemoved
     * @return Number of rows removed
     */
    static int rowsRemovedCount(const QVariantList& signal_args) {
        // Same as rowsInsertedCount - structure is identical
        return rowsInsertedCount(signal_args);
    }

    /**
     * Extract first row index from rowsInserted signal
     * @param signal_args QSignalSpy arguments from rowsInserted
     * @return First row index (or -1 if invalid)
     */
    static int getFirstRowFromSignal(const QVariantList& signal_args) {
        if (signal_args.size() < 2) {
            return -1;
        }
        return signal_args.at(1).toInt();
    }

    /**
     * Extract last row index from rowsInserted/rowsRemoved signal
     * @param signal_args QSignalSpy arguments
     * @return Last row index (or -1 if invalid)
     */
    static int getLastRowFromSignal(const QVariantList& signal_args) {
        if (signal_args.size() < 3) {
            return -1;
        }
        return signal_args.at(2).toInt();
    }

    /**
     * Inspect dataChanged signal arguments
     * @param signal_args QSignalSpy arguments from dataChanged
     * @param out_first_row Output: first row in range
     * @param out_last_row Output: last row in range
     * @param out_roles Output: affected roles
     * @return True if valid arguments
     */
    static bool inspectDataChangedSignal(
            const QVariantList& signal_args,
            int& out_first_row,
            int& out_last_row,
            QList<int>& out_roles) {
        // dataChanged(QModelIndex topLeft, QModelIndex bottomRight, QVector<int> roles)
        if (signal_args.size() < 3) {
            return false;
        }

        QModelIndex top_left = signal_args.at(0).value<QModelIndex>();
        QModelIndex bottom_right = signal_args.at(1).value<QModelIndex>();

        if (!top_left.isValid() || !bottom_right.isValid()) {
            return false;
        }

        out_first_row = top_left.row();
        out_last_row = bottom_right.row();

        // Optional: extract roles if provided
        QVariant roles_variant = signal_args.at(2);
        if (roles_variant.canConvert<QList<int>>()) {
            out_roles = roles_variant.value<QList<int>>();
        }

        return true;
    }

    /**
     * Create spy for rowsInserted signal
     * Helper to reduce boilerplate in tests
     */
    QSignalSpy* createRowsInsertedSpy() const {
        if (!m_model) {
            return nullptr;
        }
        return new QSignalSpy(m_model, &QAbstractListModel::rowsInserted);
    }

    /**
     * Create spy for rowsRemoved signal
     */
    QSignalSpy* createRowsRemovedSpy() const {
        if (!m_model) {
            return nullptr;
        }
        return new QSignalSpy(m_model, &QAbstractListModel::rowsRemoved);
    }

    /**
     * Create spy for dataChanged signal
     */
    QSignalSpy* createDataChangedSpy() const {
        if (!m_model) {
            return nullptr;
        }
        return new QSignalSpy(m_model, &QAbstractListModel::dataChanged);
    }

private:
    QAbstractListModel* m_model;
};

}  // namespace qt_test

#endif  // TEST_MODEL_HELPER_H
