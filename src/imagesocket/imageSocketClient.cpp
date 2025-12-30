#include "imageSocketClient.h"

ImageSocketClient::ImageSocketClient(const std::string &serverAddress, int serverPort)
    : serverAddress_(serverAddress),
      serverPort_(serverPort),
      ioService_(),
      socket_(ioService_),
      receiveThread_(),
      mutex_(),
      cv_(),
      currentImage_(),
      connected_(false)
{
}

ImageSocketClient::~ImageSocketClient()
{
    disconnect();
}

bool ImageSocketClient::connect()
{
    try
    {
        connected_ = false;
        if(receiveThread_.joinable())
            receiveThread_.join();

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(serverAddress_), serverPort_);
        socket_.connect(endpoint);
        connected_ = true;
        receiveThread_ = std::thread(&ImageSocketClient::sendImages, this);
        return true;
    }
    catch (const std::exception &e)
    {
        connected_ = false;
        std::cerr << "Failed to connect to server: " << e.what() << std::endl;
        // Handle connection error
    }
    return false;
}

void ImageSocketClient::disconnect()
{
    if (connected_)
    {
        connected_ = false;
        socket_.close();

        if(receiveThread_.joinable())
            receiveThread_.join();
    }
}

bool ImageSocketClient::setImage(const cv::Mat &image)
{
    if (!connected_)
    {
        return false; // Not connected
    }

    // Lock the mutex and update the current image
    std::unique_lock<std::mutex> lock(mutex_);
    currentImage_ = image.clone();
    cv_.notify_one(); // Notify the sending thread that a new image is available
    return true;
}

void ImageSocketClient::sendImages()
{
    while (connected_)
    {
        // Wait for a new image to be available
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]()
                { return !connected_ || !currentImage_.empty(); });

        // Check if still connected and an image is available
        if (!connected_ || currentImage_.empty())
        {
            continue;
        }

        try
        {
            // Convert the OpenCV image to a buffer
            std::vector<uchar> buffer;
            cv::imencode(".jpg", currentImage_, buffer);

            const size_t chunkSize = 2048; // Set your desired chunk size

            // Send datastart
            boost::asio::write(socket_, boost::asio::buffer("datastart"));

            std::this_thread::sleep_for(std::chrono::microseconds(200));

            for (size_t i = 0; i < buffer.size(); i += chunkSize)
            {
                boost::asio::write(socket_, boost::asio::buffer(&buffer[i], std::min<size_t>(chunkSize, buffer.size() - i)));
            }

            std::this_thread::sleep_for(std::chrono::microseconds(200));

            // Send dataend marker
            boost::asio::write(socket_, boost::asio::buffer("dataend"));
        }
        catch (const std::exception &ex)
        {
            std::string error = ex.what();
            if( error.find("Broken pipe") != std::string::npos)
            {
                std::cout << "Broken pipe" << std::endl;

                connected_ = false;
                socket_.close();
            }

            // Print or log the error message
            std::cerr << "Send error: " << ex.what() << std::endl;
        }

        // Reset the current image
        currentImage_.release();
    }
}
