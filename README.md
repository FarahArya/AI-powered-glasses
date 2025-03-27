# Streaming for Glasses

### Compile "camera-getFrame-libstill" code:

g++ capture_loop.cpp -std=c++17 -o capture_loop

### Compile "capture-saveFrame" code:

g++ capture-saveFrame.cpp -std=c++17 `pkg-config --cflags --libs opencv4` -o capture-saveFrame

---> To Run the Pipeline Headless:
gst-launch-1.0 libcamerasrc ! videoconvert ! fakesink
