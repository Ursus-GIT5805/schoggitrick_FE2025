#include <mutex>

#include <adafruitmotorhat.h>

#include <wiringPi.h>
#include <softPwm.h>

// GPIO pins of the tachometer
const int TACHO1 = 24;
const int TACHO2 = 22;

// GPIO pin of the servo motor
const int SERVO = 18;

// Angle decl
const int ANG_LEFT = 3;
const int ANG_RIGHT = 19;

static bool __up1 = false;
static std::mutex __up1_mutex;

static bool __up2 = false;
static std::mutex __up2_mutex;

static int __steps = false;
static std::mutex __step_mutex;

// void (*step_callback)(int step) = nullptr;

// on the current config: 1step ~= 0.825mm

void call1() {
	std::lock_guard<std::mutex> g1(__up1_mutex);
	std::lock_guard<std::mutex> g2(__up2_mutex);

	__up1 = digitalRead(TACHO1);

	if(!__up2) {
		std::lock_guard<std::mutex> g3(__step_mutex);

		if(__up1) __steps--;
		else __steps++;

		// if(step_callback != nullptr) step_callback(__steps);
	}
}

void call2() {
	std::lock_guard<std::mutex> g1(__up1_mutex);
	std::lock_guard<std::mutex> g2(__up2_mutex);

	__up2 = (bool)(digitalRead(TACHO2) == HIGH);
}

class Steer {
private:
	AdafruitMotorHAT hat;
	std::shared_ptr<AdafruitDCMotor> motor;

public:
	int power = 128;

	Steer() {
		softPwmCreate(SERVO, 0, 20);

		this->motor = this->hat.getMotor(1);

		pinMode(TACHO1, INPUT);
		pinMode(TACHO2, INPUT);

		wiringPiISR(TACHO1, INT_EDGE_BOTH, call1);
		wiringPiISR(TACHO2, INT_EDGE_BOTH, call2);
	}

	~Steer() {
		motor->setSpeed(0);
		motor->run(AdafruitDCMotor::kRelease);
	}

	void set_angle(int ang) {
		constexpr int DIFF = ANG_RIGHT-ANG_LEFT;
		constexpr double D2 = (double)DIFF / 2.0;
		constexpr double MID = (double)(ANG_LEFT + ANG_RIGHT) / 2.0;

		double res = MID + D2*((double)ang / 90.0);

		softPwmWrite(SERVO, res);
	}

	void forward(int steps) {
		int goal = __steps + steps;

		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kForward);

		while(__steps < goal) delay(1);

		motor->run(AdafruitDCMotor::kBrake);
		motor->setSpeed(0);
	}

	void backward(int steps) {
		int goal = __steps - steps;

		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kBackward);

		while(goal < __steps) delay(1);

		motor->run(AdafruitDCMotor::kBrake);
		motor->setSpeed(0);
	}

	int get_steps() {
		return __steps;
	}
};
