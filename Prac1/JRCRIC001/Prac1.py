#!/usr/bin/python3
"""
Name: Ricky Jericevich
Student Number: JRCRIC001
Prac: Prac 1
Date: <23/07/2019>
"""

# import relevant librares
import RPi.GPIO as GPIO
import time

# initialise GPIOs
GPIO.setmode(GPIO.BCM)
led1 = 17	# pin 17 is LED
GPIO.setup(led1, GPIO.OUT)
btn1 = 23	# pin 23 is button
GPIO.setup(btn1, GPIO.IN, pull_up_down=GPIO.PUD_DOWN) # initialise pin connected to button

def my_callback(channel):
    GPIO.output(led1, not GPIO.input(led1))	# pin changes state, LED changes state

def main():
    time.sleep(10) # wait for  button press

# run the functions if
if __name__ == "__main__":
    try:
        # interrupt to detect button press
        GPIO.add_event_detect(btn1, GPIO.RISING, callback=my_callback, bouncetime = 200)
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
