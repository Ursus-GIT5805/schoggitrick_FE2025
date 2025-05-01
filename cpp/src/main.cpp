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
// How many frames it should ignore the change of the floor line
const int FLOOR_LINE_IGNORE = 0;

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
	auto free_run_start = system_clock::now();

	Color floor_line = White;
	int floor_line_ignore_cnt = 0;

	DirMode dir = Unknown;

	int cnt_orange = 0; // We'll only count orange, as it's better detectable
	int cnt_blue = 0;

	int stop_step = INT32_MAX;

	{
		Mat frame = cam.read(), lab;
		cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);
		floor_line = detect_floor_line(lab);

		if(floor_line == Orange) {
			cnt_orange++;
			dir = Clockwise;
		}
		if(floor_line == Blue) {
			cnt_blue++;
			dir = CounterClockwise;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_VID_MS));

		if(dir == Clockwise) std::cout << "CLOCKWISE!\n";
		if(dir == CounterClockwise) std::cout << "COUNTER CLOCKWISE!\n";
	}

	steer.reset_steps();
	steer.power = 128;
	steer.activate();
	steer.set_angle(0);

    cv::Rect toprect(CAM_WIDTH/4, 0, CAM_WIDTH/2, CAM_HEIGHT/8);
    cv::Rect leftrect(0, CAM_HEIGHT / 8, CAM_WIDTH / 8, CAM_HEIGHT/3);
    cv::Rect rightrect(CAM_WIDTH / 8 * 7, CAM_HEIGHT / 8, CAM_WIDTH / 8, CAM_HEIGHT / 3);
    cv::Rect lowrect(CAM_WIDTH/4, CAM_HEIGHT/2, CAM_WIDTH/2, CAM_HEIGHT/2);

	while(state == FreeRun) {
		{
			auto timenow =
				std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::cout << "Up:" << ctime(&timenow) << "\n";
			std::cout << "Steps: " << steer.get_steps() << " [" << stop_step << "]\n";
		}

		if(stop_step < steer.get_steps()) {
			state = Waiting;
			break;
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

		double topratio;
		{
			int cnt = cv::countNonZero( floor(toprect) );
			topratio = (double)cnt / (double)(toprect.area());
		}

		double leftratio;
		{
			int cnt = cv::countNonZero( floor(leftrect) );
			leftratio = (double)cnt / (double)(leftrect.area());
		}

		double rightratio;
		{
			int cnt = cv::countNonZero( floor(rightrect) );
			rightratio = (double)cnt / (double)(rightrect.area());
		}

		double lowratio;
		{
			int cnt = cv::countNonZero( floor(lowrect) );
			lowratio = (double)cnt / (double)(lowrect.area());
		}


		// Line counting
		{
			Color col = detect_floor_line(lab);
			int steps = steer.get_steps();

			if(col != floor_line && FLOOR_LINE_IGNORE < ++floor_line_ignore_cnt) {
				if(col == Blue) {
					cnt_blue += 1;
					std::cout << "Floor: Blue (" << cnt_blue << ")\n";
				} else if(col == Orange) {
					cnt_orange += 1;
					std::cout << "Floor: Orange (" << cnt_orange << ")\n";
				}

				floor_line = col;
				floor_line_ignore_cnt = 0;
			}

			if(dir == Unknown) {
				if(col == Orange) {
					dir = Clockwise;
					std::cout << "CLOCKWISE!\n";
				}
				if(col == Blue) {
					dir = CounterClockwise;
					std::cout << "COUNTER CLOCKWISE!\n";
				}
			}

			bool end_soon = false, stop_now = false;

			if(12 == cnt_orange) end_soon = true;
			if(12 < cnt_orange) stop_now = true;

			// if(dir == Clockwise) {
			// } else {
			// 	if(12 == cnt_blue) end_soon = true;
			// 	if(12 < cnt_blue) stop_now = true;
			// }

			if(end_soon) stop_step = std::min(stop_step, steps+3725);

			if(stop_now) {
				state = Waiting;
				break;
			}
		}

		double left_thresh = 0.5;
		double right_thresh = 0.5;

		if(dir == Clockwise) right_thresh = 0.3;
		if(dir == CounterClockwise) left_thresh = 0.3;


		{
			std::cout << "Low: " << lowratio << "\n";
			std::cout << "Left: " << leftratio << "\n";
			std::cout << "Right: " << rightratio << "\n";
			std::cout << "Top: " << topratio << "\n";

		}

		if(lowratio < 0.2) {
			std::cout << "BOTTOM TOO MUCH!\n";

			steer.deactivate();
			steer.set_angle(0);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			steer.backward(360);
			steer.activate();
		} else if(leftratio < left_thresh) {
			std::cout << "LEFT TOO MUCH!\n";

			int div = 1;
			if(dir == CounterClockwise) div = 2;

			steer.set_angle(DEF_SPAN / div);
		} else if(rightratio < right_thresh) {
			std::cout << "RIGHT TOO MUCH!\n";

			int div = 1;
			if(dir == Clockwise) div = 2;

			steer.set_angle(-DEF_SPAN / div);
		} else if(topratio < 0.4) {
			std::cout << "TOP TOO MUCH!\n";

			if(dir == CounterClockwise) steer.set_angle(-DEF_SPAN);
			if(dir == Clockwise) steer.set_angle(DEF_SPAN);
		} else {
			steer.set_angle(0);
		}

		auto time_end = std::chrono::steady_clock::now();
		int diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
		int time_wait = TIME_VID_MS - diff;

		if(0 < time_wait) std::this_thread::sleep_for(std::chrono::milliseconds(time_wait));
	}

	steer.set_angle(0);
	steer.deactivate();

	auto free_run_end = system_clock::now();


	int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(free_run_end-free_run_start).count();

	std::cout << "Needed: " << elapsed << "\n";
}

void obstacle_run() {
    using namespace std::chrono_literals;

	steer.set_angle(0);
	std::this_thread::sleep_for(500ms);

	while(state == ObstacleRun) {
		steer.activate(); // just drive forwards bc fuck this challenge
	}

	steer.deactivate();
	steer.set_angle(0);
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

	delay(1000);

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
