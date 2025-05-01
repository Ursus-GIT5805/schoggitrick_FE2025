#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <string>
#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "imgproc.cpp"

using cv::Mat;
using namespace std;
namespace fs = std::filesystem;


const char * PICTURES_PATH = "../sample";
constexpr double ang = 180 / 180.0 * M_PI;

int ll = 26;

// const int CAM_WIDTH = 1280;
// const int CAM_HEIGHT = 720;

static void on_trackbar( int res, void* )
{
	ll = res;
}

inline void print_dir(DirMode dir) {
	if(dir == Unknown) cout << "N/A\n";
	if(dir == Clockwise) cout << "Go right! (Clockwise)\n";
	if(dir == CounterClockwise) cout << "Go left! (CounterClockwise)\n";
}

inline void algo(std::string name, Mat frame) {
	cv::namedWindow(name.c_str());
	Mat out = detect_floor2(frame);

	Mat lab;
	cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);

	DirMode dir = detect_dir_mode(lab);
	print_dir(dir);

	cv::imshow(name, out);
	cv::imshow(name + "-orig", frame);
	// cv::imshow(name + "-white", white);
	// cv::imshow(name + "-black", black);

	while(true) {
		char c = cv::waitKey(1);
		if(c == 'q') break;
	}
}

inline int do_camera() {
	cv::VideoCapture cap(2);

	cap.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

	cap.set(cv::CAP_PROP_EXPOSURE, 40);
	cap.set(cv::CAP_PROP_AUTO_EXPOSURE, -1.0);
	cap.set(cv::CAP_PROP_AUTOFOCUS, 1.0);

	if(!cap.isOpened()) return 1;

	// cap.read(frame);

	return 0;
}


int main() {
	for (const auto & entry : fs::directory_iterator(PICTURES_PATH)) {
		Mat frame = cv::imread(entry.path());
		Mat out;
		cv::resize(frame, out, cv::Size(CAM_WIDTH, CAM_HEIGHT));

		// algo(entry.path(), std::move(out));
		algo("a", std::move(out));
	}

	return 0;

	cv::namedWindow("main");
	cv::createTrackbar( "Thresh", "main", &ll, 255, on_trackbar );


	Mat spectrum;
	{
		Mat img(255, 255, CV_8UC3);

		// Iterate through each pixel and set the color
		for (int i = 0; i < 255; ++i) {
			for (int j = 0; j < 255; ++j) {
				// OpenCV uses BGR format by default, so RGB(0, i, j) is written as (j, i, 0)
				img.at<cv::Vec3b>(i, j) = cv::Vec3b(255, j, i);
			}
		}

		cv::cvtColor(img, spectrum, cv::COLOR_Lab2RGB);
	}



	Mat center(CAM_HEIGHT, CAM_WIDTH, CV_8UC1);

	int lw = CAM_WIDTH / 4, lh = CAM_HEIGHT / 4;
    cv::Rect centerRect(lw, lh, CAM_WIDTH/2, CAM_HEIGHT/2);
    center(centerRect).setTo(255);

	Mat kernel = getStructuringElement( cv::MORPH_RECT,
										 cv::Size( 7, 7 ),
										 cv::Point( 3, 3 ) );

    cv::Rect toprect(CAM_WIDTH/4, 0, CAM_WIDTH / 2, CAM_HEIGHT/8);
    cv::Rect leftrect(0, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);
    cv::Rect rightrect(CAM_WIDTH / 4 * 3, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);

	while(true) {
		Mat frame;

		// Mat gray;
		// cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);

		// Mat lab;
		// cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);

		// Mat black;
		// detect_black(gray, black);

		// Mat dilated;
		// cv::dilate(black, dilated, kernel);

		// Mat edges;
		// edge_detection(dilated, edges);

		// Mat cropped;
		// cv::copyTo(edges, cropped, center);

		// Mat cropped1, cropped;
		// // cv::copyTo(edges, cropped1, center);
		// cv::copyTo(edges, cropped, dilated);

		// int cx = CAM_WIDTH / 2;
		// int cy = CAM_HEIGHT / 2;

		// Color col = detect_color_at(lab, cx, cy);

		// if(col == Black) cout << "Black\n";
		// if(col == White) cout << "White\n";
		// if(col == Green) cout << "Green\n";
		// if(col == Red) cout << "Red\n";
		// if(col == Orange) cout << "Orange\n";
		// if(col == Blue) cout << "Blue\n";

		// // cv::Vec3b pixel = lab.at<cv::Vec3b>(cy, cx);

		// int a = pixel.val[1];
		// int b = pixel.val[2];

		// Mat new_spectrum;
		// cv::copyTo(spectrum, new_spectrum, Mat());

		// cv::circle(new_spectrum, cv::Point(a, b), 3, cv::Scalar(0,0,0), -1);

		// cv::imshow("black", out);
		// cv::imshow("aa", dilated);
		// cv::imshow("frame", frame);
		// cv::imshow("spectrum", cropped);

	}
}
