#include "project.h"

long lastInterruptTime = 0; //Used for button debounce 
int RTC; //Holds the RTC instance
usigned short int light = 0, temp = 0, humidity = 0;
char delay = 1;
int sysTime[] = {0, 0, 0};

int initPeriphs(){
    printf("Setting up...\n");

    wiringPiSetup();

    //RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

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

    // Setting up SPI
    wiringPiSPISetup(SPI_CHAN0, SPI_SPEED);//DAC
    wiringPiSPISetup(SPI_CHAN1, SPI_SPEED);//ADC
    printf("SPI setup done\n");

    printf("Setup done\n");

    return 0;
}

int main(){
    // Call the setup GPIO function
    if(initPeriphs() == -1){
        return 0;
    }
    initThread();
    
    while (1){
        if (!paused){
            
        }
    }
}

void initThread(){
    pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;
    
    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); // Safe to get existing scheduling param
    param.sched_priority = newprio;              // Set the priority; others are unchanged
    pthread_attr_setschedparam (&tattr, &param); // Setting the new scheduling param
    pthread_create(&thread_id, &tattr, dataThread, (void *)-1); // With new priority specified
}

void *dataThread(void *threadargs){
    while(1){
        
    }
}
