#include "liveimageprovider.h"
#include <QDebug>
#include <opencv2/opencv.hpp>

/**
* @brief Image provider that is used to handle the live image stream in the QML viewer.
 */
LiveImageProvider::LiveImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
    this->no_image = QImage(":/no_image.jpg");
    this->blockSignals(false);

    this->image = no_image;
}


QImage LiveImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)

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

void LiveImageProvider::updateImage(const QImage &image_recv)
{
    if(image_recv.isNull())
        return;

    this->image = image_recv;
    emit imageChanged();
}
