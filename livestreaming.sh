#!/bin/bash
# start_streamer: Launch camera MJPEG stream and expose via HTTP and PageKite

# 1. Set up the MJPEG video stream from the Raspberry Pi camera
STREAM_FIFO="/tmp/stream.mjpeg"
echo "Creating FIFO for stream at $STREAM_FIFO..."
if [ -e "$STREAM_FIFO" ]; then
    rm -f "$STREAM_FIFO"  # remove any existing file or pipe
fi
mkfifo "$STREAM_FIFO" || { echo "Failed to create FIFO at $STREAM_FIFO"; exit 1; }
chmod 666 "$STREAM_FIFO"      # make it readable/writeable by all (optional, for permissions)

echo "Starting camera capture (libcamera-vid) at 640x480, 10fps..."
libcamera-vid -t 0 -n --width 640 --height 480 --framerate 10 --codec mjpeg -o "$STREAM_FIFO" &  
CAMERA_PID=$!
sleep 2  # give libcamera-vid a moment to start
if ps -p $CAMERA_PID > /dev/null 2>&1; then
    echo "libcamera-vid is running (PID $CAMERA_PID)."
else
    echo "Error: libcamera-vid failed to start." >&2
    exit 1
fi

# 2. Launch mjpg_streamer to serve the MJPEG stream over HTTP
echo "Starting MJPG-Streamer server on port 8081..."
MJPG_DIR="/home/pi/mjpg-streamer/mjpg-streamer-experimental"   # adjust this path to where mjpg_streamer is built
cd "$MJPG_DIR" || { echo "Failed to find mjpg_streamer directory at $MJPG_DIR"; kill $CAMERA_PID; exit 1; }
# Run mjpg_streamer with input_file (reading the FIFO) and output_http (serving on 8081)
/home/pi/mjpg-streamer/mjpg-streamer-experimental/mjpg_streamer -i "input_file.so -f /tmp -n $(basename $STREAM_FIFO)" -o "output_http.so -p 8081 -w ./www" &  
STREAMER_PID=$!
sleep 2  # allow mjpg_streamer to initialize
if ps -p $STREAMER_PID > /dev/null 2>&1; then
    echo "mjpg_streamer is running (PID $STREAMER_PID) on port 8081."
else
    echo "Error: mjpg_streamer failed to start." >&2
    # Stop the camera process if streamer failed
    kill $CAMERA_PID
    exit 1
fi

# 3. Start PageKite to expose the stream to the internet
PAGEKITE_HOST="glsses.beesscamera.pagekite.me"   # replace with your PageKite domain
echo "Starting PageKite to tunnel $PAGEKITE_HOST to localhost:8081..."
pagekite 8081 "$PAGEKITE_HOST" &  
PAGEKITE_PID=$!
sleep 5  # wait for PageKite to connect
if ps -p $PAGEKITE_PID > /dev/null 2>&1; then
    echo "PageKite is running (PID $PAGEKITE_PID). Stream should be public at http://$PAGEKITE_HOST"
else
    echo "Error: PageKite failed to start." >&2
    # Optional: kill the other processes if PageKite isn't running
    kill $STREAMER_PID $CAMERA_PID
    exit 1
fi

echo "All components started. Streaming live video at http://$PAGEKITE_HOST"
