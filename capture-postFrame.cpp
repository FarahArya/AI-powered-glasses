#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <curl/curl.h>
#include <fstream>

void loadEnvFile(const std::string &filepath)
{
    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line))
    {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos)
        {
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

bool sendImageToServer(const std::string &imagePath, const std::string &serverUrl)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *field = curl_mime_addpart(form);
    curl_mime_name(field, "image");
    curl_mime_filedata(field, imagePath.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // timeout after 5 seconds

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    if (!success)
        std::cerr << "Failed to send image: " << curl_easy_strerror(res) << "\n";

    curl_mime_free(form);
    curl_easy_cleanup(curl);
    return success;
}

int main()
{
    std::cout << "Opening camera using GStreamer pipeline...\n";

    std::string pipeline =
        "libcamerasrc ! video/x-raw,width=640,height=480,framerate=2/1,format=RGB ! "
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

    // SERVER IP
    loadEnvFile(".env");

    const char *url = std::getenv("SERVER_URL");
    if (!url)
    {
        std::cerr << "SERVER_URL not set in .env file!\n";
        return -1;
    }
    std::string serverUrl = url;

    cv::Mat frame;
    auto lastCapture = std::chrono::steady_clock::now();

    while (true)
    {
        for (int i = 0; i < 5; ++i)
            cap.grab();  // flush old frames
        cap.read(frame); // get the freshest one

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
            auto now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            filename << "frames/frame_" << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S") << ".jpg";

            cv::imwrite(filename.str(), frame);
            std::cout << "Saved " << filename.str() << "\n";

            if (sendImageToServer(filename.str(), serverUrl))
                std::cout << "Image sent to server successfully.\n";

            lastCapture = now;
            ++frameCount;
        }

        int key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
    }

    cap.release();
    std::cout << "Exiting...\n";
    return 0;
}
