#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using cv::Mat;

// All measured
const int low_H = 60, high_H = 255;

const int red0_low = 0;
const int red0_high = 5;

const int red1_low = 168;
const int red1_high = 255;

const int green_low = 50;
const int green_high = 91;

// Masks green. SRC should be in HSV
inline void mask_green(cv::InputArray src, cv::OutputArray dst) {
	auto lw = cv::Scalar(green_low, low_H, low_H);
	auto up = cv::Scalar(green_high, high_H, high_H);

	cv::inRange(src, lw, up, dst);
}

// Masks green. SRC should be in HSV
// EXPERIMENTAL, NEEDS MORE testing
inline void mask_red(cv::InputArray src, cv::OutputArray dst) {
	auto lw0 = cv::Scalar(red0_low, low_H, low_H);
	auto up0 = cv::Scalar(red0_high, high_H, high_H);

	auto up1 = cv::Scalar(red1_low, low_H, low_H);
	auto lw1 = cv::Scalar(red1_high, high_H, high_H);

	Mat mask0, mask1;
	cv::inRange(src, lw0, up0, mask0);
	cv::inRange(src, lw1, up1, mask1);

	cv::bitwise_or(mask0, mask1, dst);
}
