#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using cv::Mat;
using cv::VideoCapture;

const char* WIN_NAME = "Video";

int main() {
	Mat frame;
	cv::namedWindow(WIN_NAME);
	VideoCapture cap(0);

	if(!cap.isOpened()) {
		std::cerr << "No video stream detected" << std::endl;
		return 1;
	}

	while(true) {
		cap >> frame;
		if (frame.empty()) break;

		// ---

		Mat out;
		cv::cvtColor(frame, out, cv::COLOR_RGBA2GRAY);
		cv::imshow(WIN_NAME, out);

		// ---

		char c = (char)cv::waitKey(1);
		if (c == 'q') break;
	}

	cap.release();
	return 0;
}
