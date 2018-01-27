/* MPR121 capacitive touch testing sketch. 

Connect MPR121 to a NodeMCU  
For more information on MPR121 see https://learn.sparkfun.com/tutorials/mpr121-hookup-guide?_ga=2.171784352.1236984808.1515404446-977687667.1513072901 

An OLED display is present for fast feedback via u8g2.

Variables: 
 * SERIALTESTOUTPUT : show debug info in serial at 115200
 * SHOWonU8g2: show info on the OLED display
 * connectedelectrodes: how many electrodes are used of the MPR121 ?
*/

const bool SERIALTESTOUTPUT = false;
const bool SHOWonU8g2 = true;
const int connectedelectrodes = 4; // how many touch electrodes present?

bool changedstate = false;

// OLED display with U8g2 lib
#include <U8g2lib.h>

// MPR121 constants
#include "mpr121.h"

// controlled with I2C:
#include <Wire.h>

// MPR121 IRQ pin
const int irqpin = A0;  // Analog pin 0
boolean touchStates[12]; //to keep track of the previous touch states
boolean firsttouchStates[12]; //to keep track if button was just touched
boolean releasedtouchStates[12]; //to keep track if button was just released

// setup the OLED display of 128x32
// I2C on D2 = SDA and D1 =SCL 
int pSDA = D2;
int pSCL = D1;
// Use U8X8_PIN_NONE if the reset pin is not connected
#define REPORTING_PERIOD_MS     500

U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, pSCL, pSDA, U8X8_PIN_NONE);
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, pSDA, pSCL);

bool initialized = false;
byte beat = 0;

void setup(void) {
  if (SERIALTESTOUTPUT) {
    Serial.begin(115200);   // We'll send debugging information via the Serial monitor
    Serial.print("Serial startedpin! ");
  }

  //set up OLED and MPR121 over I2C bus 
  Wire.begin(pSDA, pSCL);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  mpr121_setup();
  if (SHOWonU8g2) {
    u8g2.begin();
    initial_display();
  }
  
  for (int i=0; i < 12; i++){
    firsttouchStates[i] = 0;
    releasedtouchStates[i] = 0;
    touchStates[i] = 0;
  }
  if (SERIALTESTOUTPUT) {
    Serial.print("Ended Setup Configuration! ");
  }
}
 
void loop(void) {
  changedstate = false;
  readTouchInputs();
  if (changedstate) {
    if (SHOWonU8g2) {
      show_touch();
    }
  }
}


// initialize display 
void initial_display()
{
  if (not initialized)
  {
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
    
    u8g2.drawStr(0,10,"Capacitive");
    u8g2.drawStr(40,25,"Touch!");  // write something to the internal memory
    u8g2.sendBuffer();          // transfer internal memory to the display
    delay(4000);  
  
    //u8g2.print("Benny");
    //u8g2.setCursor(0, 30);
    //u8g2.print("Cristina");
    //u8g2.sendBuffer();
    initialized = true;
  }
}

void show_touch() {
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
  u8g2.drawStr(0,10,"Parts touched:");  // write something to the internal memory
  //u8g2.setCursor(0, 32);
  //u8g2.print(ADC);
  int xpos = 10;
  for (int i=0; i<connectedelectrodes; i++) {
    if (touchStates[i] == 0) {
      u8g2.drawStr(xpos, 25, "0");
    } else {
      u8g2.drawStr(xpos, 25, "1");
    }
    xpos += 15;
  }
  u8g2.sendBuffer();          // transfer internal memory to the display
}

/*
 * Read what button is touched and set 
 */
void readTouchInputs(){
  if (checkInterrupt()){
    changedstate = true;
    //read the touch state from the MPR121
    Wire.requestFrom(0x5A,2); 
    
    byte LSB = Wire.read();
    byte MSB = Wire.read();
    
    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    
    for (int i=0; i < 12; i++){  // Check what electrodes were pressed
      
      firsttouchStates[i] = 0;
      releasedtouchStates[i] = 0;
      if(touched & (1<<i)){
      
        if(touchStates[i] == 0){
          //pin i was just touched
          if (SERIALTESTOUTPUT ) {
            Serial.print("pin ");
            Serial.print(i);
            Serial.println(" was just touched");
          }
          firsttouchStates[i] = 1;
        } else if(touchStates[i] == 1){
          //pin i is still being touched
        }
        touchStates[i] = 1;      
      }else{
        if(touchStates[i] == 1){
          if (SERIALTESTOUTPUT ) {
            Serial.print("pin ");
            Serial.print(i);
            Serial.println(" is no longer being touched");
          }
          releasedtouchStates[i] = 1;
          //pin i is no longer being touched
       }
        
        touchStates[i] = 0;
      }
    }
  }
}


/* 
 *  Setup of the MPR121 sensor, setting operation and thresholds.
 */
void mpr121_setup(void){

  set_register(0x5A, ELE_CFG, 0x00); 
  
  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);
  
  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);
 
  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);
  
  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);
  
  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);
  
  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);
  
  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);
  
  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);
  
  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);
  
  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);
  
  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);
  
  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);
  
  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);
  
  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);
  
  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables all 12 Electrodes
  
  
  // Section F
  // Enable Auto Config and auto Reconfig
  /*set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);*/  // Target = 0.9*USL = 0xB5 @3.3V
  
  set_register(0x5A, ELE_CFG, 0x0C);
  
}

/*
 * Check the MPR121. When an action happened, returns LOW, otherwise HIGH
 */
boolean checkInterrupt(void){
  return (analogRead(irqpin) < 600);
}

// set values over I2C
void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

