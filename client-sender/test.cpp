#include <iostream>
#include <thread>
#include <chrono>
#include "imageSocketClient.h"

int main(){

    ImageSocketClient client("127.0.0.1", 5000);

    for( int i = 0; i < 100; i++ )
    {
        std::cout << "Trying to connect to server time: " << std::to_string(i) << std::endl;
        while(!client.connect())
        {
            std::cout << "Trying to connect..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        std::cout << "Socket client connected." << std::endl;

        // Open the video file using OpenCV
        std::string videoPath = "/home/david/Projetos/digit-reader/input_source.mp4";
        cv::VideoCapture videoCapture(videoPath);
        if (!videoCapture.isOpened()) {
            std::cerr << "Failed to open video file." << std::endl;
            return 1;
        }

        cv::Mat frame;
        while (videoCapture.read(frame)) {

            if( !client.setImage(frame) )
            {
                std::cout << "Failed to send image." << std::endl;
                break;
            }

            int delay = 1000 / 30;
            cv::waitKey(delay);
        }

        client.disconnect();
        std::cout << "Socket client disconnected." << std::endl;
    }

    std::cout << "Socket client finished." << std::endl;
    client.disconnect();

    return 0;
}