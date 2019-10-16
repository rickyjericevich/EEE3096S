// After installing the blynk library, place main.cpp, Makefile, project.h
// in the blynk-library/linux folder. Build using make and run using make run

#define BLYNK_PRINT stdout
#include <BlynkApiWiringPi.h>
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>
#include "project.h"
using namespace std;

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

bool paused = false, printing = false;
unsigned int lastInterruptTime = 0, period;
time_t sysTime;

int main(int argc, char* argv[]){
    const char *auth, *serv;
    uint16_t port;
    parse_options(argc, argv, auth, serv, port);

    Blynk.begin(auth, serv, port);

    initPeriphs();
    piThreadCreate(dataThread);

    while(true) {
        Blynk.run();
    }

    return 0;
}

void initPeriphs(void){

    for(int i = 0; i < sizeof(BTNS)/sizeof(BTNS[0]); i++){
        pinMode(BTNS[i], INPUT);
        pullUpDnControl(BTNS[i], PUD_UP);
    }

    pinMode(ALARM, OUTPUT);

    wiringPiISR(BTNS[0], INT_EDGE_FALLING, &resetAlarm);
    wiringPiISR(BTNS[1], INT_EDGE_FALLING, &fullReset);
    wiringPiISR(BTNS[2], INT_EDGE_FALLING, &samplingPeriod);
    wiringPiISR(BTNS[3], INT_EDGE_FALLING, &toggleMonitoring);

    wiringPiSPISetup(DAC_SPI, SPI_SPEED);
    mcp3004Setup(100, ADC_SPI);

    time(&sysTime);
}

PI_THREAD (dataThread){
    float humidity, Vout;
    int temp, light;
    unsigned int t1;
    time_t now, alarmTime = 0;

    delay(1000);
    period = 1;
    printHeaders();

    while(true){
        if (!paused){
            t1 = millis();

            humidity = 3.61*analogRead(HUMIDITY)/1023;
            light = analogRead(LDR);
            temp = (3.28*analogRead(THERMISTOR)/1023 - 0.5)/0.01;
            Vout = light*humidity/1023;

            unsigned char Vdac = Vout*1023/3.61;
            unsigned char buffer[2] = {(0b011 << 4) | (Vdac >> 4), Vdac << 2};
            wiringPiSPIDataRW(DAC_SPI, buffer , 2);

            time(&now);
            if ((difftime(now, alarmTime) >= 10) && (Vout < 0.65 || Vout > 2.65)){
                time(&alarmTime);
                digitalWrite(ALARM, 1);
            }

            outputData(now, humidity, temp, light, Vout);

            delay(period*1000 - (millis() - t1));
        } else {
            delay(200);
        }
    }
    return NULL;
}

void resetAlarm(void){
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        digitalWrite(ALARM, 0);
        Blynk.virtualWrite(V2, 0);
    }
    lastInterruptTime = interruptTime;
}

void fullReset(void){
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        printHeaders();
        time(&sysTime);
    }
    lastInterruptTime = interruptTime;
}

void samplingPeriod(void){
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        period = (period == 5) ? 1 : period*period + 1;
        if (!paused){
           while (printing);
           Blynk.virtualWrite(V1, period);
        }
    }
    lastInterruptTime = interruptTime;
}

void toggleMonitoring(void){
    unsigned int interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
        paused = !paused;
        if (paused){
            while (printing);
            delay(1000);
            Blynk.virtualWrite(V0, "--:--:--");
            Blynk.virtualWrite(V1, "- ");
            Blynk.virtualWrite(V3, "-");
            Blynk.virtualWrite(V4, "-");
            Blynk.virtualWrite(V5, "-");
            Blynk.virtualWrite(V6, "-");
        } else {
            Blynk.virtualWrite(V1, period);
        }
    }
    lastInterruptTime = interruptTime;
}

void printHeaders(void){
    Blynk.virtualWrite(V7, "clr", "RTC Time Uptime   Humid Temp Light DAC Alm\n");
    system("clear");
    delay(150);
    printf("RTC Time\tSys Timer\tHumidity\tTemp\tLight\tDAC Out\tAlarm\n");
    Blynk.virtualWrite(V1, period);
    Blynk.virtualWrite(V2, 0);
}

void outputData(time_t N, float H, int T, int L, float V){
    const char *A = "";
    if (digitalRead(ALARM)){
        A = "*";
        Blynk.virtualWrite(V2, 1);
    }
    if (!paused){
        time_t S = (time_t)difftime(N, sysTime);
        struct tm * t = localtime(&N);

        char s[9];
        strftime(s, 9, "%X", t);
        printf("%s\t", s);
        Blynk.virtualWrite(V7, s, " ");

        t = gmtime(&S);
        strftime(s, 9, "%X", t);
        printf("%s\t%1.2f V\t\t%d C\t%d\t%1.2f V\t%s\n", s, H, T, L, V, A);

        char p[30];
        if (L > 999){
            sprintf(p, "%s %1.2fV %dC  %d  %1.2fV %s\n", s, H, T, L, V, A);
        } else {
            sprintf(p, "%s %1.2fV %dC  %d   %1.2fV %s\n", s, H, T, L, V, A);
        }

        printing = true;
        Blynk.virtualWrite(V7, p);
        Blynk.virtualWrite(V0, s);
        Blynk.virtualWrite(V3, H);
        Blynk.virtualWrite(V4, T);
        Blynk.virtualWrite(V5, L);
        Blynk.virtualWrite(V6, V);
        printing = false;
    }
}
