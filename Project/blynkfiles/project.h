#ifndef PROJECT_H
#define PROJECT_H

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <pthread.h>
#include <mcp3004.h>
#include <time.h>

//Define pins
const int BTNS[] = {23, 24, 5, 6};
#define ALARM 25
#define HUMIDITY 100
#define LDR 102
#define THERMISTOR 107

//SPI Settings
#define ADC_SPI 0
#define DAC_SPI 1
#define SPI_SPEED 100000

//Function definitions
void initPeriphs(void);
void *dataThread(void *threadargs);
void resetAlarm(void);
void fullReset(void);
void samplingPeriod(void);
void toggleMonitoring(void);
void printHeaders(void);
void outputData(time_t N, float H, int T, int L, float V);

#endif
