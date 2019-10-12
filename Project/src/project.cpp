#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce
int period = 1, sysTime = 0;

void initPeriphs(void){
    system("clear");
    printf("\n");
    wiringPiSetup();

    //Set up the buttons
    for(int i = 0; i < sizeof(BTNS)/sizeof(BTNS[0]); i++){
        pinMode(BTNS[i], INPUT);
        pullUpDnControl(BTNS[i], PUD_UP);
    }

    //Set up the tone for the alarm buzzer
    pinMode(ALARM, OUTPUT);

    //Attach interrupts to buttons
    wiringPiISR(BTNS[0], INT_EDGE_FALLING, &resetAlarm);
    wiringPiISR(BTNS[1], INT_EDGE_FALLING, &fullReset);
    wiringPiISR(BTNS[2], INT_EDGE_FALLING, &samplingPeriod);
    wiringPiISR(BTNS[3], INT_EDGE_FALLING, &toggleMonitoring);

    // Setting up SPI
    wiringPiSPISetup(DAC_SPI, SPI_SPEED);//DAC
    mcp3004Setup(100, ADC_SPI);
}

int main(void){
    // Call the setup GPIO function
    initPeriphs();

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

    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
}

void *dataThread(void *threadargs){
    float humidity, Vout;
    int temp, light, alarmTime = 180;
    printf("RTC Time\tSys Timer\tHumidity\tTemp\tLight\tDAC Out\tAlarm\n");
    while(1){
        if (!paused){
            //get data
            humidity = 3.28*analogRead(HUMIDITY)/1023;
            light = analogRead(LDR);
            temp = (3.28*analogRead(THERMISTOR)/1023 - 0.5)/0.01;
            //Vout
            Vout = light*humidity/1023;

            //check if alarm must sound
            if ((alarmTime >= 180) && (Vout < 0.65 || Vout > 2.65)){
                alarmTime = 0;
                digitalWrite(ALARM, 1);
            }

            //print data
            time_t now;
            time(&now);
            struct tm * t;
            t = localtime(&now);
            char s[9];
            strftime(s, 9, "%X", t);
            printf("%s\t", s);
            printTime(sysTime);
            printf("%1.2f V\t\t%d C\t%d\t%1.2f V\t%s\n", humidity, temp, light, Vout, (digitalRead(ALARM) ? "*" : ""));
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
    if (interruptTime - lastInterruptTime > 200) digitalWrite(ALARM, 0);
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
    if (interruptTime - lastInterruptTime > 200) period = (period == 5) ? 1 : period*period + 1;
    lastInterruptTime = interruptTime;
}

void toggleMonitoring(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200) paused = !paused;
    lastInterruptTime = interruptTime;
}

void printTime(int secs){
    int mins = secs/60;
    printf("%.2d:%.2d:%.2d\t", int(mins/60), int(mins%60), secs%60);
}
