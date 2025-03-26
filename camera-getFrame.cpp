#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

int main()
{
    std::cout << "Starting camera...\n";

    // Use V4L2 backend for Raspberry Pi camera
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    if (!cap.isOpened())
    {
        std::cerr << "Error: Cannot open camera!\n";
        return -1;
    }

    // Configure camera settings
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Set additional properties to improve frame capture
    cap.set(cv::CAP_PROP_BUFFERSIZE, 3);    // Reduce internal buffer to minimize stale frames
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1); // Manual exposure
    cap.set(cv::CAP_PROP_EXPOSURE, 500);    // Adjust exposure time

    // Give the camera time to stabilize
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Verify camera settings
    std::cout << "Resolution set to "
              << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
              << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << "\n";

    cv::Mat frame;
    int frameCount = 0;
    auto lastCaptureTime = std::chrono::steady_clock::now();

    while (true)
    {
        // Clear previous frame
        frame = cv::Mat();

        // Discard any stale frames in the buffer
        for (int i = 0; i < 3; ++i)
        {
            cap.grab();
        }

        // Retrieve a fresh frame
        if (!cap.retrieve(frame) || frame.empty())
        {
            std::cerr << "Warning: Failed to capture frame!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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