#define BLYNK_PRINT stdout
#include <BlynkApiWiringPi.h>
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>
#include "project.h"
using namespace std;

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

bool paused = 0;
unsigned int lastInterruptTime = 0; //Used for button debounce
int period = 1;
time_t sysTime;

int main(int argc, char* argv[]){
    const char *auth, *serv;
    uint16_t port;
    parse_options(argc, argv, auth, serv, port);

    Blynk.begin(auth, serv, port);
    // set up the pi
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

    while(true) {
        Blynk.run();
    }

    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
}

void initPeriphs(void){
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

void *dataThread(void *threadargs){
    float humidity, Vout;
    int temp, light, alarmOn;
    unsigned int t1, t2;
    unsigned char Vdac;
    unsigned char buffer[2];
    time_t now, alarmTime = 0;

    delay(1000);
    printHeaders();

    while(1){
        if (!paused){
            t1 = millis();
            //get data
            humidity = 3.61*analogRead(HUMIDITY)/1023;
            light = analogRead(LDR);
            temp = (3.28*analogRead(THERMISTOR)/1023 - 0.5)/0.01;
            Vout = light*humidity/1023;

            Vdac = Vout*1023/3.28;
            buffer[0] = (0b1011 << 4) | 0b1111;//(Vdac >> 6);
            buffer[1] = 0b11111100;//Vdac << 2;
            wiringPiSPIDataRW(DAC_SPI, buffer , 2);

            time(&now);
            //check if alarm must sound
            if ((difftime(now, alarmTime) >= 180) && (Vout < 0.65 || Vout > 2.65)){
                time(&alarmTime);
                digitalWrite(ALARM, 1);
            }

            outputData(now, humidity, temp, light, Vout);

            t2 = millis();
            delay(period*1000 - (t2 - t1));
        } else {
            delay(200); //uses less cpu
        }
    }
    pthread_exit(NULL);
}

void resetAlarm(void){
    // Debounce
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        digitalWrite(ALARM, 0);
        Blynk.virtualWrite(V2, 0);
    }
    lastInterruptTime = interruptTime;
}

void fullReset(void){
    // Debounce
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        //clear console
        printHeaders();
        time(&sysTime);
    }
    lastInterruptTime = interruptTime;
}

void samplingPeriod(void){
    // Debounce
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200) period = (period == 5) ? 1 : period*period + 1;
    lastInterruptTime = interruptTime;
}

void toggleMonitoring(void){
    // Debounce
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        paused = !paused;
        if (paused){
            Blynk.virtualWrite(V0, "--:--:--");
            Blynk.virtualWrite(V1, "- ");
            Blynk.virtualWrite(V3, "-");
            Blynk.virtualWrite(V4, "-");
            Blynk.virtualWrite(V5, "-");
            Blynk.virtualWrite(V6, "-");
        }
    }
    lastInterruptTime = interruptTime;
}

void printHeaders(void){
    Blynk.virtualWrite(V7, "clr", "RTC Time Uptime   Humid Temp Light DAC Alm\n");
    system("clear");
    delay(150);
    printf("RTC Time\tSys Timer\tHumidity\tTemp\tLight\tDAC Out\tAlarm\n");
}

void outputData(time_t N, float H, int T, int L, float V){
    time_t S = (time_t)difftime(N, sysTime);
    struct tm *t1 = localtime(&N), *t2 = gmtime(&S);

    char s[9];
    strftime(s, 9, "%X", t1);
    printf("%s\t", s);
    Blynk.virtualWrite(V7, s, " ");

    int A = digitalRead(ALARM);
    strftime(s, 9, "%X", t2);
    printf("%s\t%1.2f V\t\t%d C\t%d\t%1.2f V\t%s\n", s, H, T, L, V, (A ? "*" : ""));

    char p[30];
    if (L > 999){
        sprintf(p, "%s %1.2fV %dC  %d  %1.2fV %s\n",s, H, T, L, V, (A ? "*" : ""));
    } else {
        sprintf(p, "%s %1.2fV %dC  %d   %1.2fV %s\n", s, H, T, L, V, (A ? "*" : ""));
    }

    Blynk.virtualWrite(V7, p);
    Blynk.virtualWrite(V0, s);
    Blynk.virtualWrite(V1, period);
    Blynk.virtualWrite(V2, A);
    Blynk.virtualWrite(V3, H);
    Blynk.virtualWrite(V4, T);
    Blynk.virtualWrite(V5, L);
    Blynk.virtualWrite(V6, V);
}
