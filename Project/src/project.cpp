#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce
int period = 1, sysTime = 0;

int initPeriphs(void){
    cout << "Setting up..." << endl;

    wiringPiSetup();

    //Set up the buttons
    for(int i = 0; i < sizeof(BTNS)/sizeof(BTNS[0]); i++){
        pinMode(BTNS[i], INPUT);
        pullUpDnControl(BTNS[i], PUD_UP);
    }
    cout << "Buttons done" << endl;

    //Set up the tone for the alarm buzzer
    pinMode(ALARM, OUTPUT);
    cout << "Alarm Buzzer done" << endl;

    //Attach interrupts to buttons
    if (wiringPiISR(BTNS[0], INT_EDGE_FALLING, &resetAlarm) < 0){
        cout << "Alarm Reset button (BTNS[0]) ISR error" << endl;
    }
    if (wiringPiISR(BTNS[1], INT_EDGE_FALLING, &fullReset) < 0){
        cout << "Full Reset button (BTNS[1]) ISR error" << endl;
    }
    if (wiringPiISR(BTNS[2], INT_EDGE_FALLING, &samplingPeriod) < 0){
        cout << "Sampling Period button (BTNS[2]) ISR error" << endl;
    }
    if (wiringPiISR(BTNS[3], INT_EDGE_FALLING, &toggleMonitoring) < 0){
        cout << "Print button (BTNS[3]) ISR error" << endl;
    }
    cout << "BTNS done" << endl;

    // Setting up SPI
    wiringPiSPISetup(DAC_SPI, SPI_SPEED);//DAC
    mcp3004Setup(100, ADC_SPI);
    cout << "SPI setup done" << endl;

    cout << "Setup done" << endl;
    return 0;
}

int main(void){
    // Call the setup GPIO function
    if (initPeriphs() == -1) return 0;
    //create thread
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

    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
}

void *dataThread(void *threadargs){
    int alarmTime = 180;
    while(1){
        if (!paused){
            //get data
            float humidity = 3.32*analogRead(HUMIDITY)/1023;
            int light = analogRead(LDR);
            float temp = ((3.32*analogRead(THERMISTOR)/1023) - 0.5)/0.01;
            //Vout
            float Vout = light*temp/1023;

            //print data
            //printData(humidity, temp, light, Vout);
            int *currentTime = getTime();
            //printTime(currentTime);
            cout << "\t0:0:" << sysTime;//fix
            cout << "\t" << humidity << " V\t" << temp << " C\t" << light << "\t" << Vout << " V\t";
            cout << (digitalRead(ALARM) ? "*" : "") << endl;

            //check if alarm must sound
            if (alarmTime >= 180 && (Vout < 0.65 || Vout > 2.65)){
                alarmTime = 0;
                digitalWrite(ALARM, 1);
                cout << "Alarm on" << endl;
            }
        }
        delay(period*1000);
        alarmTime += period;
	sysTime += period;
    }
    pthread_exit(NULL);
}

void resetAlarm(void){
	// Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        if (digitalRead(ALARM)){
            digitalWrite(ALARM, 0);
            cout << "Alarm off" << endl;
        }
    }
    lastInterruptTime = interruptTime;
}

void fullReset(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        //clear console
        system("clear");
        sysTime = 0;
    }
    lastInterruptTime = interruptTime;
}

void samplingPeriod(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        period = (period == 5) ? 1 : period*period + 1;
        cout << period << "s sampling period" << endl;
    }
    lastInterruptTime = interruptTime;
}

void toggleMonitoring(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        paused = !paused;
        cout << (paused ? "paused" : "unpaused") << endl;
    }
    lastInterruptTime = interruptTime;
}

int* getTime(){
    int time[3] = {getHours(), getMins(), getSecs()};
    return time;
}

void printData(float H, int T, float L, float V){

}

void printTime(int* time){
    cout << time[0] << ":" << time[1] << ":" << time[2];
}
