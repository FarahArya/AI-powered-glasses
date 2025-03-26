#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

// Function to run MJPG Streamer
void runMjpgStreamer() {
    std::cout << "Starting MJPG Streamer..." << std::endl;
    std::system("./mjpg_streamer -i \"./input_file.so -f /tmp -n stream.mjpeg\" -o \"./output_http.so -w ./www -p 8081\"");
}

// Function to run PageKite for tunneling
void runPageKite() {
    std::cout << "Starting PageKite..." << std::endl;
    std::system("pagekite 8081 glsses.beesscamera.pagekite.me");
}

// Function to ensure MJPG Streamer is running before starting PageKite
void startStreamAndTunnel() {
    std::cout << "Running MJPG Streamer and PageKite..." << std::endl;

    // Run MJPG Streamer
    std::thread mjpgThread(runMjpgStreamer);

    // Wait for the streamer to initialize
    std::this_thread::sleep_for(std::chrono::seconds(5)); // Give MJPG Streamer time to start

    // Run PageKite
    std::thread pageKiteThread(runPageKite);

    // Wait for both threads to complete
    mjpgThread.join();
    pageKiteThread.join();

    std::cout << "MJPG Streamer and PageKite are running." << std::endl;
}

int main() {
    startStreamAndTunnel();
    return 0;
}
