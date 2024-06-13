# -*- coding: utf-8 -*-

import RPi.GPIO as GPIO
import time
from AlphaBot2.python.motor_test import AlphaBot2
from AlphaBot2.python.TRSensors import TRSensor

whl = AlphaBot2()

Button = 7
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(Button, GPIO.IN, GPIO.PUD_UP)
print("start")

TR = TRSensor()

while GPIO.input(Button) != 0:
    time.sleep(0.05)

try:
    while True:
        sensor = TR.AnalogRead()
        left_sensor = sensor[0]
        left_sensor_2 = sensor[1]
        middle_sensor = sensor[2]
        right_sensor = sensor[3]
        right_sensor_2 = sensor[4]
        
        if middle_sensor < 500:
            whl.forward(30)
            time.sleep(0.01)
            
        if left_sensor < 500:
            whl.left(20)
            time.sleep(0.01)
            
        if right_sensor < 500:
            whl.right(20)
            time.sleep(0.01)
            
        # if left_sensor_2 < 400:
        #     whl.left(15)
        #     time.sleep(0.05)
          
        # if right_sensor_2 < 400:
        #     whl.right(15)
        #     time.sleep(0.05)
except:
    print("error")
    GPIO.cleanup()
