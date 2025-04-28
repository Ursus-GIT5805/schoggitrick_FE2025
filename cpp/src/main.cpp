#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/imgproc.hpp>
#include <wiringPi.h>

#include "input.cpp"
#include "drive.cpp"
#include "camera.cpp"

bool button_pressable = true;
bool running = false;

Steer steer;
Camera cam;

// more time would've been much better
void free_run() {
	DirMode dir = Unknown;

	steer.power = 128;
	steer.activate();

	steer.set_angle(0);
	delay(1000);

	// Mat top(CAM_HEIGHT, CAM_WIDTH, CV_8UC1);

    cv::Rect toprect(CAM_WIDTH/4, 0, CAM_WIDTH / 2, CAM_HEIGHT/8);
    cv::Rect leftrect(0, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);
    cv::Rect rightrect(CAM_WIDTH / 4 * 3, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);

	// top(rect).setTo(255);

	running = true;
	while(running) {
		Mat frame = cam.read();

		Mat gray, lab;
		cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
		cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);

		Mat floor = detect_floor(lab);

		{
			Mat stream;
			cv::cvtColor(floor, stream, cv::COLOR_GRAY2RGB);
			cam.stream_frame(stream);
		}

		bool topratio;
		{
			int cnt = cv::countNonZero( floor(toprect) );
			topratio = (double)cnt / (double)(toprect.area());
		}

		bool leftratio;
		{
			int cnt = cv::countNonZero( floor(leftrect) );
			leftratio = (double)cnt / (double)(leftrect.area());
		}

		bool rightratio;
		{
			int cnt = cv::countNonZero( floor(rightrect) );
			rightratio = (double)cnt / (double)(rightrect.area());
		}


		// Ratio of NON_FLOOR / TOTAL_AREA
		double left_inv = 1.0 - leftratio;
		double right_inv = 1.0 - rightratio;

		{
			double mn = std::min(left_inv, right_inv);
			left_inv -= mn;
			right_inv -= mn;
		}

		if(0.4 < left_inv) steer.set_angle(DEF_SPAN);
		if(0.4 < right_inv) steer.set_angle(-DEF_SPAN);

		if(dir == Unknown) {
			dir = detect_dir_mode(lab);

			if(dir == Clockwise) std::cout << "CLOCKWISE!\n";
			if(dir == CounterClockwise) std::cout << "COUNTER CLOCKWISE!\n";
		}

		if(topratio < 0.5) {
			if(dir == CounterClockwise) steer.set_angle(-DEF_SPAN);
			if(dir == Clockwise) steer.set_angle(DEF_SPAN);

		} else {
			steer.set_angle(0);
		}

		delay(2);
	}

}

void obstacle_run() {

}

void handle_button_press() {
	if(!button_pressable) return; // Can't press button right now

	auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::cout << ctime(&timenow) << std::endl;

	return;

	Mode mode = read_mode_switch();

	set_led(false);

	if(mode == FREE_RUN) {
		std::cout << "Start Free Run\n";
		free_run();
	} else {
		std::cout << "Start Obstacle Run\n";
		obstacle_run();
	}

	set_led(true);
}

void handle_mode_switch() {
    using namespace std::chrono_literals;

	if(running) {
		running  = false;
		steer.deactivate();
		return; // ignore switch
	}

	button_pressable = false;

	set_led(false);
	std::this_thread::sleep_for(1s);
	set_led(true);

	button_pressable = true;
}

int main() {
    using namespace std::chrono_literals;

	std::cout << "Setup GPIO pins\n";
	if( wiringPiSetupGpio() != 0 ) {
		std::cerr << "Could not setup GPIO!" << std::endl;
		return 1;
	}
	setupInputPins();
	set_led(false);

	std::cout << "Setup steer\n";
	steer.init();
	steer.deactivate();
	// steer.set_angle(0);

	std::cout << "Setup edge detection\n";
	wiringPiISR(PIN_BUTTON, INT_EDGE_RISING, handle_button_press);
	wiringPiISR(PIN_OP_MODE, INT_EDGE_BOTH, handle_mode_switch);

	set_led(true);

	std::cout << "Start\n";

	// Run indefinitely (or 200 years ig)
	std::this_thread::sleep_for(1752000h);

	std::cout << "Quitting\n";

	return 0;
}

/*
  int main3() {
	if( wiringPiSetupGpio() != 0 ) {
		std::cerr << "Could not setup GPIO!" << std::endl;
		return 1;
	}
	Steer steer;

	int angle = (ANG_LEFT + ANG_RIGHT) / 2;
	const int BOUND = 90;

	steer.set_angle(0);

	initscr();

	while(true) {
		char c = getch();

		if(c == 'w') steer.forward(30);
		else if(c == 's') steer.backward(30);
		else if(c == 'a') {
			angle = std::max(-BOUND, angle - 5);
			steer.set_angle(angle);
		}
		if(c == 'd') {
			angle = std::min(BOUND, angle + 5);
			steer.set_angle(angle);
		}

		if(c == 'q') break;
	}

	endwin();

	return 0;
}*/
