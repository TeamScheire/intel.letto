/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   DESCRIPTION

   Bed occupancy sensor using MPR121.  Requires BareConductive MPR121 Library (https://github.com/BareConductive/mpr121)
   This sketch will detect both sides of a bed (two people) but it can easily be modified to detect one person.
   There is a customizable 5 second detection threshold.  It will check every second to see if a the state has changed
   and then trigger if all 5 states are the same.  So, if you shift around in your sleep it won't keep triggering.  You can also customize
   the varianceValue to make it more or less sensitive.  The lower the number the more senstive it will be (don't set
   it below 4).  I have found a value of 15 to work well even under a couple of blankets.

   Check out the How To video here: https://youtu.be/KofnH0reWCQ

   REVISION HISTORY
   3/30/16 - PeteWill - Version 1.0
*/


#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>
#include <MPR121.h>
#include <Wire.h>

#define SN "Bed Occupancy"
#define SV "1.0"

#define NODE_ID 34 //Change to AUTO if you want it to automatically assign it for you

#define RIGHT_BED_CHILD_ID 0
#define LEFT_BED_CHILD_ID 1

#define OCC_LED_RIGHT  7 // Arduino Digital I/O pin for the LED
#define OCC_LED_LEFT  4  // Arduino Digital I/O pin for the LED

MySensor gw;

unsigned long OccPreviousMillis;

//Store the state of current bed occupancy
uint8_t rightOccupied;
uint8_t leftOccupied;

//Store the state of previous bed occupancy.  When current and previous differ, send update to gateway.
uint8_t rightOccPrev;
uint8_t leftOccPrev;

//Variables used to test if occupancy is true for the duration of occDelay * 5
uint8_t occStateRight[] = {0, 0, 0, 0, 0};
uint8_t occStateLeft[] = {0, 0, 0, 0, 0};

uint16_t occDelay = 1000; // delay (in milliseconds) between reading bed occupancy sensors
uint8_t varianceValue = 15; //if the difference between baseline data and filtered data is greater than this number it will flag occupied.
int filtValPrev[9]; //Used to keep track of the previous filtered values which will determine if the sensor is triggered
uint8_t occupied[9]; //Used to keep track of initial occupancy (tracked by the difference between baseline and filtered data). Make sure to update this array with the number of occupancy sensors used
uint8_t captured[9]; //Used to determine if the baseline and filtered data was already captured after an inital occupancy check. Make sure to update this array with the number of occupancy sensors used
int filterCapture[9]; //Used to keep track of what the filtered values were when the sensor was first triggered

MyMessage msgRightOcc(RIGHT_BED_CHILD_ID, V_TRIPPED);
MyMessage msgLeftOcc(LEFT_BED_CHILD_ID, V_TRIPPED);

#define DEBUG_ON   // comment out to supress serial monitor output

#ifdef DEBUG_ON
#define DEBUG_PRINT(x)   Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define SERIAL_START(x)  Serial.begin(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define SERIAL_START(x)
#endif

void setup()
{
  gw.begin(NULL, NODE_ID);
  gw.sendSketchInfo(SN, SV);

  // 0x5A is the default MPR121 I2C address
  if (!MPR121.begin(0x5A)) {
    DEBUG_PRINTLN(F("error setting up MPR121"));
    switch (MPR121.getError()) {
      case NO_ERROR:
        DEBUG_PRINTLN(F("no error"));
        break;
      case ADDRESS_UNKNOWN:
        DEBUG_PRINTLN(F("incorrect address"));
        break;
      case READBACK_FAIL:
        DEBUG_PRINTLN(F("readback failure"));
        break;
      case OVERCURRENT_FLAG:
        DEBUG_PRINTLN(F("overcurrent on REXT pin"));
        break;
      case OUT_OF_RANGE:
        DEBUG_PRINTLN(F("electrode out of range"));
        break;
      case NOT_INITED:
        DEBUG_PRINTLN(F("not initialised"));
        break;
      default:
        DEBUG_PRINTLN(F("unknown error"));
        break;
    }

  }

  gw.present(RIGHT_BED_CHILD_ID, S_MOTION);
  gw.present(LEFT_BED_CHILD_ID, S_MOTION);

  //Set LED pins to Output
  pinMode(OCC_LED_RIGHT, OUTPUT);
  pinMode(OCC_LED_LEFT, OUTPUT);

  //Set initial state of LEDs to off
  digitalWrite(OCC_LED_RIGHT, LOW);
  digitalWrite(OCC_LED_LEFT, LOW);

  //Set occupancy to 0 when node is first turned on
  msgRightOcc.set(0);
  msgLeftOcc.set(0);

}

//  Check if digital input has changed and send in new value
void loop()
{
  unsigned long OccCurrentMillis = millis();
  if (OccCurrentMillis - OccPreviousMillis > occDelay) {
    OccPreviousMillis = OccCurrentMillis;
    MPR121.updateFilteredData();

    //First Bed Occupancy Sensor
    rightOccupied = 0;
    // Read bed occ state
    for (uint8_t i = 0; i < 4; i++) {    // adjust for the number of sensors used
      int filtVal = MPR121.getFilteredData(i);

      DEBUG_PRINT(F("Value of sensor "));
      DEBUG_PRINT(i);
      DEBUG_PRINT(F(": "));
      DEBUG_PRINT(filtVal);

      //Check if the state has changed
      if (filtValPrev[i] - filtVal > varianceValue) {
        //Someone is in the bed
        if (occupied[i] == 0) {
          filterCapture[i] = filtValPrev[i];
          occupied[i] = 1;
        }
      }
      else {
        if (filtVal < filterCapture[i] - (varianceValue * 2)) {
          //Still occupied
          occupied[i] = 1;
        }
        else {
          //The filtered value has gone back to the original value.  We need to 0 out the tracking arrays.
          occupied[i] = 0;
        }
      }
      //Add occupied counter to rightOccupied var to keep track of any tripped sensors
      rightOccupied += occupied[i];
      filtValPrev[i] = filtVal;
    }

    //Assign previous readings to incremented variables
    for (int i = 0; i < 4; i++) {
      occStateRight[i] = occStateRight[i + 1];
    }

    //If more than one sensor was tripped, reduce rightOccupied var to 1 so it can be used in "tripped" logic
    occStateRight[4] = rightOccupied >= 1 ? 1 : 0;

    if ((occStateRight[0] == occStateRight[1]) && (occStateRight[1] == occStateRight[2]) && (occStateRight[2] == occStateRight[3]) && (occStateRight[3] == occStateRight[4])) {
      DEBUG_PRINTLN(F("5 States are the same"));
      if (occStateRight[4] != rightOccPrev) {
        DEBUG_PRINTLN(F("Values are different.  Send to gateway"));
        // Send in the new value
        gw.send(msgRightOcc.set(occStateRight[4]));  // Send tripped value to gw
        rightOccPrev = occStateRight[4];
        if (rightOccPrev == 1) {
          digitalWrite(OCC_LED_RIGHT, HIGH);
        }
        else {
          digitalWrite(OCC_LED_RIGHT, LOW);
        }
      }
    }

    //Second Bed Occupancy Sensor.  If only using one sensor delete or comment out code below.
    leftOccupied = 0;
    // Read bed occ state
    for (uint8_t i = 4; i < 8; i++) {    // adjust for the number of sensors used
      int filtVal = MPR121.getFilteredData(i);

      DEBUG_PRINT(F("Value of sensor "));
      DEBUG_PRINT(i);
      DEBUG_PRINT(F(": "));
      DEBUG_PRINT(filtVal);

      if (filtValPrev[i] - filtVal > varianceValue) {
        if (occupied[i] == 0) {
          filterCapture[i] = filtValPrev[i];
          occupied[i] = 1;
        }
      }
      else {
        if (filtVal < filterCapture[i] - (varianceValue * 2)) {
          //Still occupied
          occupied[i] = 1;
        }
        else {
          //The baseline has gone back to the normal value or the sensor is still not triggered.  We need to 0 out the tracking arrays.
          occupied[i] = 0;
        }
      }
      //Add add occupied counter to leftOccupied var to keep track of any tripped sensors
      leftOccupied += occupied[i];
      filtValPrev[i] = filtVal;
    }

    //Assign previous readings to incremented variables
    for (int i = 0; i < 4; i++) {
      occStateLeft[i] = occStateLeft[i + 1];
    }

    //If more than one sensor was tripped, reduce leftOccupied var to 1 so it can be used in "tripped" logic
    occStateLeft[4] = leftOccupied >= 1 ? 1 : 0;

    if ((occStateLeft[0] == occStateLeft[1]) && (occStateLeft[1] == occStateLeft[2]) && (occStateLeft[2] == occStateLeft[3]) && (occStateLeft[3] == occStateLeft[4])) {
      DEBUG_PRINTLN(F("5 States are the same"));
      if (occStateLeft[4] != leftOccPrev) {
        DEBUG_PRINTLN(F("Values are different.  Send to gateway"));
        // Send in the new value
        gw.send(msgLeftOcc.set(occStateLeft[4]));  // Send tripped value to gw
        leftOccPrev = occStateLeft[4];
        if (leftOccPrev == 1) {
          digitalWrite(OCC_LED_LEFT, HIGH);
        }
        else {
          digitalWrite(OCC_LED_LEFT, LOW);
        }
      }
    }
  }
}

