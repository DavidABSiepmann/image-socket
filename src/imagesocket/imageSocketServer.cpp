#include "imageSocketServer.h"
#include <QDebug>

ImageSocketServer::ImageSocketServer(QObject* parent)
    : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image)
{
    this->no_image = QImage(":/no_image.jpg");
    this->image = no_image;

    server = new QTcpServer(this);

    // Connect signals and slots
    connect(server, &QTcpServer::newConnection, this, &ImageSocketServer::handleNewConnection);
    connect(server, &QTcpServer::acceptError, this, &ImageSocketServer::handleError);

    server->setMaxPendingConnections(1);
    server->close();
}

ImageSocketServer::~ImageSocketServer()
{
    stopVideo();
}

void ImageSocketServer::startVideo()
{

    if (server->isListening())
    {

        server->resumeAccepting();
        return;
    }

    if(!server->listen(QHostAddress(SOCKET_ADD), SOCKET_PORT))
    {
        emit updateLabel("Error: Unable to start the server");
        return;
    }

    server->resumeAccepting();
    emit updateLabel("Server started");
}

void ImageSocketServer::stopVideo()
{
    waitToClose = true;

    emit updateLabel("Server stopped");
}

void ImageSocketServer::resetVideo()
{
    waitToClose = true;
    startVideo();
}

void ImageSocketServer::closeConnection(QTcpSocket *clientSocket)
{
    if (!clientSocket)
        return;

    qDebug() << "Closing connection";

    clientSocket->disconnectFromHost();
    clientSocket->deleteLater();
    waitToClose = false;
}


void ImageSocketServer::handleNewConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();

    if( !clientSocket )
        return;


    server->pauseAccepting();
    waitToClose = false;

    connect(clientSocket, &QTcpSocket::readyRead, this, &ImageSocketServer::readData);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

    emit updateLabel("Client connected");
}


void ImageSocketServer::readData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (!clientSocket)
         return;

    if(waitToClose)
    {
        closeConnection(clientSocket);
    }

    QByteArray dataRead = clientSocket->readAll();
    bool tryDecode = false;

    //Reset the image data when receive datastart
    if (dataRead.startsWith("datastart")) {
        imageData.clear();
        return;
    }
    if (dataRead.startsWith("dataend")) {
        tryDecode = true;
    }
    else
    {
        imageData.append(dataRead);
    }

    if(!tryDecode) {
        return;
    }

    cv::Mat frame = cv::imdecode(cv::Mat(imageData.size(), 1, CV_8UC1, imageData.data()), cv::IMREAD_COLOR);

    if (!frame.empty())
    {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);  // Convert the frame to RGB format
        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

        this->image = image;
        emit imageChanged();
    }
}

void ImageSocketServer::handleError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    Q_EMIT updateLabel("Socket error occurred: " + server->errorString());
}


QImage ImageSocketServer::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(id);

    QImage result = this->image;

    if(result.isNull()) {
        result = this->no_image;
    }

    if(size) {
        *size = result.size();
    }

    if(requestedSize.width() > 0 && requestedSize.height() > 0) {
        result = result.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }

    return result;
}

/*
ImageSocketClient::ImageSocketClient(QObject* parent)
    : QObject(parent), socket_(nullptr), reconnectTimer_(nullptr)
{
    socket_ = new QTcpSocket(this);
    reconnectTimer_ = new QTimer(this);

    connect(socket_, &QTcpSocket::readyRead, this, &ImageSocketClient::readData);
    connect(socket_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &ImageSocketClient::handleError);
    connect(reconnectTimer_, &QTimer::timeout, this, &ImageSocketClient::reconnect);

    this->lip = obj;
}

ImageSocketClient::~ImageSocketClient()
{
    disconnectFromServer();
}

void ImageSocketClient::connectToServer(const QString& serverAddress, quint16 serverPort)
{
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        Q_EMIT updateLabel("Client is already connected or in the process of connecting.");
        return;
    }

    socket_->connectToHost(serverAddress, serverPort);

    if(!socket_->waitForConnected(3000)) {
        Q_EMIT updateLabel("Error: " + socket_->errorString());
        return;
    }

    if(socket_->state() == QAbstractSocket::ConnectedState) {
        Q_EMIT updateLabel("Connected to server.");
    }
}

void ImageSocketClient::disconnectFromServer()
{
    if (socket_->state() != QAbstractSocket::ConnectedState) {
        Q_EMIT updateLabel("Client is not connected.");
        return;
    }

    socket_->disconnectFromHost();

    if(!socket_->waitForDisconnected(3000)) {
        Q_EMIT updateLabel("Error: " + socket_->errorString());
        return;
    }

    if(socket_->state() == QAbstractSocket::UnconnectedState) {
        Q_EMIT updateLabel("Disconnected from server.");
    }
}

void ImageSocketClient::readData()
{
    QByteArray dataRead = socket_->readAll();
    
    //Reset the image data when receive datastart
    if (dataRead.startsWith("datastart")) {
        imageData.clear();
        return;
    }

    bool tryDecode = false;

    imageData.append(dataRead);
    if (imageData.endsWith("dataend")) {
        tryDecode = true;
    }

    if(!tryDecode) {
        return;
    }

    cv::Mat frame = cv::imdecode(cv::Mat(imageData.size(), 1, CV_8UC1, imageData.data()), cv::IMREAD_COLOR);

    if (!frame.empty())
    {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);  // Convert the frame to RGB format
        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        lip->updateImage(image);
    }
}

void ImageSocketClient::handleError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    Q_EMIT updateLabel("Socket error occurred: " + socket_->errorString());

    socket_->disconnectFromHost();
    reconnectTimer_->start(3000); // Wait 3 seconds before attempting to reconnect
}

void ImageSocketClient::reconnect()
{
    reconnectTimer_->stop();
    connectToServer(SOCKET_ADD, SOCKET_PORT);
}

void ImageSocketClient::startVideo()
{
    reconnectTimer_->stop();
    connectToServer(SOCKET_ADD, SOCKET_PORT);
}
void ImageSocketClient::stopVideo()
{
    disconnectFromServer();
}

void ImageSocketClient::resetVideo()
{
    reconnectTimer_->stop();
    disconnectFromServer();
    connectToServer(SOCKET_ADD, SOCKET_PORT);
}
*/
