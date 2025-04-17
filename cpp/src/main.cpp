#include <chrono>
#include <thread>

#include <wiringPi.h>
#include "adafruitmotorhat.h"

const int LED = 23;

using namespace std::chrono_literals;

int main() {

	wiringPiSetupGpio();
	pinMode(LED, OUTPUT);

	for(int i = 0 ; i < 1 ; ++i) {
		digitalWrite(LED, HIGH);
		delay(500);
		digitalWrite(LED, LOW);
		delay(500);
	}

    // connect using the default device address 0x60
    AdafruitMotorHAT hat;

    // get the motor connected to port 1
    if (auto motor { hat.getMotor (1) }) {
        motor->setSpeed (255);

        motor->run (AdafruitDCMotor::kForward);
        std::this_thread::sleep_for (1s);

        motor->run (AdafruitDCMotor::kBackward);
        std::this_thread::sleep_for (1s);

        // release the motor after use
        motor->run (AdafruitDCMotor::kRelease);
    }

	return 0;
}
