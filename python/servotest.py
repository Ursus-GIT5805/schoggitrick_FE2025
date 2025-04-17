import RPi.GPIO as GPIO
from time import sleep

PIN=18

# GPIO.cleanup()


GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)

pwm = GPIO.PWM(PIN, 50)
pwm.start(0)

def set_angle(ang):
    pwm.ChangeDutyCycle(ang)
    sleep(1)

# set_angle(2.5) # -90
set_angle(5.0) # -45
set_angle(7.5) # 0
set_angle(10.0) # 45
# set_angle(12.5) # 90

pwm.stop()
GPIO.cleanup()
