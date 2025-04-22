#include <wiringPi.h>

#include "input.cpp"
#include "drive.cpp"
#include "camera.cpp"

int main() {
	if( wiringPiSetupGpio() != 0 ) {
		std::cerr << "Could not setup GPIO!" << std::endl;
		return 1;
	}
	setupInputPins();

	Steer steer;
	Camera cam;

	while (true) {
		cv::Mat frame = cam.read();
		cam.stream_frame(frame);
		delay(20);
	}

	// steer.set_angle(0);

	// steer.forward(200);
	// std::cout << steer.get_steps() << std::endl;

	// steer.backward(200);
	// std::cout << steer.get_steps() << std::endl;

	// delay(100);
}

/* int main3() {
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
