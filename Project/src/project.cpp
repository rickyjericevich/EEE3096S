#include "project.h"

long lastInterruptTime = 0; //Used for button debounce 
int RTC; //Holds the RTC instance
usigned short int light = 0, temp = 0, humidity = 0;
char delay = 1;
int sysTime[] = {0, 0, 0};

int initPeriphs(void){

    printf("Setting up...\n");

    wiringPiSetup();

    RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

    //Set up the buttons
    for(int i; i < sizeof(BTNS)/sizeof(BTNS[0]); i++){ 
        pinMode(BTNS[i], INPUT);
        pullUpDnControl(BTNS[i], PUD_UP);
    }
    printf("Buttons done\n");

    //Set up the tone for the alarm buzzer 
    
    //printf("Alarm Buzzer done\n");

    //Attach interrupts to buttons 
    if (wiringPiISR(BTN[0], INT_EDGE_FALLING, &resetAlarm) < 0){
	    printf("Alarm Reset button (BTN[0]) ISR error\n");
    }
    if (wiringPiISR(BTN[1], INT_EDGE_FALLING, &fullReset) < 0){
	    printf("Full Reset button (BTN[1]) ISR error\n");
    }
    if (wiringPiISR(BTN[2], INT_EDGE_FALLING, &samplingPeriod) < 0){
	    printf("Sampling Period button (BTN[2]) ISR error\n");
    }
    if (wiringPiISR(BTN[3], INT_EDGE_FALLING, &playpausePrint) < 0){
	    printf("Print button (BTN[3]) ISR error\n");
    }
    printf("BTNS done\n"); 

    // Setting up the SPI interface
    wiringPiSPISetup(SPI_CHAN0, SPI_SPEED);
    wiringPiSPISetup(SPI_CHAN1, SPI_SPEED);
    printf("SPI setup done\n");

    printf("Setup done\n");

    return 0;
}
