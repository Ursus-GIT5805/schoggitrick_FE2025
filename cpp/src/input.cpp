#include <wiringPi.h>

const int PIN_LED = 23;
const int PIN_BUTTON = 27;
const int PIN_OP_MODE = 17;

enum Mode {
	FREE_RUN,
	OBSTACLE_RUN,
};

inline void setupInputPins() {
	pinMode(PIN_LED, OUTPUT);
	pullUpDnControl(17, PUD_UP);

	pinMode(PIN_BUTTON, INPUT);
	pullUpDnControl(PIN_BUTTON, PUD_DOWN);

	pinMode(PIN_OP_MODE, INPUT);
	pullUpDnControl(PIN_OP_MODE, PUD_DOWN);
}

inline Mode read_mode_switch() {
	if(digitalRead(PIN_OP_MODE) == HIGH) return FREE_RUN;
	return OBSTACLE_RUN;
}

inline void set_led(bool on) {
	if(on) digitalWrite(PIN_LED, HIGH);
	else digitalWrite(PIN_LED, LOW);
}
