import time
from adafruit_motorkit import MotorKit
from adafruit_motor import motor
import RPi.GPIO as GPIO

class MindstormsMotorBuilder:
    def __init__(self, motor):
        self.config = {}
        self.config['motor'] = motor

    def with_tacho(self, tacho1, tacho2, callback):
        self.config['tacho1'] = tacho1
        self.config['tacho2'] = tacho2
        self.config['tacho_callback'] = callback
        return self

    def build(self):
        return MindstormsMotor(self.config)

"""
Wrapper for the Mindstorms motor using the adafruit stepper motor HAT.
"""
class MindstormsMotor:
    def __init__(self, config):
        self.motor = config['motor']
        self.motor.decay_mode = (motor.SLOW_DECAY)

        if 'tacho1' in config and 'tacho2' in config:
            self.__tacho1 = config['tacho1']
            self.__tacho2 = config['tacho2']

            GPIO.setup(self.__tacho1, GPIO.IN)
            GPIO.setup(self.__tacho2, GPIO.IN)

            self.__high1 = GPIO.input(self.__tacho1)
            self.__high2 = GPIO.input(self.__tacho2)
            callback = config['tacho_callback']

            def callback1(pin):
                self.__high1 = GPIO.input(pin)
                if not self.__high2:
                    if self.__high1:
                        callback(-1)
                    else:
                        callback(1)

            def callback2(pin):
                self.__high2 = GPIO.input(pin)

            GPIO.add_event_detect(self.__tacho1, GPIO.BOTH, callback=callback1)
            GPIO.add_event_detect(self.__tacho2, GPIO.BOTH, callback=callback2)

    def __del__(self):
        self.motor.throttle = None

    def set_throttle(self, throttle):
        self.motor.throttle = throttle

    def get_throttle(self):
        return self.motor.throttle
