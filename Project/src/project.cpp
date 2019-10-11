#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce 
int RTC; //Holds the RTC instance
unsigned short int light = 0, temp = 0, humidity = 0;
unsigned char delay = 1, sysTime[];

int initPeriphs(){
    cout << "Setting up..." << endl;

    wiringPiSetup();

    //RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

    //Set up the buttons
    for(int i; i < sizeof(BTNS)/sizeof(BTNS[0]); i++){ 
        pinMode(BTNS[i], INPUT);
        pullUpDnControl(BTNS[i], PUD_UP);
    }
    cout << "Buttons done" << endl;

    //Set up the tone for the alarm buzzer
    pinMode(ALARM, OUTPUT);
    cout << "Alarm Buzzer done" << endl;

    //Attach interrupts to buttons 
    if (wiringPiISR(BTN[0], INT_EDGE_FALLING, &resetAlarm) < 0){
        cout << "Alarm Reset button (BTN[0]) ISR error" << endl;
    }
    if (wiringPiISR(BTN[1], INT_EDGE_FALLING, &fullReset) < 0){
        cout << "Full Reset button (BTN[1]) ISR error" << endl;
    }
    if (wiringPiISR(BTN[2], INT_EDGE_FALLING, &samplingPeriod) < 0){
        cout << "Sampling Period button (BTN[2]) ISR error" << endl;
    }
    if (wiringPiISR(BTN[3], INT_EDGE_FALLING, &playpausePrint) < 0){
        cout << "Print button (BTN[3]) ISR error" << endl;
    }
    cout << "BTNS done" << endl;

    // Setting up SPI
    wiringPiSPISetup(SPI_CHAN0, SPI_SPEED);//DAC
    wiringPiSPISetup(SPI_CHAN1, SPI_SPEED);//ADC
    cout << "SPI setup done" << endl;

    cout << "Setup done" << endl;
    return 0;
}

int main(){
    // Call the setup GPIO function
    if (initPeriphs() == -1) return 0;
    initThread();
    sysTime* = getTime();
    printTime(sysTime);
    
    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
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
    cout << "Thread created" << endl;
}

void *dataThread(void *threadargs){
    unsigned char alarmTime* = getTime();
    while(1){
        if (!paused){
            unsigned char currentTime* = getTime();
            //get data
            
            //type Vout = ...
            
            //print data
            
            //check if alarm must sound
            if ((currentTime[1] - alarmTime[1]) >= 3 && (Vout < 0.65 || Vout > 2.65)){
                alarmTime* = getTime();
                digitalWrite(ALARM, 1);
                cout << "Alarm on" << endl;
            }
        }
        delay(delay*1000);
    }
    pthread_exit(NULL);
}

void resetAlarm(){
	// Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        digitalWrite(ALARM, 0);
        cout << "Alarm off" << endl;
    }
    lastInterruptTime = interruptTime;
}

void fullReset(){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        //clear console
        
        sysTime* = getTime();
    }
    lastInterruptTime = interruptTime;
}

void samplingPeriod(){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        delay = (delay == 5) ? 1 : delay*delay + 1;
        cout << +delay << "s sampling period" << endl;
    }
    lastInterruptTime = interruptTime;
}

void playpausePrint(){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        paused = !paused
        cout << (paused ? "paused" : "unpaused") << endl;
    }
    lastInterruptTime = interruptTime;
}

unsigned char* getRTCTime(){
    unsigned char time[] = {
        BCDtoDecimal(wiringPiI2CReadReg8(RTC, HOUR)),
        BCDtoDecimal(wiringPiI2CReadReg8(RTC, MIN)),
        BCDtoDecimal(wiringPiI2CReadReg8(RTC, SEC) - 0x80);//double check this
                           }
    return time;
}

void printTime(unsigned char* time){
        cout << +time[0] << ":" << +time[1] << ":" << +time[2] << endl;
}

unsigned char BCDtoDecimal(unsigned char bcd){
    return bcd - 6*(bcd >> 4);
}
