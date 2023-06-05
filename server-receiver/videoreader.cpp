#include "videoreader.h"

VideoClass::VideoClass(LiveImageProvider *obj)
{
    capture.open("/home/david/Projetos/digit-reader/input_source.mp4");
    if (!capture.isOpened()) {
        qDebug() << "Failed to open video: /home/david/Projetos/digit-reader/input_source.mp4";
    }

    this->lip = obj;

    QObject::connect(&timer, &QTimer::timeout, this, &VideoClass::readFrame);
}

VideoClass::~VideoClass()
{
    capture.release();
}

void VideoClass::readFrame()
{
    cv::Mat frame;
    capture >> frame;  // Read a frame from the camera

    if (!frame.empty()) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);  // Convert the frame to RGB format

        QImage image(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        this->lip->updateImage(image);  // Update the image in the LiveImageProvider
    }
}

void VideoClass::startVideo()
{
    timer.start(30);
}

void VideoClass::stopVideo()
{
    timer.stop();
}

void VideoClass::resetVideo()
{
    if (!capture.isOpened())
        capture.release();

    capture.open("/home/david/Projetos/digit-reader/input_source.mp4");
    if (!capture.isOpened())
        qDebug() << "Failed to open video: /home/david/Projetos/digit-reader/input_source.mp4";
};
