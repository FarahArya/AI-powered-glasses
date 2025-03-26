#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

int main()
{
    std::cout << "Starting camera...\n";

    // Explicitly use gstreamer backend for Raspberry Pi camera
    std::string camera_pipeline =
        "libcamerasrc ! video/x-raw,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! appsink";

    cv::VideoCapture cap(camera_pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened())
    {
        std::cerr << "Error: Cannot open camera!\n";
        std::cerr << "Ensure libcamera and gstreamer are properly installed.\n";
        return -1;
    }

    std::cout << "Camera opened successfully!\n";

    // Give the camera time to warm up
    std::this_thread::sleep_for(std::chrono::seconds(2));

    cv::Mat frame;
    int frameCount = 0;
    auto lastCaptureTime = std::chrono::steady_clock::now();

    while (true)
    {
        // Capture frame
        cap >> frame;

        if (frame.empty())
        {
            std::cerr << "Warning: Captured empty frame!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Check if 2 seconds have passed since last capture
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastCaptureTime);

        if (duration.count() >= 2)
        {
            // Generate filename with zero-padded frame number
            std::ostringstream filename;
            filename << "frame_" << std::setw(4) << std::setfill('0') << frameCount << ".jpg";

            // Save the frame
            cv::imwrite(filename.str(), frame);

            std::cout << "Saved " << filename.str() << "\n";

            // Update last capture time
            lastCaptureTime = now;
            frameCount++;
        }

        // Exit condition
        int key = cv::waitKey(100);
        if (key == 'q' || key == 27) // 'q' or ESC key
        {
            break;
        }
    }

    std::cout << "Exiting...\n";
    cap.release();
    return 0;
}