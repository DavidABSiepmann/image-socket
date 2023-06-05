#ifndef IMAGE_SOCKET_CLIENT_H
#define IMAGE_SOCKET_CLIENT_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QQuickImageProvider>
#include <opencv2/opencv.hpp>

#define SOCKET_ADD "127.0.0.1"
#define SOCKET_PORT 5000

class ImageSocketServer : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit ImageSocketServer(QObject* parent = nullptr); //Create a socket server to wait clients
    ~ImageSocketServer();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override; //ImageProvider Qml

signals:
    void connectionLost();
    void updateLabel(QString value);
    void imageChanged();

private slots:
    void readData();
    void handleNewConnection();
    void handleError(QAbstractSocket::SocketError error);
    void closeConnection(QTcpSocket *clientSocket);

public slots:
    void startVideo(); //Start to listen clients
    void stopVideo(); //Stop listen clients
    void resetVideo(); //Close the connection and open again

private:
    QTcpServer* server;
    QTimer* reconnectTimer_;
    QByteArray imageData;
    bool waitToClose = false;

    QImage image;
    QImage no_image;
};

#endif
