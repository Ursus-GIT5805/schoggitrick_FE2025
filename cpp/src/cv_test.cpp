#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "imgproc.cpp"

using cv::Mat;

const int BLACK_THRESHOLD = 26;
const int WHITE_THRESHOLD = 106;

constexpr double ang = 180 / 180.0 * M_PI;

int ll = 26;

// src should be a black and white image
void detect_black(cv::InputArray src, cv::OutputArray dst) {
	cv::threshold(src, dst, BLACK_THRESHOLD, 255, cv::THRESH_BINARY_INV);
}

void edge_detection(cv::InputArray src, cv::OutputArray dst) {
	Mat blur;
	cv::blur( src, blur, cv::Size(3,3) );
	cv::Canny(blur, dst, ll, 255, 3);
}

static void on_trackbar( int res, void* )
{
	ll = res;
}

int main() {
	cv::VideoCapture cap(2);
	cap.set(cv::CAP_PROP_EXPOSURE, 40);
	cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0);
	cap.set(cv::CAP_PROP_AUTOFOCUS, 0);

	if(!cap.isOpened()) return 1;

	cv::namedWindow("main");
	cv::createTrackbar( "Thresh", "main", &ll, 255, on_trackbar );

	while(true) {
		Mat frame;
		cap.read(frame);

		Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		Mat white;
		edge_detection(gray, white);
		// cv::threshold(gray, white, ll, 255, cv::THRESH_BINARY);

		cv::imshow("frame", frame);
		cv::imshow("main", white);

		char c = cv::waitKey(1);
		if(c == 'q') break;
	}
}
