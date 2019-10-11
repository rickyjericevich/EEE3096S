#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce 
int RTC; //Holds the RTC instance
unsigned short int light = 0, temp = 0, humidity = 0;
unsigned char delay = 1;
unsigned int sysTime[];

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
    softToneCreate (??)
    //cout << "Alarm Buzzer done" << endl;

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
    //sysTime[] = currentime
    
    while (1){
        if (!paused){
            
        }
	delay(delay*1000);
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
    cout << "Thread created" << endl;
}

void *dataThread(void *threadargs){
    while(1){
        
        if ((currenttime - alarmtime) >= 3 min && (Vout < 0.65 || Vout > 2.65)){
            alarmtime = 0
            softToneWrite(ALARM, 1000); // 1kHz frequency
            cout << "Alarm on" << endl;
        }
    }
	
}

void resetAlarm(){
	// Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        softToneWrite(ALARM, 0);
        cout << "Alarm off" << endl;
    }
    lastInterruptTime = interruptTime;
}

void fullReset(){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        //clear console
        
        //sysTime = currentime
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
