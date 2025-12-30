#ifndef WEBSOCKETIMAGECLIENT_H
#define WEBSOCKETIMAGECLIENT_H

#include <QObject>
#include <QString>
#include <functional>

class WebSocketImageClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketImageClient(const QString &host = QStringLiteral("127.0.0.1"), quint16 port = 5000, QObject* parent = nullptr);
    ~WebSocketImageClient() override;

    // Connect synchronously (returns true if connected)
    bool connectToServer();
    void disconnectFromServer();

    // Send a JPEG-encoded frame binary. Returns true on success (queued or sent).
    bool sendFrame(const QByteArray &jpegData);

    // Send a serialized Protobuf control message
    bool sendControlMessage(const QByteArray &serialized);

    // Setters for endpoint
    void setHost(const QString &host) { m_host = host; }
    void setPort(quint16 port) { m_port = port; }

    // Optional callbacks
    void setOnConnected(std::function<void()> cb) { m_onConnected = std::move(cb); }
    void setOnDisconnected(std::function<void()> cb) { m_onDisconnected = std::move(cb); }

    // Get the last configured FPS sent by the server (0 == unset)
    int configuredFps() const;

    // Optional callback when configured FPS changes (may be called from IO thread)
    void setOnFpsChanged(std::function<void(int)> cb) { m_onFpsChanged = std::move(cb); }

    // Alias (optional): used to present a human-friendly name in the server UI
    void setAlias(const QString& alias);
    QString alias() const;

private:
    void doAsyncRead();
    void cleanupConnection();

private:
    QString m_host;
    quint16 m_port;
    QString m_alias;

    // Pimpl to hide Boost.Beast implementation details
    struct Impl;
    Impl* m_impl = nullptr;

    std::function<void()> m_onConnected;
    std::function<void()> m_onDisconnected;
    std::function<void(int)> m_onFpsChanged;
};

#endif // WEBSOCKETIMAGECLIENT_H
