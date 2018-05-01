// Ingegno 02/2017
// control massage mattress
// Wiring:
// 12V input to Arduino
// 5V out to drive NodeMCU
// MASTER: This is code on the NodeMCUArduino, which is the master device.
//         Based on mqtt request we set massage on the device. 

/*  START USER SETTABLE OPTIONS */
bool use_static_IP = false;         //use a static IP address
uint8_t static_IP[4] = {192, 168, 1, 42}; 
uint8_t static_gateway_IP[4] = {192, 168, 1, 1};// if you want to use static IP make sure you know what the gateway IP is, here 192.168.1.1

// wifi data
// write here your wifi credentials 
//const char* ssid = "*****";   // insert your own ssid 
const char* password = "********"; // and password
const char* ssid = "intelletto";   // insert your own ssid 

//mqtt server/broker 
//const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "raspberrypi.local";
const char* mqtt_server = "192.168.1.29";  //eth0 address of the raspberry pi - Ingegno
uint8_t mqtt_server_IP[4] = {192, 168, 1, 29};
//const char* mqtt_server = "192.168.0.213";  //eth0 address of the raspberry pi - Big Fix
//uint8_t mqtt_server_IP[4] = {192, 168, 0, 213};

/*  END USER SETTABLE OPTIONS */

// I2C on D2 = SDA and D3 =SCL 
int pSDA = D2;
int pSCL = D3;

//massage mattress controller
const byte SLAVE_ADDRESS = 9;  // Massage control slave device

enum MASSAGE {MS_NONE, 
              MS_NECKON, MS_NECKOFF, MS_NECK, MS_NECKWEAK,
              MS_BREASTON, MS_BREASTOFF, MS_BREAST, MS_BREASTWEAK, 
              MS_BELLYON, MS_BELLYOFF, MS_BELLY, MS_BELLYWEAK, 
              MS_HIPON, MS_HIPOFF, MS_HIP, MS_HIPWEAK, 
              MS_F1};
               
MASSAGE massagescenario = MS_NONE;

//includes
#include <Wire.h>

// debug info
#define SERIALTESTOUTPUT true

void send2Slave(int msg) {
  Wire.beginTransmission(SLAVE_ADDRESS); // transmit to device SLAVE_ADDRESS
  Wire.write(msg);           // sends one byte
  Wire.endTransmission();    // stop transmitting
}

//wifi and timing lib
#include "wifilib.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  // put your setup code here, to run once:
  if (SERIALTESTOUTPUT) Serial.begin(115200); // most ESP-01's use 115200 but this could vary

  //set up I2C bus 
  Wire.begin(pSDA, pSCL);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  while (WiFi.status() != WL_CONNECTED) {
   setupWiFi(true); // Connect to local Wifi
  }
  //set randomseed
  randomSeed(micros());

  // mqtt client start
  // Here we subscribe to MQTT topic intellettoMassage
  setupMQTTClient();

  if (SERIALTESTOUTPUT) {
    Serial.println("");
    Serial.print("Connected to WiFi at ");
  }
  ip = WiFi.localIP();
  if (SERIALTESTOUTPUT) {
    Serial.print(ip);
    Serial.println("");
    Serial.print("Wifi end of IP: ");
    Serial.print(ip[3]);
    Serial.println("");
  }
  obtainedwifi = true;
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  //handle MQTT messages
  handleMQTTClient();

  // if disconnected, reconnect to wifi:
  if ( WiFi.status() != WL_CONNECTED) {
      delay(1);
      setupWiFi(false);
  }
  
  if (SERIALTESTOUTPUT) {
    //delay(200);
  }

  //if a program needs to run, we now handle it
  if (current_program == 1) {
    run_program_1();
  }
}

void run_program_1() {
  // program of 4 blocks, 10 sec hip, 10 sec tummy, 10 sec breast, 10 sec neck
  unsigned long current_time = millis();
  unsigned long current_block = (current_time - start_program_time) % 40;
  if (current_block < 10) {
    send2Slave(MS_HIPWEAK);
    send2Slave(MS_BELLYOFF);
    send2Slave(MS_BREASTOFF);
    send2Slave(MS_NECKOFF);
  } else if (current_block < 20) {
    send2Slave(MS_HIPOFF);
    send2Slave(MS_BELLY);
    send2Slave(MS_BREASTOFF);
    send2Slave(MS_NECKOFF);
  } else if (current_block < 20) {
    send2Slave(MS_HIPOFF);
    send2Slave(MS_BELLYOFF);
    send2Slave(MS_BREAST);
    send2Slave(MS_NECKOFF);
  } else {
    send2Slave(MS_HIPOFF);
    send2Slave(MS_BELLYOFF);
    send2Slave(MS_BREASTOFF);
    send2Slave(MS_NECKWEAK);
  }
}


