#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce
int delay = 1, sysTime[];

int initPeriphs(void){
    cout << "Setting up..." << endl;

    wiringPiSetup();

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
    wiringPiSPISetup(SPI_, SPI_SPEED);//DAC
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

    sysTime* = getTime();
    cout << "Started: ";
    printTime(sysTime);
    cout << endl;

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
            printData(humidity, temp, light, Vout);
            //check if alarm must sound
            if (alarmTime >= 180 && (Vout < 0.65 || Vout > 2.65)){
                alarmTime* = getTime();
                digitalWrite(ALARM, 1);
                cout << "Alarm on" << endl;
            }
        }
        delay(delay*1000);
        alarmTime += delay;
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
        sysTime* = getTime();
    }
    lastInterruptTime = interruptTime;
}

void samplingPeriod(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        delay = (delay == 5) ? 1 : delay*delay + 1;
        cout << +delay << "s sampling period" << endl;
    }
    lastInterruptTime = interruptTime;
}

void playpausePrint(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        paused = !paused
        cout << (paused ? "paused" : "unpaused") << endl;
    }
    lastInterruptTime = interruptTime;
}

int* getTime(){
    return {getHours(), getMins(), getSecs()};
}

void printData(float H, unsigned int T, float L, float V){
    int currentTime[] = {getHours(), getMins(), getSecs()};
    printTime(currentTime);
    cout << "\t";
    printTime({currentTime[0] - sysTime[0], currentTime[1] - sysTime[1], currentTime[2] - sysTime[2]});
    cout << "\t" << H << " V\t" << T << " C\t" << L << "\t" << V << " V\t";
    cout << (digitalRead(ALARM) ? "*" : "") << endl;
}

void printTime(unsigned char* time){
    cout << +time[0] << ":" << +time[1] << ":" << +time[2];
}
