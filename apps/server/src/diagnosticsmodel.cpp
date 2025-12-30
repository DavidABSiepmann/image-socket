#include "diagnosticsmodel.h"
#include <QDebug>

DiagnosticsModel::DiagnosticsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int DiagnosticsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_entries.size());
}

QVariant DiagnosticsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    int row = index.row();
    if (row < 0 || row >= static_cast<int>(m_entries.size())) return QVariant();
    const DiagnosticEntry &e = m_entries[row];

    switch (role) {
    case IdRole: return e.id;
    case CodeRole: return e.code;
    case MessageRole: return e.message;
    case SeverityRole: return e.severity;
    case CountRole: return e.count;
    case FirstTimestampRole: return e.firstTimestamp.toString(Qt::ISODate);
    case LastTimestampRole: return e.lastTimestamp.toString(Qt::ISODate);
    case SourceRole: return e.source;
    case MetadataRole: return QVariant::fromValue(e.metadata);
    case SuppressedRole: return e.suppressed;
    default: return QVariant();
    }
}

QHash<int, QByteArray> DiagnosticsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[CodeRole] = "code";
    roles[MessageRole] = "message";
    roles[SeverityRole] = "severity";
    roles[CountRole] = "count";
    roles[FirstTimestampRole] = "firstTimestamp";
    roles[LastTimestampRole] = "lastTimestamp";
    roles[SourceRole] = "source";
    roles[MetadataRole] = "metadata";
    roles[SuppressedRole] = "suppressed";
    return roles;
}

void DiagnosticsModel::upsertEntry(const DiagnosticEntry &entry)
{
    // Try to find an existing entry with same id
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == entry.id) {
            // update in place and emit dataChanged
            m_entries[i].count = entry.count;
            m_entries[i].lastTimestamp = entry.lastTimestamp;
            m_entries[i].message = entry.message;
            m_entries[i].metadata = entry.metadata;
            m_entries[i].suppressed = entry.suppressed;
            QModelIndex idx = index(static_cast<int>(i));
            emit dataChanged(idx, idx);
            return;
        }
    }

    // append new entry at front (most recent first)
    beginInsertRows(QModelIndex(), 0, 0);
    m_entries.push_front(entry);
    endInsertRows();

    // enforce max entries
    while (static_cast<int>(m_entries.size()) > m_maxEntries) {
        beginRemoveRows(QModelIndex(), static_cast<int>(m_entries.size()) - 1, static_cast<int>(m_entries.size()) - 1);
        m_entries.pop_back();
        endRemoveRows();
    }
}

void DiagnosticsModel::setMaxEntries(int maxEntries)
{
    m_maxEntries = maxEntries;
    // trim if necessary
    while (static_cast<int>(m_entries.size()) > m_maxEntries) {
        beginRemoveRows(QModelIndex(), static_cast<int>(m_entries.size()) - 1, static_cast<int>(m_entries.size()) - 1);
        m_entries.pop_back();
        endRemoveRows();
    }
}

int DiagnosticsModel::maxEntries() const { return m_maxEntries; }

void DiagnosticsModel::clear()
{
    if (m_entries.empty()) return;
    beginResetModel();
    m_entries.clear();
    endResetModel();
}
