import time
import math
import board
from adafruit_motorkit import MotorKit
from motor.mindstorms_motor import *

kit = MotorKit(i2c=board.I2C())

def callback(v):
    print(v)

motor = MindstormsMotorBuilder(kit.motor1).with_tacho(14, 15, callback).build()
# motor = MindstormsMotorBuilder(kit.motor1).build()

try:
    # i = 0
    # delay = 0.5
    while True:
        pass
        # throttle = math.sin(i)
        # throttle = 1.0
        # motor.set_throttle(-throttle)
        # time.sleep(delay)
        # motor.set_throttle(throttle)
        # time.sleep(delay)
        # i += 1

finally:
    motor.set_throttle(None)
