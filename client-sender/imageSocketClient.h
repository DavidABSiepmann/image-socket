#ifndef IMAGE_SOCKET_CLIENT_H
#define IMAGE_SOCKET_CLIENT_H

#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 5000

class ImageSocketClient
{
public:
    ImageSocketClient(const std::string &serverAddress, int serverPort);
    ~ImageSocketClient();

    bool connect();
    void disconnect();
    bool setImage(const cv::Mat &image);

private:
    void sendImages();

    std::string serverAddress_;
    int serverPort_;
    boost::asio::io_service ioService_;
    boost::asio::ip::tcp::socket socket_;
    std::thread receiveThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    cv::Mat currentImage_;
    bool connected_;
};

#endif
