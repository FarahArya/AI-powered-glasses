#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <filesystem>

int main()
{
    std::cout << "Opening camera using GStreamer pipeline...\n";

    // GStreamer pipeline compatible with libcamera
    std::string pipeline =
        "libcamerasrc ! video/x-raw,width=640,height=480,format=RGB ! "
        "videoconvert ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened())
    {
        std::cerr << "Error: Unable to open camera via GStreamer.\n";
        return -1;
    }

    std::cout << "Camera opened successfully.\n";

    std::filesystem::create_directories("frames");

    int frameCount = 0;
    const auto captureInterval = std::chrono::milliseconds(2000);
    cv::Mat frame;

    auto lastCapture = std::chrono::steady_clock::now();

    while (true)
    {
        cap >> frame;

        if (frame.empty())
        {
            std::cerr << "Warning: Empty frame captured.\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        auto now = std::chrono::steady_clock::now();
        if (now - lastCapture >= captureInterval)
        {
            std::ostringstream filename;
            filename << "frames/frame_" << std::setw(4) << std::setfill('0') << frameCount << ".jpg";
            cv::imwrite(filename.str(), frame);
            std::cout << "Saved " << filename.str() << "\n";

            lastCapture = now;
            ++frameCount;
        }

        // Optional: press ESC or 'q' to quit
        int key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
    }

    cap.release();
    std::cout << "Exiting...\n";
    return 0;
}
