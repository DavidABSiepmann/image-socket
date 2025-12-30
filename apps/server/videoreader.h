#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QTimer>
#include <opencv2/opencv.hpp>
#include "liveimageprovider.h"

class VideoClass : public QObject
{
    Q_OBJECT
    cv::VideoCapture capture;
    LiveImageProvider *lip = nullptr;
    QTimer timer;
public:
    VideoClass(LiveImageProvider *obj);
    ~VideoClass();

public slots:
    void readFrame();
    void startVideo();
    void stopVideo();
    void resetVideo();
};


#endif // VIDEOREADER_H
