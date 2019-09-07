/* Prac4.cpp
 *
 * Originally written by Stefan Schröder and Dillion Heald
 * Adapted for EEE3096S 2019 by Keegan Crankshaw
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Prac4.h"

using namespace std;

bool playing = true;       // Set false when paused
bool stopped = false;      // If set to true, program closes
unsigned char buffer[2][BUFFER_SIZE][2];
int buffer_location = 0;
bool bufferReading = 0;   // Using this to switch between column 0 and 1 - the first column
bool threadReady = false; // Using this to finish writing the first column at the start of the song, before the column is played
long lastInterruptTime = 0;

// Configure interrupts using debouncing
void play_pause_isr(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
   	playing = !playing;
   	(playing) ? printf("Playing\n") : printf("Paused\n");
    }
    lastInterruptTime = interruptTime;
}

void stop_isr(void){
    // Debounce
    long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 200){
	stopped = !stopped;
	printf("Stopped\n");
    }
    lastInterruptTime = interruptTime;
}

// Setup Function. Called once
int setup_gpio(void){
    // Set up wiring Pi
    wiringPiSetup();

    // Setting up the buttons
    pinMode(PLAY_BUTTON, INPUT);
    pullUpDnControl(PLAY_BUTTON, PUD_UP);
    pinMode(STOP_BUTTON, INPUT);
    pullUpDnControl(STOP_BUTTON, PUD_UP);
    printf("BTNs done\n");

    // Setting up the button interrupts
    if (wiringPiISR(PLAY_BUTTON, INT_EDGE_FALLING, &play_pause_isr) < 0){
	printf("Play Button ISR error\n");
    }
    if (wiringPiISR(STOP_BUTTON, INT_EDGE_FALLING, &stop_isr) < 0){
	printf("Stop Button ISR error\n");
    }

    // Setting up the SPI interface
    wiringPiSPISetup(SPI_CHAN, SPI_SPEED);
    printf("SPI setup done\n");
    return 0;
}

/* Thread that handles writing to SPI
 *
 * Pause writing to SPI if not playing is true (the player is paused).
 * Using the buffer_location variable to check when it needs to switch buffers
 */
void *playThread(void *threadargs){
    // If the thread isn't ready, don't do anything
    while(!threadReady){
        continue;
    }

    // Playing if the stopped flag is false
    while(!stopped){
        // Suspend playing if paused
	while (!playing){
	    continue;
	}

	// Write buffer to DAC using SPI
	wiringPiSPIDataRW(SPI_CHAN, buffer[bufferReading][buffer_location], 2);

	// Check if buffers need to be toggled
	buffer_location++;
	if (buffer_location >= BUFFER_SIZE){
	    buffer_location = 0;
	    bufferReading = !bufferReading;
	}
    }

    // Exit program when stopped
    exit(0);
    pthread_exit(NULL);
}

int main(){
    // Call the setup GPIO function
    if(setup_gpio() == -1){
        return 0;
    }

    /* Initialize thread with parameters
     * Play thread has a 99 priority
     */
    pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;

    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); // Safe to get existing scheduling param
    param.sched_priority = newprio;              // Set the priority; others are unchanged
    pthread_attr_setschedparam (&tattr, &param); // Setting the new scheduling param
    pthread_create(&thread_id, &tattr, playThread, (void *)-1); // With new priority specified

    /* Read from the file, character by character
     * Using bit shifting
     * buffer[bufferWriting][counter][0] is set with the control bits
     * as well as the first few bits of audio.
     * buffer[bufferWriting][counter][1] is set with the last audio bits
     * Check if you pause is set or not when writing to the buffer
     */

    // Open the file
    unsigned char ch;
    FILE *filePointer;
    printf("%s\n", FILENAME);
    filePointer = fopen(FILENAME, "r"); // Read mode

    if (filePointer == NULL) {
        perror("Error while opening the file\n");
        exit(EXIT_FAILURE);
    } else {
	printf("File opened\n");
    }

    int counter = 0, bufferWriting = 0, c = 0;
    unsigned char chararr[150];

    // Have a loop to read from the file
    while((ch = fgetc(filePointer)) != EOF){
	// EOF isnt working on the pi, so manually check for end-of-file
	chararr[c++] = ch;
	if (c == 150){
	    int num = 0;
	    for (int i = 0; i < c; i++){
		if (chararr[i] == 129){
		    num++;
		}
	    }
	    if (num == c){
		stopped = !stopped;
	    }
	    c = 0;
	}

        while(threadReady && bufferWriting==bufferReading && counter==0){
            //waits in here after it has written to a side, and the thread is still reading from the other side
            continue;
        }
        //Set config bits for first 8 bit packet and OR with upper bits
        buffer[bufferWriting][counter][0] = (0b0111 << 4) | (ch >> 6);
        //Set next 8 bit packet
        buffer[bufferWriting][counter][1] = ch << 2;

        counter++;
        if(counter >= BUFFER_SIZE+1){
            if(!threadReady){
                threadReady = true;
            }

            counter = 0;
            bufferWriting = !bufferWriting;
        }
    }

    // Close the file
    fclose(filePointer);

    //Join and exit the playthread
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
}
