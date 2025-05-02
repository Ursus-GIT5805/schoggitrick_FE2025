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

// Time delay between 2 frames in 1 second (in ms)
constexpr int TIME_VID_MS = 1000 / CAM_FPS;

// How many frames the change of the floor line is ignore
const int FLOOR_LINE_IGNORE = 0;
// Number of steps to ignore the last floorline
const int STEPS_FLOOR_LINE_IGNORE = 360*2;

// Number of final steps to do after having done 3 laps in Free Run
const int FINAL_STEPS = 3900;

enum State {
	Waiting,
	FreeRun,
	ObstacleRun,
};

// Direction used to save the cw, ccw state in Freerun
enum DirMode {
	Unknown,
	Clockwise,
	CounterClockwise,
};

// --- Global Variables ---

State state = Waiting;
bool button_pressable = true;

Steer steer;
Camera cam;

// ---

// === Free Run ===
inline void free_run() {
	auto free_run_start = std::chrono::system_clock::now();

	Color floor_line = White;
	int floor_line_ignore_cnt = 0;

	DirMode dir = Unknown;

	int cnt_orange = 0; // We'll only count orange, as it's better detectable
	int last_orange = -1000000;
	int cnt_blue = 0;

	int stop_step = INT32_MAX; // If the step counter passes this counter, stop immediately

	// Read one frame, see if it can immediately detect a floor line
	{
		Mat frame = cam.read(), lab;
		cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);
		floor_line = detect_floor_line(lab);

		if(floor_line == Orange) {
			cnt_orange++;
			last_orange = 0;
			dir = Clockwise;
		}
		if(floor_line == Blue) {
			cnt_blue++;
			dir = CounterClockwise;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_VID_MS));

#ifdef DEBUG
		if(dir == Clockwise) std::cout << "CLOCKWISE!\n";
		if(dir == CounterClockwise) std::cout << "COUNTER CLOCKWISE!\n";
#endif
	}

	// Setup steer
	steer.reset_steps();
	steer.power = 128;
	steer.activate();
	steer.set_angle(0);

	// Segment the frame in several segments
    cv::Rect toprect(CAM_WIDTH/4, 0, CAM_WIDTH/2, CAM_HEIGHT/8);
    cv::Rect leftrect(0, CAM_HEIGHT / 8, CAM_WIDTH / 8, CAM_HEIGHT/3);
    cv::Rect rightrect(CAM_WIDTH / 8 * 7, CAM_HEIGHT / 8, CAM_WIDTH / 8, CAM_HEIGHT / 3);
    cv::Rect lowrect(CAM_WIDTH/4, CAM_HEIGHT/2, CAM_WIDTH/2, CAM_HEIGHT/2);

	while(state == FreeRun) {
#ifdef DEBUG
		{
			auto timenow =
				std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

			std::cout << "Up:" << ctime(&timenow) << "\n";
			std::cout << "Steps: " << steer.get_steps() << " [" << stop_step << "]\n";
		}
#endif

		// Check if we should stop
		if(stop_step < steer.get_steps()) {
			state = Waiting;
			break;
		}

		auto time_start = std::chrono::steady_clock::now();

		Mat frame = cam.read();
		Mat gray, lab;
		cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
		cv::cvtColor(frame, lab, cv::COLOR_RGB2Lab);

		Mat floor = detect_floor(lab, gray);

#ifdef DEBUG
		{
			Mat stream;
			cv::cvtColor(floor, stream, cv::COLOR_GRAY2RGB);
			cam.stream_frame(stream);
		}
#endif

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


		// Analyze the floor line
		{
			Color col = detect_floor_line(lab);
			int steps = steer.get_steps();

			if(col != floor_line && FLOOR_LINE_IGNORE < ++floor_line_ignore_cnt) {

				int step_diff = steps-last_orange;

				if(col == Blue) {
					cnt_blue += 1;
					// std::cout << "Floor: Blue (" << cnt_blue << ")\n";
				} else if(col == Orange && STEPS_FLOOR_LINE_IGNORE <= step_diff) {
					cnt_orange += 1;
					last_orange = steps;
					std::cout << "Floor: Orange (" << cnt_orange << ")\n";
				}

				floor_line = col;
				floor_line_ignore_cnt = 0;
			}

			if(dir == Unknown) {
				if(col == Orange) {
					dir = Clockwise;
					// std::cout << "CLOCKWISE!\n";
				}
				if(col == Blue) {
					dir = CounterClockwise;
					// std::cout << "COUNTER CLOCKWISE!\n";
				}
			}

			// Slowly start to stop on the 12th occurance
			if(12 == cnt_orange) {
				int add = 0;
				// Drive a bit more on cw, bc it's first orange, then blue
				if(dir == Clockwise) add += 360;

				stop_step = std::min(stop_step, steps + FINAL_STEPS + add);
			}

			// Counted the 13th line, we should stop!
			if(12 < cnt_orange) {
				state = Waiting;
				break;
			}
		}

		double left_thresh = 0.5;
		double right_thresh = 0.5;

		// If we turn clockwise, it should act less if it wants to turn in the other direction
		if(dir == Clockwise) right_thresh = 0.3;
		if(dir == CounterClockwise) left_thresh = 0.3;

#ifdef DEBUG
		{
			std::cout << "Low: " << lowratio << "\n";
			std::cout << "Left: " << leftratio << "\n";
			std::cout << "Right: " << rightratio << "\n";
			std::cout << "Top: " << topratio << "\n";
		}
#endif

		if(lowratio < 0.2) {
			// There's a wall just in front of us: fall back
			steer.deactivate();
			steer.set_angle(0);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			steer.backward(720);
			steer.activate();
		} else if(leftratio < left_thresh) {
			// Too much wall on the left: turn right

			int div = 1;
			if(dir == CounterClockwise) div = 2;

			steer.set_angle(DEF_SPAN / div);
		} else if(rightratio < right_thresh) {
			// Too much wall on the right: turn left

			int div = 1;
			if(dir == Clockwise) div = 2;

			steer.set_angle(-DEF_SPAN / div);
		} else if(topratio < 0.4) {
			// Too much in front: turn in the direction we detected (cw, ccw)

			if(dir == CounterClockwise) steer.set_angle(-DEF_SPAN);
			if(dir == Clockwise) steer.set_angle(DEF_SPAN);
		} else {
			// No wall in either direction, just drive forwards
			steer.set_angle(0);
		}

		auto time_end = std::chrono::steady_clock::now();
		int diff = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
		int time_wait = TIME_VID_MS - diff;

		if(0 < time_wait) std::this_thread::sleep_for(std::chrono::milliseconds(time_wait));
	}

	// Stop motor, reset steer
	steer.set_angle(0);
	steer.deactivate();

	auto free_run_end = std::chrono::system_clock::now();
	int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(free_run_end-free_run_start).count();
	std::cout << "Needed: " << elapsed << "\n";
}

// === Obstacle run ===
// does absolutely nothing as we gave up on this
inline void obstacle_run() {
    using namespace std::chrono_literals;

	steer.set_angle(0);
	std::this_thread::sleep_for(500ms);

	while(state == ObstacleRun) {
		steer.activate();
	}

	steer.deactivate();
	steer.set_angle(0);
}

// ISR event to be handled on a button press
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

// ISR event to be handled on mode switch
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
	setupIOPins();
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

#ifdef DEBUG
		{
			Mat stream = cam.read();
			cam.stream_frame(stream);
		}
#endif

		std::this_thread::sleep_for(std::chrono::milliseconds(TIME_VID_MS));
	}

	std::cout << "Quitting\n";

	return 0;
}
