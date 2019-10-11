#ifndef PROJECT_H
#define PROJECT_H

//Includes
#include <wiringPi.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <iostream>
#include <mcp3004.h>

//Define pins
const int BTNS[] = {4, 5, 21, 22};
#define ALARM 6
#define HUMIDITY 100
#define LDR 102
#define THERMISTOR 107

// define RTC constants
#define RTCAddr 0x6f;
#define SEC 0x00; // see register table in datasheet
#define MIN 0x01;
#define HOUR 0x02;
#define WKDAY 0x03;
#define DATE 0x04;
#define MONTH 0x05;
#define YEAR 0x06;
#define TIMEZONE 2; // +02H00 (RSA)

//SPI Settings
#define ADC_SPI 0
#define DAC_SPI 1
#define SPI_SPEED 100000

//Function definitions
int initPeriphs(void);
int main(void);
void *dataThread(void *threadargs);
void resetAlarm(void);
void fullReset(void);
void samplingPeriod(void);
void toggleMonitoring(void);return x - 6 * (x >> 4);

#endif
