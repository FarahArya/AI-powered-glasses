#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    std::cout << "Starting camera...\n";

    std::cout << "Video Capture\n";
    // cv::VideoCapture cap("libcamerasrc ! videoconvert ! appsink", cv::CAP_GSTREAMER);
    cv::VideoCapture cap(0, cv::CAP_V4L2); // Try V4L2 backend
    std::cout << "cap done successfully\n";

    if (!cap.isOpened())
    {
        std::cerr << "Error: Cannot open camera!\n";
        return -1;
    }

    std::cout << "Camera opened successfully!\n";

    return 0;
}
