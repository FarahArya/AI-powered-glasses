#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080

void handle_client(int client_socket, cv::VideoCapture& cap) {
    const std::string header = 
        "HTTP/1.1 200 OK\r\n"
        "Server: MJPEGStreamer\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";

    send(client_socket, header.c_str(), header.size(), 0);

    cv::Mat frame;
    std::vector<uchar> buffer;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        buffer.clear();
        std::vector<int> param = {cv::IMWRITE_JPEG_QUALITY, 80};
        cv::imencode(".jpg", frame, buffer, param);

        std::string part_header = 
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: " + std::to_string(buffer.size()) + "\r\n\r\n";

        send(client_socket, part_header.c_str(), part_header.size(), 0);
        send(client_socket, buffer.data(), buffer.size(), 0);
        send(client_socket, "\r\n", 2, 0);

        // Small delay
        usleep(50000);  // ~20 FPS
    }

    close(client_socket);
}

int main() {
    cv::VideoCapture cap(0); // 0 = default camera
    if (!cap.isOpened()) {
        std::cerr << "❌ Could not open camera" << std::endl;
        return -1;
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("❌ Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("❌ Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("❌ Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "🚀 MJPEG Stream started on http://localhost:" << PORT << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket >= 0) {
            std::thread(handle_client, new_socket, std::ref(cap)).detach();
        }
    }

    cap.release();
    return 0;
}
