#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <QObject>
#include <QPointer>

class QWebSocket;

namespace imagesocket { namespace control { class ControlMessage; } }

class ClientSession : public QObject
{
    Q_OBJECT
public:
    explicit ClientSession(QWebSocket* socket, QObject* parent = nullptr);
    ~ClientSession() override;

    QString id() const;
    QWebSocket* socket() const;

    // send a serialized protobuf control message (raw bytes) to the peer
    void sendControlMessage(const QByteArray& serialized);

signals:
    void controlMessageReceived(const QString& clientId, const QByteArray& serialized);
    void frameReceived(const QString& clientId, const QImage& image);
    void disconnected(const QString& clientId);

private slots:
    void onBinaryMessageReceived(const QByteArray& message);
    void onSocketDisconnected();
    void onSocketError();

private:
    QPointer<QWebSocket> m_socket;
    QString m_id;
};

#endif // CLIENTSESSION_H
