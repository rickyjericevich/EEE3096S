#!/usr/bin/python3
"""
Name: Ricky Jericevich
Student Number: JRCRIC001
Prac: Prac 1
Date: <26/07/2019>
"""

# import relevant librares
import RPi.GPIO as GPIO
from time import sleep
from itertools import product

# initialise pin mode
GPIO.setmode(GPIO.BCM)

# initialise buttons
btns = 23, 24
GPIO.setup(btns, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)

# initialise LEDs
leds = 17, 27, 22
GPIO.setup(leds, GPIO.OUT)
GPIO.output(leds, 0)	# ensure LEDs are off

# declare global variables
count = 0
list = list(product([0, 1], repeat = 3))

# when button 1 is pressed
def btn1_pressed(channel):
    global count, list
    count = 0 if count == 7 else count + 1	# count goes from 7 to 0 instead of 8
    GPIO.output(leds, list[count])		# LEDs display the count in binary

# when button 2 is pressed
def btn2_pressed(channel):
    global count, list
    count = 7 if count == 0 else count - 1	# count goes from 0 to 7 instead of -1
    GPIO.output(leds, list[count])		# LEDs display the count in binary

def main():
    sleep(10)	# wait for button press

# run main if
if __name__ == "__main__":
    try:
        # interrupts to detect button press
        GPIO.add_event_detect(btns[0], GPIO.RISING, callback = btn1_pressed, bouncetime = 200)
        GPIO.add_event_detect(btns[1], GPIO.RISING, callback = btn2_pressed, bouncetime = 200)
	# repeat main function
        while True:
            main()
    # correctly stop GPIOs
    except KeyboardInterrupt:
        GPIO.cleanup()
        print("Exiting gracefully")
    except Exception as e:
        GPIO.cleanup()
        print("Some other error occurred")
        print e
