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
GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)

def main():
    GPIO.output(17, 1)		   #pin goes high, LED turns on
    while True:			   #repeatedly turn LED on/off
        state = GPIO.input(17)	   #determine if pin is high or low
        time.sleep(1) 		   #wait 1 second
        GPIO.output(17, not state) #pin changes state, LED changes state

# Only run the functions if 
if __name__ == "__main__":
    # Make sure the GPIO is stopped correctly
    try:
        while True:
            main()
    except KeyboardInterrupt:
        print("Exiting gracefully")
        # Turn off your GPIOs here
        GPIO.cleanup()
    except Exception as e:
        GPIO.cleanup()
        print("Some other error occurred")
        print e
