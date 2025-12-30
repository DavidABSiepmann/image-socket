#include <iostream>
#include <thread>
#include <chrono>
#include "network/config.h"
#include "network/websocketimageclient.h"
#include <QByteArray>
#include <opencv2/opencv.hpp>

/// Example client application: Connects to server and streams video frames.
///
/// This CLI application demonstrates:
/// - Connecting to ImageSocketServer.
/// - Reading video frames from a file (OpenCV VideoCapture).
/// - Sending frames to the server with automatic reconnection.
/// - Handling client errors and connection state changes.
///
/// Usage:
///   send_image_client [video_file_path]
///   send_image_client --video /path/to/video.mp4
///   send_image_client --server 192.168.1.100 --port 5000 --video video.mp4
///
/// The application loops 100 times, each iteration:
///   - Connects to the server (with exponential backoff if needed).
///   - Opens the video file.
///   - Streams all frames at ~30 FPS.
///   - Disconnects and repeats.
///
int main(int argc, char** argv)
{
    // Simple CLI: use flags --server <addr> --port <port> --video <path>
    std::string serverAddr = kDefaultServerAddress;
    int serverPort = kDefaultServerPort;
    std::string videoPath;
    std::string alias;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server" && i + 1 < argc) {
            serverAddr = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            serverPort = std::stoi(argv[++i]);
        } else if (arg == "--video" && i + 1 < argc) {
            videoPath = argv[++i];
        } else if (arg == "--alias" && i + 1 < argc) {
            alias = argv[++i];
        } else if (videoPath.empty()) {
            // Backwards-compatible positional first argument treated as video path
            videoPath = arg;
        }
    }

    // Use the new WebSocket-based client
    WebSocketImageClient client(QString::fromStdString(serverAddr), static_cast<quint16>(serverPort));
    if (!alias.empty()) client.setAlias(QString::fromStdString(alias));

    // Main loop: demonstrates the client's connect, stream and disconnect cycle.
    for( int i = 0; i < 100; i++ )
    {
        std::cout << "Trying to connect to server time: " << std::to_string(i) << std::endl;

        // Attempt connection (simple blocking connect)
        if (!client.connectToServer()) {
            std::cout << "Failed to connect. Retrying in 1s..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        std::cout << "Socket client connected." << std::endl;
        // Open the video file using OpenCV.
        // If no path was provided on the command line, use a default (for testing).
        if (videoPath.empty()) {
            videoPath = "/home/david/Projetos/digit-reader/input_source.mp4";
            std::cerr << "Warning: no video path provided. Using default: " << videoPath << std::endl;
            std::cerr << "Use --video /path/to/video.mp4 or pass the path as first positional argument." << std::endl;
        }

        cv::VideoCapture videoCapture(videoPath);
        if (!videoCapture.isOpened()) {
            std::cerr << "Failed to open video file: " << videoPath << std::endl;
            return 1;
        }

        // Stream video frames at approximately 30 FPS.
        // Each frame is encoded to JPEG and sent to the server by the client's send thread.
        cv::Mat frame;
        while (videoCapture.read(frame)) {
            // Encode frame as JPEG
            std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 75};
            std::vector<unsigned char> buf;
            cv::imencode(".jpg", frame, buf, params);
            QByteArray jpegData(reinterpret_cast<const char*>(buf.data()), static_cast<int>(buf.size()));

            if (!client.sendFrame(jpegData)) {
                std::cout << "Failed to send frame, disconnecting." << std::endl;
                break;
            }

            // Read configured FPS from server (if provided) and simulate that rate.
            int fps = client.configuredFps();
            if (fps <= 0) fps = 30; // default fallback
            int delay = std::max(1, 1000 / fps);
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }

        // Gracefully disconnect and repeat.
        client.disconnectFromServer();
        std::cout << "Socket client disconnected." << std::endl;
    }

    std::cout << "Socket client finished." << std::endl;
    client.disconnect();

    return 0;
}