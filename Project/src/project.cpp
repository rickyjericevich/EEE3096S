#include "project.h"
using namespace std;

bool paused = 0;
long lastInterruptTime = 0; //Used for button debounce
int period = 1;
time_t sysTime;

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

    time(&sysTime);
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
    int temp, light;
    time_t alarmTime = 0;
    printf("RTC Time\tSys Timer\tHumidity\tTemp\tLight\tDAC Out\tAlarm\n");
    while(1){
        if (!paused){
            //get data
            humidity = 3.28*analogRead(HUMIDITY)/1023;
            light = analogRead(LDR);
            temp = (3.28*analogRead(THERMISTOR)/1023 - 0.5)/0.01;
            //Vout
            Vout = light*humidity/1023;

            time_t now;
            time(&now);
            //check if alarm must sound
            if ((difftime(now, alarmTime) >= 180) && (Vout < 0.65 || Vout > 2.65)){
                time(&alarmTime);
                digitalWrite(ALARM, 1);
            }

            printData(now, (time_t)difftime(now, sysTime), humidity, temp, light, Vout);
            }
        delay(period*1000);
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
        delay(150);
        printf("RTC Time\tSys Timer\tHumidity\tTemp\tLight\tDAC Out\tAlarm\n");
        time(&sysTime);
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

void printData(time_t N, time_t S, float H, int T, int L, float V){
    struct tm * t;
    t = localtime(&N);

    char s[9];
    strftime(s, 9, "%X", t);
    printf("%s\t", s);

    t = gmtime(&S);
    strftime(s, 9, "%X", t);
    printf("%s\t%1.2f V\t\t%d C\t%d\t%1.2f V\t%s\n",s , H, T, L, V, (digitalRead(ALARM) ? "*" : ""));
}

void printTime(int secs){
    int mins = secs/60;
    printf("%.2d:%.2d:%.2d\t", int(mins/60), int(mins%60), secs%60);
}
