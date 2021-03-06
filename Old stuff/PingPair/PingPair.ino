/*
** Copyright Martin Di Rollo / PlayMe - 2014
**
** //// USB to RF24 /////
** This program aims to wirelessly transmit commands from the master PlayMeLightSetup
** Processing sketch in order to control various custom devices
** 
** The animations for custom devices are set using these commands:
**
** <number>c : Select device (channel)
** <number>v : Set animation to new value
**
** These can be combined. For example:
** 100c355w : Set channel 100 to value 255.
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#define NUMBER_OF_DEVICES 7
#define DEVICE_NUMBER 1
#define RF_CHANNEL 96
#define DEBUG 1


// Hardware configuration
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);


// Topology
// Radio pipe addresses for 7 nodes to communicate, plus one for the master channel
const uint64_t masterPipe = 0xF0F0F0F011LL;

const uint64_t pipes[NUMBER_OF_DEVICES] = { 0xF0F0F0F022LL, 
                                            0xF0F0F0F033LL, 
                                            0xF0F0F0F044LL, 
                                            0xF0F0F0F055LL, 
                                            0xF0F0F0F066LL, 
                                            0xF0F0F0F077LL,
                                            0xF0F0F0F088LL };

// Table containing the animations to be played, for devices whose number range from 0 to 6
int currentAnimations[NUMBER_OF_DEVICES] = {0,0,0,0,0,0,0};

//Temporary variables used to receive data from Processing
int value = 0;
int channel = 0;

void setup(void)
{

  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();

  printf("\n\rUSB Serial to nRF24 \n\r");
  
  printf("SerialToCustomDevice ready\n\r");
  printf("\n\r");
  printf("Syntax to use with Processing sketch:\n\r");
  printf("  123c : use device # 123\n");
  printf("  45w  : set current device to animation 45\n\r");
  printf("\n\r");


  // Setup and configure rf radio
  radio.begin();

  // Set the delay between retries & the number of retries to 0
  radio.setRetries(0,0);

  // Optionally, reduce the payload size.  seems to improve reliability
  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)


  //radio.openWritingPipe(pipes[0]);
  //radio.openReadingPipe(1,masterPipe);



  // Stop listening : this sketch only aims to push data as fast as possible
  radio.stopListening();
  
  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

void loop(void)
{

    for (int i=0; i<NUMBER_OF_DEVICES; i++) {
      //If serial data is available, this loop will be preempted
      //Go directly to the serial processing step
      if (Serial.available()) {
        break;
      }
      else {
        setAnimation(i);
      }
    }
  
    // Check if data coming from Processing is available
    int c;
    if (Serial.available()) {
      c = Serial.read();
      if ((c>='0') && (c<='9')) {
        value = 10*value + c - '0';
      } else {
        if (c=='c') channel = value;
        else if (c=='w') {
          currentAnimations[channel] = value;
          //Save a slight bit of time by sending the command for this channel right away
          //This allows the system to be slightly more reactive
          setAnimation(channel);  
          //#ifdef DEBUG
            //printf("Received command for channel %d : animation %d\n", channel, value);
            //printf("Current animation table : %d, %d, %d, %d, %d, %d, %d\n", currentAnimations[0], currentAnimations[1],currentAnimations[2],currentAnimations[3],currentAnimations[4],currentAnimations[5],currentAnimations[6]);
          //#endif
        }
        value = 0;
      }
    }  



    // Take the time, and send it.  This will block until complete
//    printf("Now sending %d...",currentAnimations[0]);
//    bool ok = radio.write( &currentAnimations[0], sizeof(int) );


    
}

void setAnimation(int deviceNumber) {
  radio.openWritingPipe(pipes[deviceNumber]);
  radio.write( &currentAnimations[deviceNumber], sizeof(int) );
}
