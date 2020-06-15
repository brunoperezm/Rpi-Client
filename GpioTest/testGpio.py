import RPi.GPIO as gpio
import time
gpio.setmode(gpio.BOARD)

gpio.setup(7, gpio.OUT)
gpio.output(7, True)
time.sleep(1)
gpio.output(7, False)
