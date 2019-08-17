/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 *
 * <JRCRIC001> <MKNASH006>
 * Date
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions
#include <softPwm.h> //for software PWM

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH,MM,SS;

void initGPIO(void){
	/*
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 * You can also use "gpio readall" in the command line to get the pins
	 * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
	 */
	printf("Setting up\n");
	wiringPiSetup(); //This is the default mode. If you want to change pinouts, be aware

	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

	//Set up the LEDS
	for(int i; i < sizeof(LEDS)/sizeof(LEDS[0]); i++){
	    pinMode(LEDS[i], OUTPUT);
	}

	//Set Up the Seconds LED for PWM
	pinMode(SECS, PWM_OUTPUT);
	if (softPwmCreate(26, 0, 59) != 0){
		printf("PWMCreate error\n");
	}
	printf("LEDS done\n");

	//Set up the Buttons
	for(int j; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
		pinMode(BTNS[j], INPUT);
		pullUpDnControl(BTNS[j], PUD_UP);
	}
	//Attach interrupts to Buttons
	if (wiringPiISR(BTNS[0], INT_EDGE_RISING, &minInc) < 0){
		printf("Hour Button ISR error\n");
	}
	if (wiringPiISR(BTNS[1], INT_EDGE_RISING, &minInc) < 0){
		printf("Minute Button ISR error\n");
	}
	printf("BTNS done\n");
	printf("Setup done\n");
}


/*
 * The main function
 * This function is called, and calls all relevant functions we've written
 */
int main(void){
	initGPIO();

	//Set time
	toggleTime();

	// Repeat this until we shut down
	for(;;){
		//Fetch the time from the RTC
		hours = hFormat(wiringPiI2CReadReg8(RTC, HOUR));
		mins = wiringPiI2CReadReg8(RTC, MIN);
		secs = wiringPiI2CReadReg8(RTC, SEC) - 0x80;
		//Function calls to toggle LEDs
		lightHours(hours);
		lightMins(mins);
		secPWM(secs);
		// Print out the time we have stored on our RTC
		printf("The current time is: %x:%x:%x\n", hours, mins, secs);

		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
	}
	return 0;
}

/*
 * Change the hour format to 12 hours
 */
int hFormat(int hours){
	/*formats to 12h*/
	if (hours >= 24){
		hours = 0;
	} else if (hours > 12){
		hours -= 12;
	}
	return (int)hours;
}

/*
 * Turns on corresponding LED's for hours
 */
void lightHours(int units){
	int digit[4];
	units = hexCompensation(units);
	int_to_binary_array(units, 4, digit);
	digitalWrite(0, digit[0]);
	digitalWrite(2, digit[1]);
	digitalWrite(3, digit[2]);
	digitalWrite(25, digit[3]);
}

/*
 * Turn on the Minute LEDs
 */
void lightMins(int units){
	int digit[6];
	units = hexCompensation(units);
	int_to_binary_array(units, 6, digit);
	digitalWrite(7, digit[0]);
	digitalWrite(22, digit[1]);
	digitalWrite(21, digit[2]);
	digitalWrite(27, digit[3]);
	digitalWrite(4, digit[4]);
	digitalWrite(6, digit[5]);
}

/*
 * PWM on the Seconds LED
 * The LED should have 60 brightness levels
 * The LED should be "off" at 0 seconds, and fully bright at 59 seconds
 */
void secPWM(int units){
	softPwmWrite(26, hexCompensation(units));
}

/*
 * hexCompensation
 * This function may not be necessary if you use bit-shifting rather than decimal checking for writing out time values
 */
int hexCompensation(int units){
	/*Convert HEX or BCD value to DEC where 0x45 == 0d45
	  This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
	  perform operations which work in base10 and not base16 (incorrect logic)
	*/
	int unitsU = units%0x10;

	if (units >= 0x50){
		units = 50 + unitsU;
	} else if (units >= 0x40){
		units = 40 + unitsU;
	} else if (units >= 0x30){
		units = 30 + unitsU;
	} else if (units >= 0x20){
		units = 20 + unitsU;
	} else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}


/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units){
	int unitsU = units%10;

	if (units >= 50){
		units = 0x50 + unitsU;
	} else if (units >= 40){
		units = 0x40 + unitsU;
	} else if (units >= 30){
		units = 0x30 + unitsU;
	} else if (units >= 20){
		units = 0x20 + unitsU;
	} else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}


/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void){
	//Debounce
	printf("increment hour");
	delay(200);
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 1 triggered, %x + 1\n", hours);
		//Fetch RTC Time
		hours = hexCompensation(wiringPiI2CReadReg8(RTC, HOUR));
		//Increase hours by 1, ensuring not to overflow
		hours = hFormat(hours+1);
		//Write hours back to the RTC
		hours = decCompensation(hours);
		wiringPiI2CWriteReg8(RTC, HOUR, hours);
	}
	lastInterruptTime = interruptTime;
}

/*
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void){
	//Debounce
	delay(200);
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 2 triggered, %x + 1\n", mins);
		//Fetch RTC Time
		mins = hexCompensation(wiringPiI2CReadReg8(RTC, MIN));
		//Increase minutes by 1, ensuring not to overflow
		mins += 1;
		if (mins >= 60){
		hourInc();
		mins = 0;
		}
		//Write minutes back to the RTC
		mins = decCompensation(mins);
		wiringPiI2CWriteReg8(RTC, MIN, mins);
	}
	lastInterruptTime = interruptTime;
}

//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void){
	delay(200);
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		HH = getHours();
		MM = getMins();
		SS = getSecs();

		HH = hFormat(HH);
		HH = decCompensation(HH);
		wiringPiI2CWriteReg8(RTC, HOUR, HH);

		MM = decCompensation(MM);
		wiringPiI2CWriteReg8(RTC, MIN, MM);

		SS = decCompensation(SS);
		wiringPiI2CWriteReg8(RTC, SEC, 0b10000000+SS);

	}
	lastInterruptTime = interruptTime;
}

void int_to_binary_array(unsigned int in, int count, int* out){
	/* assert: count <= sizeof(int)*CHAR_BIT */
	unsigned int mask = 1U << (count - 1);
	int i;
	for (i = 0; i < count; i++){
	out[i] = (in & mask) ? 1 : 0;
	in <<= 1;
	}
}
