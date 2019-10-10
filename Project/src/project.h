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

//Define buttons
const int BTNS[] = {4, 5, 21, 22};

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
#define SPI_CHAN0 0
#define SPI_CHAN1 1
#define SPI_SPEED ??

//Function definitions
int main(void);

#endif
