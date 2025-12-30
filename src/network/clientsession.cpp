#include "clientsession.h"
#include <QWebSocket>
#include <QUuid>
#include <QDebug>
#include <QImage>
#include <QBuffer>

#include "control.pb.h"

using imagesocket::control::ControlMessage;

ClientSession::ClientSession(QWebSocket* socket, QObject* parent)
    : QObject(parent), m_socket(socket), m_id(QUuid::createUuid().toString())
{
    if (m_socket) {
        m_socket->setParent(this);
        connect(m_socket, &QWebSocket::binaryMessageReceived, this, &ClientSession::onBinaryMessageReceived);
        connect(m_socket, &QWebSocket::disconnected, this, &ClientSession::onSocketDisconnected);
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
                this, &ClientSession::onSocketError);
    }
}

ClientSession::~ClientSession()
{
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

QString ClientSession::id() const
{
    return m_id;
}

QWebSocket* ClientSession::socket() const
{
    return m_socket;
}

void ClientSession::sendControlMessage(const QByteArray& serialized)
{
    if (!m_socket)
        return;

    QByteArray out;
    out.append(char(0x01)); // control prefix
    out.append(serialized);
    m_socket->sendBinaryMessage(out);
}

void ClientSession::onBinaryMessageReceived(const QByteArray& message)
{    
    if (message.isEmpty()){
        qWarning() << "Received empty message from client" << m_id;
        return;}

    const unsigned char prefix = static_cast<unsigned char>(message.at(0));
    if (prefix == 0x01) {
        // control message
        QByteArray payload = message.mid(1);
        ControlMessage msg;
        if (!msg.ParseFromArray(payload.constData(), payload.size())) {
            qWarning() << "Failed to parse ControlMessage from client" << m_id;
            return;
        }

        qDebug() << "Received ControlMessage from client" << m_id << "type=" << msg.type();
        // emit raw serialized form to avoid moc issues with protobuf type
        emit controlMessageReceived(m_id, payload);
    } else {
        // image or other binary data
        QByteArray payload;
        if (prefix == 0x00)
            payload = message.mid(1); // explicit image prefix
        else
            payload = message; // no prefix, assume whole payload is image

        QImage img;
        if (!img.loadFromData(reinterpret_cast<const uchar*>(payload.constData()), payload.size(), "JPEG")) {
            qWarning() << "Failed to decode image from client" << m_id << "size" << payload.size();
            return;
        }

        emit frameReceived(m_id, img);
    }
}

void ClientSession::onSocketDisconnected()
{
    qInfo() << "ClientSession disconnected" << m_id;
    emit disconnected(m_id);
}

void ClientSession::onSocketError()
{
    if (m_socket) {
        qWarning() << "ClientSession socket error for" << m_id << ":" 
                   << m_socket->errorString()
                   << "error code:" << m_socket->error();
    }
}
