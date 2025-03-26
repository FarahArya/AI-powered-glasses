#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

int main()
{
    std::cout << "Starting camera...\n";

    // Use 0 for default camera with libcamera backend
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened())
    {
        std::cerr << "Error: Cannot open camera!\n";
        return -1;
    }

    // Configure camera settings
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Additional configuration attempts
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(cv::CAP_PROP_FPS, 30);

    // Give the camera time to warm up
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Camera opened successfully!\n";
    std::cout << "Resolution: "
              << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
              << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << "\n";

    cv::Mat frame;
    int frameCount = 0;
    auto lastCaptureTime = std::chrono::steady_clock::now();

    while (true)
    {
        // Attempt to grab and retrieve frame
        if (!cap.grab())
        {
            std::cerr << "Failed to grab frame\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        if (!cap.retrieve(frame) || frame.empty())
        {
            std::cerr << "Failed to retrieve frame\n";
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
        int key = cv::waitKey(1);
        if (key == 'q' || key == 27) // 'q' or ESC key
        {
            break;
        }
    }

    std::cout << "Exiting...\n";
    cap.release();
    return 0;
}