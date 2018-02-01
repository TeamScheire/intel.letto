// adapted from 
// example from: http://www.instructables.com/id/Simplest-ESP8266-Local-Time-Internet-Clock-With-OL/
//
// Libraries needed:
//  Time.h & TimeLib.h:  https://github.com/PaulStoffregen/Time
//  Timezone.h: (via install library) https://github.com/JChristensen/Timezone
//  NTPClient.h: (via install library) https://github.com/arduino-libraries/NTPClient
//  ESP8266WiFi.h & WifiUDP.h: https://github.com/ekstrand/ESP8266wifi
//  U8g2: for OLED display via install library
// 

// alarm
uint8_t alarm_hour = 21;
uint8_t alarm_min = 0;

unsigned long NTPUpdateInterval = 60000 ;
 
unsigned long NTPstartTijd;
unsigned long NTPlastUpdate;
time_t utc;
unsigned long huidigeTijd;
unsigned long wifireconnectTime = 0;

#include <ESP8266WiFi.h>
//#include <WifiUDP.h>
#include <WiFiUdp.h>
//#include <String.h>
#include <Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

// OLED display with U8g2 lib
#include <U8g2lib.h>

// setup the OLED display of 128x32
// I2C on D2 = SDA and D1 =SCL 
int pSDA = D2;
int pSCL = D1;
// Use U8X8_PIN_NONE if the reset pin is not connected
#define REPORTING_PERIOD_MS     500

bool use_static_IP = false;         //use a static IP address
uint8_t static_IP[4] = {192, 168, 1, 42}; 
uint8_t static_gateway_IP[4] = {192, 168, 1, 1};// if you want to use static IP make sure you know what the gateway IP is, here 192.168.1.1

// wifi data
//const char* ssid = "*****";   // insert your own ssid 
//const char* password = "********"; // and password
IPAddress ip;
bool obtainedwifi = false;

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
bool UK_DATE = false;

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Create a display object
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, pSCL, pSDA, U8X8_PIN_NONE);
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, pSDA, pSCL);

bool initialized = false;

String date;
String t;
//const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
//const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
//const char * ampm[] = {"AM", "PM"} ;
const char * days[] = {"Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za"} ;
const char * months[] = {"Jan", "Feb", "Maa", "Apr", "Mei", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"VM", "NM"} ;


// set up the wifi connection. If wait, we wait for the wifi to come up
// indefenitely
void setupWiFi(bool wait)
{
  int nrpoints;
  // Connect to WiFi network
  if (SERIALTESTOUTPUT) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }

  //we only try to set up wifi once every 2 minutes !
  if (wait || (millis() - wifireconnectTime > 2*60000L)) { 
    //clean up any old config that are still present
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    if (use_static_IP) {
      // we force a static IP for this station and set 
      IPAddress  stationIP(static_IP[0], static_IP[1], static_IP[2], static_IP[3]);
      IPAddress gateway(static_gateway_IP[0], static_gateway_IP[1], static_gateway_IP[2], static_gateway_IP[3]); // set gateway to match your network
      IPAddress subnet(255, 255, 255, 0); // set subnet mask to match yourelse
      WiFi.mode(WIFI_STA);
      delay(100);
      
      //set a static IP address
      WiFi.config(stationIP, gateway, subnet);
    } else {
      WiFi.mode(WIFI_AP);
    }
    WiFi.begin(ssid, password);
    wifireconnectTime = millis();
  }

  if (wait) {
    while (WiFi.status() != WL_CONNECTED) {
      nrpoints += 1;
      delay(500);
      if (SERIALTESTOUTPUT) {
        Serial.print(".");
      }
      if (nrpoints > 50) {
        nrpoints = 0;
         if (SERIALTESTOUTPUT) Serial.println(" ");
      }
    }
    
    if (SERIALTESTOUTPUT) {
      Serial.println("");
      Serial.println("WiFi connected");
    }
  } 
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // most ESP-01's use 115200 but this could vary

  //set up OLED and MPR121 over I2C bus 
  Wire.begin(pSDA, pSCL);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  u8g2.begin();
  initial_display(false);
  // Connect to wifi
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  setupWiFi(true);
  
  timeClient.begin();   // Start the NTP UDP client
  Serial.println("");
  Serial.print("Connected to WiFi at ");
  ip = WiFi.localIP();
  Serial.print(ip);
  Serial.println("");
  Serial.print("Wifi end of IP: ");
  Serial.print(ip[3]);
  Serial.println("");
  initial_display(true);
  obtainedwifi = true;
  delay(1000);

  obtainDateTime();
  NTPstartTijd = millis();
  huidigeTijd = NTPstartTijd;
  
}

void loop() {
  huidigeTijd = millis();
  if (huidigeTijd - NTPstartTijd > NTPUpdateInterval) {
    //reupdate NTP data
    obtainDateTime();
    //reset NTPstart time
    NTPstartTijd = millis();
  }
  displayDateTime();
}

void write_wifi(bool wifi){
  u8g2.setFont(u8g2_font_pixelle_micro_tr);
  if (wifi) {
    u8g2.drawStr(100,32,"Wifi");
    char buf[4];
    snprintf (buf, 4, "%d", ip[3]);
    u8g2.drawStr(113, 32, buf);
    initialized = true;
  } else {
    u8g2.drawStr(100,32,"No Wifi");
  }
}

// initialize display 
void initial_display(bool wifi)
{
  if (not initialized)
  {
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
    
    //u8g2.drawStr(0,10,"Time");
    u8g2.drawStr(15,25,"Intel.Letto!");  // write something to the internal memory
    write_wifi(wifi);
    u8g2.sendBuffer();          // transfer internal memory to the display
    delay(1000);  
  
  }
}

void obtainDateTime() {
  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  {   
    obtainedwifi = true;
    date = "";  // clear the variables
    t = "";
    
    // update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    NTPlastUpdate = millis();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local;
    utc = epochTime;
    setTime(utc);
    
    // Then convert the UTC UNIX timestamp to local time
    // normal time from zon 2 march to sun 2 nov 
    TimeChangeRule euBRU = {"BRU", Second, Sun, Mar, 2, +60};  //normal time UTC + 1 hours - change this as needed
    TimeChangeRule euUCT = {"UCT", First, Sun, Nov, 2, 0};     //daylight saving time summer: UTC - change this as needed
    Timezone euBrussel(euBRU, euUCT);
    local = euBrussel.toLocal(utc);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += day(local);
    date += " ";
    date += months[month(local)-1];
    date += ", ";
    date += year(local);

    if (UK_DATE) {
      // format the time to 12-hour format with AM/PM and no seconds
      t += hourFormat12(local);
    } else {
      // normal format hour
      t += hour(local);
    }
      t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    if (UK_DATE) {
      t += ampm[isPM(local)];
      t += " ";
    }

    // Display the date and time
    Serial.println("");
    Serial.print("Local date: ");
    Serial.print(date);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);
  } else {
    //reconnect
    obtainedwifi = false;
    setupWiFi(false);
  }
}

// display the time on the OLED
void displayDateTime() {
    time_t local;
    date = "";  // clear the variables
    t = "";
  
    // print the date and time on the OLED
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
    write_wifi(obtainedwifi);

    time_t timenow = now();
    // Then convert the UTC UNIX timestamp to local time

    // Then convert the UTC UNIX timestamp to local time
    // normal time from zon 2 march to sun 2 nov 
    TimeChangeRule euBRU = {"BRU", Second, Sun, Mar, 2, +60};  //normal time UTC + 1 hours - change this as needed
    TimeChangeRule euUCT = {"UCT", First, Sun, Nov, 2, 0};     //daylight saving time summer: UTC - change this as needed
    Timezone euBrussel(euBRU, euUCT);
    local = euBrussel.toLocal(timenow);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += " ";
    date += day(local);
    date += " ";
    date += months[month(local)-1];
    date += " ";
    date += year(local);
    date += " ";

    if (UK_DATE) {
      // format the time to 12-hour format with AM/PM and no seconds
      t += hourFormat12(local);
    } else {
      // normal format hour
      t += hour(local);
    }
    t += ":";
    if(minute(local) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(local);
    t += " ";
    if (UK_DATE) {
      t += ampm[isPM(local)];
     t += " ";
    }

    char datebuf[date.length() + 1];
    date.toCharArray(datebuf, date.length());
    char timebuf[t.length() + 1];
    t.toCharArray(timebuf, t.length());

    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); //9px font https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2.drawStr(0,10,datebuf);
    u8g2.setFont(u8g2_font_profont17_tf); //11px font https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2.drawStr(32,25,timebuf);
    
    u8g2.sendBuffer();          // transfer internal memory to the display
}

