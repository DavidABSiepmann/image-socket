#include "qmlimageprovider.h"
#include "imageserverbridge.h"
#include <QImage>

QmlImageProvider::QmlImageProvider(ImageServerBridge* bridge)
    : QQuickImageProvider(QQuickImageProvider::Image), m_bridge(bridge)
{
}

QImage QmlImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);
    Q_UNUSED(requestedSize);

    if (!m_bridge)
        return QImage();

    QImage frame = m_bridge->lastFrame();
    if (size && !frame.isNull()) {
        *size = frame.size();
    }
    return frame;
}
