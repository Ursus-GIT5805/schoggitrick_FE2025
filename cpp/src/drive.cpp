
#include <iostream>
#include <mutex>
#include <math.h>

#include <adafruitmotorhat.h>

#include <type_traits>
#include <wiringPi.h>
#include <softPwm.h>

// GPIO BCM pins of the tachometer/servo motor
const int TACHO1 = 24;
const int TACHO2 = 22;

const int PIN_SERVO = 18;

// Angle declarations
const int ANG_LEFT = 3;
const int ANG_RIGHT = 19;

static bool __up1 = false;
static bool __up2 = false;

static int __steps = false;

// ===== Distances are in millimeters =====

const double into_rad(double ang) {
	return ang / 180.0 * M_PI;
}

const int TACHO_COUNT_PER_WIND = 360; // 360 steps per turn
const double WHEEL_DIAMETER = 43.2;

const double RAD_PER_TACHO = 2.0 * M_PI / (double)TACHO_COUNT_PER_WIND;
const double WHEEL_RADIUS = WHEEL_DIAMETER / 2.0;

const double STEP_DIS = WHEEL_RADIUS * RAD_PER_TACHO;

const double WHEEL_BASE = 120; // Distance between front and back wheels center
const double TREAD = 88.5; // Distance between both front or back wheels

const double ANGLE_SPAN_RIGHT = 27.0;
const double ANGLE_SPAN_RIGHT_RAD = into_rad(ANGLE_SPAN_RIGHT);

const double ANGLE_SPAN_LEFT = 28.0;
const double ANGLE_SPAN_LEFT_RAD = into_rad(ANGLE_SPAN_LEFT);

const double DEF_SPAN = std::min(ANGLE_SPAN_RIGHT, ANGLE_SPAN_LEFT);

const double STEERING_VELOCITY = (ANGLE_SPAN_LEFT + ANGLE_SPAN_RIGHT) / 0.16;
const double STEERING_VELOCITY_RAD = into_rad(STEERING_VELOCITY);

static void __call_tacho1() {
	__up1 = digitalRead(TACHO1);

	if(!__up2) {
		if(!__up1) __steps++;
		else __steps--;
	}
}

static void _call_tacho2() {
	__up2 = (bool)(digitalRead(TACHO2) == HIGH);

	if(__up1) {
		if(!__up2) __steps++;
		else __steps--;
	}
}

// Wrapper around the mindstorms motor (driving) and the servo
// motor (steering)
class Steer {
private:
	AdafruitMotorHAT hat;
	std::shared_ptr<AdafruitDCMotor> motor;

public:
	// Also known as throttle
	int power = 128;
	double angle = 0.0;

	Steer() {
		this->motor = this->hat.getMotor(1);
	}

	// Initialises the servo motor and the tacho meter
	// wiringPiSetup must be called before!
	void init() {
		softPwmCreate(PIN_SERVO, 0, 20);

		pinMode(TACHO1, INPUT);
		pinMode(TACHO2, INPUT);

		__up1 == digitalRead(TACHO1);
		__up2 == digitalRead(TACHO2);

		wiringPiISR(TACHO1, INT_EDGE_BOTH, __call_tacho1);
		wiringPiISR(TACHO2, INT_EDGE_BOTH, _call_tacho2);
	}

	~Steer() {
		motor->setSpeed(0);
		motor->run(AdafruitDCMotor::kRelease);
	}

	// Sets the current angle
	void set_angle(double ang) {
		ang = std::min(ANGLE_SPAN_RIGHT, ang);
		ang = std::max(-ANGLE_SPAN_LEFT, ang);

		constexpr int DIFF = ANG_RIGHT-ANG_LEFT;
		constexpr double D2 = (double)DIFF / 2.0;
		constexpr double MID = (double)(ANG_LEFT + ANG_RIGHT) / 2.0;

		int span = ANGLE_SPAN_RIGHT;
		if(ang < 0) span = ANGLE_SPAN_LEFT;

		double res = MID + D2*((double)ang / span);

		this->angle = into_rad(ang);
		std::cout << "New angle: " << this->angle << "\n";
		std::cout << "Set servo: " << res << "\n";

		softPwmWrite(PIN_SERVO, (int)res);
	}

	// unused
	void turn(double ang) {
		double sign = 1.0;

		double L = WHEEL_BASE;

		if(ang < 0) sign = -1.0;
		// else L = WHEEL_BASE + TREAD;

		set_angle(sign*DEF_SPAN);
		delay(500);

		double sine = std::sin(this->angle);
		double rad = into_rad(ang);

		double dis = rad * L / sine;

		double steps = dis / STEP_DIS;

		std::cout << sine << "\n";
		std::cout << dis << "\n";
		std::cout << steps << "\n";

		this->forward((int)steps);
	}

	void activate() {
		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kForward);
	}

	void deactivate() {
		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kBrake);
	}

	void forward(int steps) {
		int goal = __steps + steps;

		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kForward);

		while(__steps < goal) delay(1);

		motor->run(AdafruitDCMotor::kBrake);
		motor->setSpeed(0);
	}

	void forward_dis(double dis) {
		double steps = dis / STEP_DIS;
		this->forward((int)steps);
	}

	void backward(int steps) {
		int goal = __steps - steps;

		motor->setSpeed(this->power);
		motor->run(AdafruitDCMotor::kBackward);

		while(goal < __steps) delay(1);

		motor->run(AdafruitDCMotor::kBrake);
		motor->setSpeed(0);
	}

	void reset_steps() {
		__steps = 0;
	}

	int get_steps() {
		return __steps;
	}
};
