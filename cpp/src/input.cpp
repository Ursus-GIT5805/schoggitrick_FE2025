#include <wiringPi.h>

#include <chrono>

// Time to wait until an hit is considered valid (in microseconds)
const int SIGNAL_IGNORE_MIC_SEC = 1000;

// GPIO BCM pins
const int PIN_LED = 23;
const int PIN_BUTTON = 27;
const int PIN_OP_MODE = 17;

// Mode: either FreeRun or ObstacleRun
enum Mode {
	FREE_RUN,
	OBSTACLE_RUN,
};

// Setup the pins used to communicate with humans
// i.e: LED, mode switch, and start button
inline void setupIOPins() {
	pinMode(PIN_LED, OUTPUT);
	pullUpDnControl(17, PUD_UP);

	pinMode(PIN_BUTTON, INPUT);
	pullUpDnControl(PIN_BUTTON, PUD_DOWN);

	pinMode(PIN_OP_MODE, INPUT);
	pullUpDnControl(PIN_OP_MODE, PUD_DOWN);
}

// Reads the current mode
inline Mode read_mode_switch() {
	if(digitalRead(PIN_OP_MODE) == HIGH) return FREE_RUN;
	return OBSTACLE_RUN;
}

inline void set_led(bool on) {
	if(on) digitalWrite(PIN_LED, HIGH);
	else digitalWrite(PIN_LED, LOW);
}

// Edge detection with false hit reduction (for the mode switch)
int op_mode_state = LOW;
void (*op_mode_callback)() = nullptr;

static void __op_pin_edge() {
	using namespace std::chrono_literals;

	int bef = digitalRead(PIN_OP_MODE);

	if(bef == op_mode_state) return;
	std::this_thread::sleep_for( std::chrono::microseconds(SIGNAL_IGNORE_MIC_SEC) );
	if(bef != digitalRead(PIN_OP_MODE)) return;

	op_mode_state = bef;

	if(op_mode_callback != nullptr) op_mode_callback();
}


// Edge detection with false hit reduction (for the button)
int button_state = LOW;
void (*button_callback)() = nullptr;

static void __button_edge() {
	using namespace std::chrono_literals;

	int bef = digitalRead(PIN_BUTTON);

	if(bef == button_state) return;
	std::this_thread::sleep_for( std::chrono::microseconds(SIGNAL_IGNORE_MIC_SEC) );
	if(bef != digitalRead(PIN_BUTTON)) return;

	button_state = bef;

	if(button_callback != nullptr) button_callback();
}
