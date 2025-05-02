#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using cv::Mat;

// Camera data
const int CAM_WIDTH = 1280;
const int CAM_HEIGHT = 720;
const int CAM_FPS = 30;

// All manually measured/entered
const int low_H = 60, high_H = 255;

const int red0_low = 0;
const int red0_high = 5;

const int red1_low = 168;
const int red1_high = 255;

const int green_low = 50;
const int green_high = 91;

const int BLACK_THRESHOLD = 42;
const int WHITE_THRESHOLD = 138;

// ---

constexpr double camera_min_vertical_view = 44.2559408  / 180 * M_PI;

constexpr double FOV_HORIZONTAL = 53.5 / 180 * M_PI;
constexpr double FOV_VERTICAL = 41.41 / 180 * M_PI;

const double CAM_PHYSICAL_HEIGHT = 117;

// Masks green. SRC should be in HSV
// ultimately unused
inline void detect_green(cv::InputArray src, cv::OutputArray dst) {
	auto lw = cv::Scalar(green_low, low_H, low_H);
	auto up = cv::Scalar(green_high, high_H, high_H);

	cv::inRange(src, lw, up, dst);
}

// Masks red. SRC should be in HSV
// EXPERIMENTAL, NEEDS MORE testing
// ultimately unused
inline void detect_red(cv::InputArray src, cv::OutputArray dst) {
	auto lw0 = cv::Scalar(red0_low, low_H, low_H);
	auto up0 = cv::Scalar(red0_high, high_H, high_H);

	auto up1 = cv::Scalar(red1_low, low_H, low_H);
	auto lw1 = cv::Scalar(red1_high, high_H, high_H);

	Mat mask0, mask1;
	cv::inRange(src, lw0, up0, mask0);
	cv::inRange(src, lw1, up1, mask1);

	cv::bitwise_or(mask0, mask1, dst);
}

// unused
inline std::pair<double, double> dist_from_camera(double pixel_x, double pixel_y){
    std::pair<double, double> location;

	double cam_w = (double)CAM_WIDTH;
	double cam_h = (double)CAM_HEIGHT;

    //std::cout<<fov_vert<<" ";
    double alpha = camera_min_vertical_view + FOV_VERTICAL/2 + atan(tan(FOV_VERTICAL/2) * (pixel_y - cam_h/2)/(cam_h/2));
    double beta = atan(tan(FOV_HORIZONTAL/2) * (pixel_x - cam_w/2)/(cam_w/2));
    //std::cout<<alpha<<" "<<beta<<" ";

	location.first = tan(beta) * CAM_PHYSICAL_HEIGHT / cos(alpha);
    location.second = tan(alpha) * CAM_PHYSICAL_HEIGHT;
    return location;
}

// Creates a mask where all blackish pixels are on
// src must be gray to work correctly
// unused
inline void detect_black(cv::InputArray src, cv::OutputArray dst) {
	// auto lw = cv::Scalar(0, 32, 32);
	// auto up = cv::Scalar(40, 232, 232);
	// cv::inRange(src, lw, up, dst);
	cv::threshold(src, dst, BLACK_THRESHOLD, 255, cv::THRESH_BINARY_INV);
}

// Creates a mask where all white-ish pixels are on
// src must be gray to work correctly
// unused
inline void detect_white(cv::InputArray src, cv::OutputArray dst) {
	cv::threshold(src, dst, WHITE_THRESHOLD, 255, cv::THRESH_BINARY);


	Mat kernel_small = getStructuringElement( cv::MORPH_RECT,
										 cv::Size( 3, 3 ),
										 cv::Point( 1, 1 ) );

	Mat kernel_big = getStructuringElement( cv::MORPH_RECT,
										 cv::Size( 7, 7 ),
										 cv::Point( 3, 3 ) );

	Mat tmp;
	cv::erode(dst, tmp, kernel_small);
	cv::dilate(tmp, dst, kernel_big);
}

// unused
inline std::pair<double, double> computeRegressionLine(const std::vector<std::pair<double,double>>& points) {
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    int n = points.size();

    for (const auto& point : points) {
        sumX += point.first;
        sumY += point.second;
        sumXY += point.first * point.second;
        sumX2 += point.first * point.first;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    double intercept = (sumY - slope * sumX) / n;

    return {slope, intercept};
}

// Returns how much we have to turn in radians
// barries should contain all the barrier points near the point of interest
// (-100 to 100 pixel in each direction too many might add wrong data points and take a while)
double turn_amount(bool turn_clockwise, std::vector<cv::Point> &barries){
	std::cout << "Num pts: " << barries.size() << "\n";

	std::vector< std::pair<double, double> > v(barries.size());
	for(int32_t i = 0 ; i < barries.size() ; ++i) {
		v[i] = dist_from_camera(barries[i].x, barries[i].y);
	}

    std::pair<double, double> regression_line = computeRegressionLine(v);

	std::cout << "reg: " << regression_line.first << " ";
	double res = atan(regression_line.first);
    if(turn_clockwise){
		if(M_PI/2 - res > -2.5)return -res - M_PI/2;
        return M_PI/2 - res;
    } else {
		if(-M_PI/2 + res < -2.5)return res + M_PI/2;
        return -M_PI/2 + res;
    }
}

// =====

// An enum of colors we have to distinguish
enum Color {
	Black, White,
	Red, Green,
	Blue, Orange,
};
const int NUM_COLORS = 6;

// The Lab centroids used to create borders between colors
const std::pair<int,int> CENTERS[NUM_COLORS] = {
	// Uses the same enumeration as the Color enum
	{10000, 10000}, // black
	{128, 128}, // white
	{60, 180}, // red
	{100, 160}, // green
	{135, 135}, // blue
	{150, 80}, // orange
};


// Returns the color given the pixel
// lab_pixel: A pixel in the Lab color space
inline Color detect_color(cv::Vec3b lab_pixel) {
	int l = lab_pixel.val[0];
	int a = lab_pixel.val[1];
	int b = lab_pixel.val[2];

	// check for extreme luminousity
	if(l < 34) return Black;

	// Find the nearest centroids
	int mx = 0, dmax = INT32_MAX;
	for(int i = 0 ; i < NUM_COLORS ; ++i) {
		int x = CENTERS[i].first;
		int y = CENTERS[i].second;
		int dx = x-a;
		int dy = y-b;

		int dist = dx*dx + dy*dy;

		if(dist < dmax) {
			mx = i;
			dmax = dist;
		}
	}

	return (Color)mx;
}

// Detect the areas that are considered the floor (blue, orange, white)
// lab: frame in Lab-colorspace
// gray: frame in gray-colorspace
inline Mat detect_floor(cv::Mat &lab, cv::Mat &gray) {
	cv::Size sz = lab.size();
	Mat dst = Mat(sz.height, sz.width, CV_8UC1);

	auto op = [&](cv::Vec3b &pixel, const int * position) -> void {
		uint8_t brightness = gray.at<uint8_t>(position[0], position[1]);

		Color col = detect_color(pixel);

		bool false_white = brightness < BLACK_THRESHOLD && col == White;
		bool is_floor = col == Blue ||
			col == Orange ||
			col == White;

		is_floor = is_floor && !false_white;

		dst.at<uint8_t>(position[0], position[1]) = 255*(int)is_floor;
	};

	lab.forEach<cv::Vec3b>(op);

	Mat kernel = getStructuringElement( cv::MORPH_RECT,
										 cv::Size( 3, 3 ),
										 cv::Point( 1, 1 ) );

	cv::erode(dst, dst, kernel);

	return dst;
}

// src should be in lab
inline Color detect_floor_line(cv::Mat &src) {
	const int COL_THRESH = 20;
	int cnts[2] = {0,0};

	cv::Rect lower = cv::Rect(0, CAM_HEIGHT/3, CAM_WIDTH, CAM_HEIGHT/3*2);

	int x = CAM_WIDTH / 2;
	for(int y = 0 ; y < CAM_HEIGHT ; ++y) {
		cv::Vec3b pixel = src.at<cv::Vec3b>(y, x);

		Color col = detect_color(pixel);

		if(col == Orange) {
			cnts[0]++;
			if(COL_THRESH <= cnts[0]) return Orange;
		}
		if(col == Blue) {
			cnts[1]++;
			if(COL_THRESH <= cnts[1]) return Blue;
		}
	}

	return White;
}
