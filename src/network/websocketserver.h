#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QHostAddress>
#include <QImage>
#include <QByteArray>
#include "eventcodes.h"

class QWebSocketServer;
class QWebSocket;

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer() override;

    bool start(quint16 port = 0);
    void stop();
    quint16 port() const;

    bool sendControlToClient(const QString& clientId, const QByteArray& serialized);

signals:
    void clientConnected(const QString& clientId, const QHostAddress& address);
    void clientDisconnected(const QString& clientId);
    void controlMessageReceived(const QString& clientId, const QByteArray& serialized);
    void frameReceived(const QString& clientId, const QImage& image);
    // Emit event code + details (details may include {port, reason})
    void serverError(imagesocket::EventCode code, const QVariantMap &details);

private slots:
    void onNewConnection();
    void onSessionDisconnected(const QString& clientId);

private:
    QWebSocketServer* m_server = nullptr;
    QVector<QPointer<class ClientSession>> m_sessions;
};

#endif // WEBSOCKETSERVER_H
