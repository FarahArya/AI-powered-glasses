#include <cstdlib>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <filesystem>

int main()
{
    std::cout << "Starting capture loop using libcamera-still...\n";

    int frameCount = 0;
    const int intervalSeconds = 2;

    std::string outputDir = "/home/pi/frames"; // Adjust as needed
    std::filesystem::create_directories(outputDir);

    while (true)
    {
        // Build the filename
        std::ostringstream filename;
        filename << outputDir << "/frame_"
                 << std::setw(4) << std::setfill('0') << frameCount
                 << ".jpg";

        // Construct and execute libcamera-still command
        std::ostringstream command;
        command << "libcamera-still --width 640 --height 480 "
                << "--nopreview -o " << filename.str();

        int result = std::system(command.str().c_str());

        if (result == 0)
        {
            std::cout << "Captured " << filename.str() << "\n";
        }
        else
        {
            std::cerr << "Failed to capture image with libcamera-still\n";
        }

        ++frameCount;

        // Wait for next frame
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }

    return 0;
}
