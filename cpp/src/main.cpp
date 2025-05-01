#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/imgproc.hpp>
#include <wiringPi.h>

#include "input.cpp"
#include "drive.cpp"
#include "camera.cpp"

// Constants

constexpr int TIME_VID_MS = 1000 / CAM_FPS;

// ---

enum State {
	Waiting,
	FreeRun,
	ObstacleRun,
};

State state = Waiting;
bool button_pressable = true;

// ---

Steer steer;
Camera cam;

// ---

// more time would've been much better
void free_run() {
	DirMode dir = Unknown;

	steer.power = 96;
	steer.activate();
	steer.set_angle(0);

    cv::Rect toprect(CAM_WIDTH/4, 0, CAM_WIDTH / 2, CAM_HEIGHT/8);
    cv::Rect leftrect(0, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);
    cv::Rect rightrect(CAM_WIDTH / 4 * 3, 0, CAM_WIDTH / 4, CAM_HEIGHT/2);

	while(state == FreeRun) {
		{
			auto timenow =
				std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::cout << "Up:" << ctime(&timenow) << std::endl;
		}


		auto time_start = std::chrono::steady_clock::now();

		Mat frame = cam.read();

		Mat gray, lab;
		cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
		cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);

		Mat floor = detect_floor2(frame);

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

		if(leftratio < 0.3) {
			std::cout << "LEFT TOO MUCH!\n";
			steer.set_angle(DEF_SPAN);
		}
		if(rightratio < 0.3) {
			std::cout << "RIGHT TOO MUCH!\n";
			steer.set_angle(-DEF_SPAN);
		}

		if(dir == Unknown) {
			dir = detect_dir_mode(lab);

			if(dir == Clockwise) std::cout << "CLOCKWISE!\n";
			if(dir == CounterClockwise) std::cout << "COUNTER CLOCKWISE!\n";
		}

		if(topratio < 0.6) {
			std::cout << "TOP TOO MUCH!\n";

			if(dir == CounterClockwise) steer.set_angle(-DEF_SPAN);
			if(dir == Clockwise) steer.set_angle(DEF_SPAN);
		} else {
			steer.set_angle(0);
		}

		auto time_end = std::chrono::steady_clock::now();
		int diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
		int time_wait = TIME_VID_MS - diff;

		std::this_thread::sleep_for(std::chrono::milliseconds(time_wait));
	}
}

void obstacle_run() {

}

void handle_button_press() {
	if(!button_pressable) return; // Can't press button right now
	if(digitalRead(PIN_BUTTON) == LOW) return;

	Mode mode = read_mode_switch();

	set_led(false);
	if(mode == FREE_RUN) {
		state = FreeRun;
		std::cout << "Start Free Run\n";
	} else {
		state = ObstacleRun;
		std::cout << "Start Obstacle Run\n";
	}
	set_led(true);
}

void handle_mode_switch() {
    using namespace std::chrono_literals;

	if(state != Waiting) {
		state = Waiting;
		steer.deactivate();
		steer.set_angle(0);
		button_pressable = true;
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
	steer.set_angle(0);

	std::cout << "Setup edge detection\n";

	button_callback = handle_button_press;
	op_mode_callback = handle_mode_switch;
	wiringPiISR(PIN_BUTTON, INT_EDGE_BOTH, __button_edge);
	wiringPiISR(PIN_OP_MODE, INT_EDGE_BOTH, __op_pin_edge);

	set_led(true);

	std::cout << "Start\n";

	while(true) {
		if(state == FreeRun) {
			set_led(false);
			button_pressable = false;
			free_run();
			set_led(true);
		} else if(state == ObstacleRun) {
			set_led(false);
			button_pressable = false;
			obstacle_run();
			set_led(true);
		} else {
			button_pressable = true;
		}

		{
			Mat stream = cam.read();
			cam.stream_frame(stream);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_VID_MS));
	}

	std::cout << "Quitting\n";

	return 0;
}
