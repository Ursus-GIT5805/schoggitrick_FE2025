#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
	cv::VideoCapture cap("udpsrc port=5000 ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,framerate=30/1 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink", cv::CAP_GSTREAMER);

	if (!cap.isOpened()) {
		std::cerr << "VideoCapture not opened" << std::endl;
        exit(-1);
    }

    while (true) {
		cv::Mat frame;
        cap.read(frame);
        imshow("receiver", frame);

		char c = cv::waitKey(1);
        if (c == 'q') break;
    }

    return 0;
}
