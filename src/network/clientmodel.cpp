#include "clientmodel.h"
#include <cmath>
#include <QDateTime>

ClientModel::ClientModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ClientModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_clients.size();
}

QVariant ClientModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    const int row = index.row();
    if (row < 0 || row >= m_clients.size())
        return QVariant();
    const ClientEntry& e = m_clients.at(row);
    switch (role) {
    case IdRole: return e.id;
    case StatusRole: return e.status;
    case AliasRole: {
        if (!e.alias.isEmpty())
            return e.alias;
        // Fallback: first 4 characters of the id (strip braces if present)
        QString clean = e.id;
        clean.remove('{'); clean.remove('}');
        QString shortId = clean.left(4);
        return QStringLiteral("client_%1").arg(shortId);
    }
    case ConfiguredFpsRole:
        return e.configuredFps;
    case MeasuredFpsRole:
        return e.measuredFps;
    default: return QVariant();
    }
}

QHash<int, QByteArray> ClientModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "clientId";
    roles[StatusRole] = "status";
    roles[AliasRole] = "alias";
    roles[ConfiguredFpsRole] = "configuredFps";
    roles[MeasuredFpsRole] = "measuredFps";
    return roles;
}

void ClientModel::addClient(const QString& id, const QString& status)
{
    if (indexOfClient(id) != -1)
        return;
    beginInsertRows(QModelIndex(), m_clients.size(), m_clients.size());
    ClientEntry e; e.id = id; e.status = status;
    m_clients.append(e);
    endInsertRows();

    emit countChanged(m_clients.size());
}

void ClientModel::removeClient(const QString& id)
{
    int idx = indexOfClient(id);
    if (idx == -1)
        return;
    beginRemoveRows(QModelIndex(), idx, idx);
    m_clients.removeAt(idx);
    endRemoveRows();
}

void ClientModel::clear()
{
    beginResetModel();
    m_clients.clear();
    endResetModel();
}

int ClientModel::indexOfClient(const QString& id) const
{
    for (int i = 0; i < m_clients.size(); ++i) {
        if (m_clients.at(i).id == id)
            return i;
    }
    return -1;
}

void ClientModel::setClientStatus(const QString& id, const QString& status)
{
    int idx = indexOfClient(id);
    if (idx == -1)
        return;
    if (m_clients[idx].status == status)
        return;
    m_clients[idx].status = status;
    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, { StatusRole });
}

void ClientModel::setClientAlias(const QString& id, const QString& alias)
{
    int idx = indexOfClient(id);
    if (idx == -1)
        return;
    if (m_clients[idx].alias == alias)
        return;
    m_clients[idx].alias = alias;
    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, { AliasRole });
}

void ClientModel::setClientConfiguredFps(const QString& id, int fps)
{
    int idx = indexOfClient(id);
    if (idx == -1) return;
    if (m_clients[idx].configuredFps == fps) return;
    m_clients[idx].configuredFps = fps;
    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, { ConfiguredFpsRole });
}

void ClientModel::setClientMeasuredFps(const QString& id, int fps)
{
    int idx = indexOfClient(id);
    if (idx == -1) return;
    if (m_clients[idx].measuredFps == fps) return;
    m_clients[idx].measuredFps = fps;
    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, { MeasuredFpsRole });
}

void ClientModel::recordFrameReceived(const QString& id, qint64 timestampMs)
{
    int idx = indexOfClient(id);
    if (idx == -1) return;

    ClientEntry &e = m_clients[idx];
    if (timestampMs == 0) {
        // missing timestamp, use current time
        timestampMs = QDateTime::currentMSecsSinceEpoch();
    }

    // Initialize window start if needed
    if (e.windowStartMs == 0) {
        e.windowStartMs = timestampMs;
        e.framesInWindow = 0;
    }

    // Count this frame in current window
    e.framesInWindow++;
    e.lastFrameTsMs = timestampMs;

    qint64 elapsed = timestampMs - e.windowStartMs;
    if (elapsed >= 1000) {
        // compute frames per second over the window
        int measured = int(std::round((double(e.framesInWindow) * 1000.0) / double(elapsed)));
        e.measuredFps = measured;

        // reset window
        e.framesInWindow = 0;
        e.windowStartMs = timestampMs;

        QModelIndex modelIndex = index(idx, 0);
        emit dataChanged(modelIndex, modelIndex, { MeasuredFpsRole });
    }
}

QString ClientModel::clientIdAt(int index) const
{
    if (index < 0 || index >= m_clients.size())
        return QString();
    return m_clients.at(index).id;
} 

QString ClientModel::aliasAt(int index) const
{
    if (index < 0 || index >= m_clients.size())
        return QString();
    const ClientEntry& e = m_clients.at(index);
    if (!e.alias.isEmpty())
        return e.alias;
    QString clean = e.id;
    clean.remove('{'); clean.remove('}');
    return QStringLiteral("client_%1").arg(clean.left(4));
}

int ClientModel::configuredFpsAt(int index) const
{
    if (index < 0 || index >= m_clients.size())
        return 0;
    return m_clients.at(index).configuredFps;
}

int ClientModel::measuredFpsAt(int index) const
{
    if (index < 0 || index >= m_clients.size())
        return 0;
    return m_clients.at(index).measuredFps;
}
