#ifndef QMLIMAGEPROVIDER_H
#define QMLIMAGEPROVIDER_H

#include <QQuickImageProvider>

class ImageServerBridge;

class QmlImageProvider : public QQuickImageProvider
{
public:
    explicit QmlImageProvider(ImageServerBridge* bridge);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    ImageServerBridge* m_bridge;
};

#endif // QMLIMAGEPROVIDER_H
