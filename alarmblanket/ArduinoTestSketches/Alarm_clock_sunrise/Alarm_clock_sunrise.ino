// Ingegno 01/2017
// Alarm clock via sunrise 
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

//includes

#include <ESP8266WiFi.h>
//#include <WifiUDP.h>
#include <WiFiUdp.h>
//#include <String.h>
#include <Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

//Adafruit_Neopixel usage library
#include "neopattern.h"

// OLED display with U8g2 lib
#include <U8g2lib.h>

// debug info

#define SERIALTESTOUTPUT false

/*  START USER SETTABLE OPTIONS */
// alarm
uint8_t alarm_hour = 8;
uint8_t alarm_min = 0;
bool alarm_sunrise_set = true;  // do alarm or not
uint8_t sunrise_start_min_before = 10;  // minutes to start sunrise before alarm time (max 59)
bool dageraad1on = false;

// wifi data
// write here your wifi credentials 
const char* ssid = "*****";   // insert your own ssid 
const char* password = "********"; // and password
/*  END USER SETTABLE OPTIONS */

/* WIRING OF THE ALARM */

#define pushbutton D7   //pushbutton on D7 (internal pullup)
bool btnpress = false;

int PixelStrook = D3;   //Neopixel IN wire

// I2C on D2 = SDA and D1 =SCL 
int pSDA = D2;
int pSCL = D1;
// Use U8X8_PIN_NONE if the reset pin is not connected
#define REPORTING_PERIOD_MS     500

/* END WIRING OF THE ALARM */

bool alarm_sunrise_on = false;
bool alarm_over_midnight = false;

unsigned long sunrise_millis = sunrise_start_min_before * 60 * 1000;
uint8_t alarm_sunrise_start_min, alarm_sunrise_start_hour, alarm_sunrise_stop_min, alarm_sunrise_stop_hour;
unsigned int alarm_min_hour, alarm_sunrise_start_min_hour, alarm_sunrise_stop_min_hour;
long sec_from_alarm;

unsigned long NTPUpdateInterval = 60000 ;
 
unsigned long NTPstartTijd;
unsigned long NTPlastUpdate;
time_t utc, localtimenow;
unsigned long huidigeTijd;

IPAddress ip;
bool obtainedwifi = false;

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
//#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
#define NTP_ADDRESS  "europe.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
bool UK_DATE = false;

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// setup the OLED display of 128x32
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

//neopixel data
int nrPixels = 72;
int fractie;
int R;
int G;
int B;

void PixelStrookComplete();
 
// Define NeoPatterns for the stick
//  as well as some completion routines
NeoPatterns myNeo_PixelStrook(nrPixels, PixelStrook, NEO_GRB + NEO_KHZ800, &PixelStrookComplete);
 
// myNeo_PixelStrook Completion Callback
void PixelStrookComplete()
{
    // Random color change for next scan
    return; // at the moment, don't change anything
    //myNeo_PixelStrook.Color1 = myNeo_PixelStrook.Wheel(random(255));  //random color
}

void determine_alarm_values(uint8_t alarm_hour, uint8_t alarm_min, uint8_t sunrise_start_min_before) {
  //determine constants for the alarm value
  if (sunrise_start_min_before > alarm_min ) {
    alarm_sunrise_start_hour = alarm_hour - 1;
    if (alarm_sunrise_start_hour < 0) alarm_sunrise_start_hour = 23;
    
    alarm_sunrise_start_min = 60 - (sunrise_start_min_before - alarm_min);
  } else {
    alarm_sunrise_start_hour = alarm_hour;
    alarm_sunrise_start_min = alarm_min - sunrise_start_min_before;
  }
  // no longer than 15 min full sunrise alarm ! 
  alarm_sunrise_stop_hour = alarm_hour;
  alarm_sunrise_stop_min = alarm_min + 15;
  if (alarm_sunrise_stop_min >= 60) {
    alarm_sunrise_stop_min -= 60;
    alarm_sunrise_stop_hour += 1;
  }
  if (alarm_sunrise_stop_hour > 23) alarm_sunrise_stop_hour = 0;
  alarm_min_hour = alarm_hour *  60 + alarm_min;
  alarm_sunrise_start_min_hour = alarm_sunrise_start_hour * 60 + alarm_sunrise_start_min;
  alarm_sunrise_stop_min_hour = alarm_sunrise_stop_hour * 60 + alarm_sunrise_stop_min;
  if (alarm_sunrise_start_min_hour > alarm_sunrise_stop_min_hour) {
    alarm_over_midnight = true;
  } else {
    alarm_over_midnight = false;
  }
}

void setup() {
  if (SERIALTESTOUTPUT) Serial.begin(115200); // most ESP-01's use 115200 but this could vary

  //set the alarm data
  determine_alarm_values(alarm_hour, alarm_min, sunrise_start_min_before);

  //pushbutton on D7
  
  pinMode(pushbutton, INPUT_PULLUP);
  //set up OLED and MPR121 over I2C bus 
  Wire.begin(pSDA, pSCL);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  u8g2.begin();
  initial_display(false);
  // Connect to wifi
  if (SERIALTESTOUTPUT) {
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.print(ssid);
  }
  WiFi.begin(ssid, password);
  int nrpoints = 0;
  while (WiFi.status() != WL_CONNECTED) 
  { nrpoints += 1;
    delay(500);
    if (SERIALTESTOUTPUT) Serial.print(".");
    if (nrpoints > 50) {
      nrpoints = 0;
       if (SERIALTESTOUTPUT) Serial.println(" ");
    }
  }
  timeClient.begin();   // Start the NTP UDP client
  
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
  initial_display(true);
  obtainedwifi = true;
  delay(1000);

  obtainDateTime();
  NTPstartTijd = millis();
  huidigeTijd = NTPstartTijd;
  
  // Initialize the pixelStrips
  myNeo_PixelStrook.begin();
    
  // Kick off a pattern, uncomment one of the following
  // theaterchase
//  myNeo_PixelStrook.TheaterChase(myNeo_PixelStrook.Color(255,255,0), 
//                                 myNeo_PixelStrook.Color(0,0,50), 100);
  //rainbowcycle
//  myNeo_PixelStrook.RainbowCycle(3);
//  myNeo_PixelStrook.Color1 = myNeo_PixelStrook.Color(255,255,0);
  //scanner
  myNeo_PixelStrook.Scanner(myNeo_PixelStrook.Color(255,0,0), 55);
  //do nothing
//  myNeo_PixelStrook.show();

}

void loop() {
  huidigeTijd = millis();
  if (huidigeTijd - NTPstartTijd > NTPUpdateInterval) {
    //reupdate NTP data
    obtainDateTime();
    //reset NTPstart time
    NTPstartTijd = millis();
  }
  
  // Display the date and time
  displayDateTime();
  
  //determine if alarm needed
  if (alarm_sunrise_set) {
    if (SERIALTESTOUTPUT) Serial.println("Alarm set");
    determine_alarm_time();
    sunrise_color();
    
  }
  if (digitalRead(pushbutton)) {
    // button pressed
    btnpress = true;
  } else {
    btnpress = false;    
  }
  if (SERIALTESTOUTPUT) {
    delay(1000);
  } else {
    delay(100);
  }
}

void determine_alarm_time() {
  // localtimenow is the time shown on the display. Should there be alarm? 
  // If between start and end alarm, YES!
  // We keep track of seconds into the alarm
  uint8_t curmin = minute(localtimenow);
  uint8_t curhour = hour(localtimenow);
  uint8_t cursec = second(localtimenow);
  unsigned long curminhour = curhour * 60 + curmin;
  
  sec_from_alarm = 0;
  if (!alarm_over_midnight) {
    if (curminhour >= alarm_sunrise_start_min_hour && curminhour < alarm_sunrise_stop_min_hour) {
      // do alarm !
      alarm_sunrise_on = true;
      //determine seconds from alarm
      sec_from_alarm = curminhour - alarm_min_hour;
    } else {
      alarm_sunrise_on = false;
    }
  } else {
    if (curminhour >= alarm_sunrise_start_min_hour || curminhour < alarm_sunrise_stop_min_hour) {
      // do alarm !
      alarm_sunrise_on = true;
      //determine seconds from alarm
      if (alarm_hour >= 23 ) {
        if (curhour >= 23 ) sec_from_alarm = curminhour - alarm_min_hour;
        else sec_from_alarm = curminhour + (24 * 60 - alarm_min_hour);
      } else {
        if (curhour >= 23 ) sec_from_alarm = -(24 * 60 - curminhour) - alarm_min_hour; //before midnight still
        else sec_from_alarm = curminhour - alarm_min_hour;
      }
    } else {
      alarm_sunrise_on = false;
    }
  }
  
  //before the alarm time eg 7:39 with alarm at 7:40 gives -1 minute
  //after the alarm time eg 7:41 with alarm at 7:40 gives +1 minute
  sec_from_alarm = sec_from_alarm * 60 + cursec;  // convert min to sec
  
  if (SERIALTESTOUTPUT) {
    Serial.println("Computed alarm values");
    Serial.println(alarm_sunrise_start_min_hour);
    Serial.println(alarm_min_hour);
    Serial.println(alarm_sunrise_stop_min_hour);
    Serial.println(curminhour);
    Serial.println(sec_from_alarm);
    Serial.print("Alarm sunrise on? ");Serial.println(alarm_sunrise_on);
  }
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
    ip = WiFi.localIP();
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

    if (SERIALTESTOUTPUT) {
      // Display the date and time
      Serial.println("");
      Serial.print("Local date: ");
      Serial.print(date);
      Serial.println("");
      Serial.print("Local time: ");
      Serial.print(t);
    }
  } else {
    //reconnect
    obtainedwifi = false;
    WiFi.begin(ssid, password);
  }
}

// display the time on the OLED
void displayDateTime() {
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
    localtimenow = euBrussel.toLocal(timenow);

    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(localtimenow)-1];
    date += " ";
    date += day(localtimenow);
    date += " ";
    date += months[month(localtimenow)-1];
    date += " ";
    date += year(localtimenow);
    date += " ";

    if (UK_DATE) {
      // format the time to 12-hour format with AM/PM and no seconds
      t += hourFormat12(localtimenow);
    } else {
      // normal format hour
      t += hour(localtimenow);
    }
    t += ":";
    if(minute(localtimenow) < 10)  // add a zero if minute is under 10
      t += "0";
    t += minute(localtimenow);
    t += " ";
    if (UK_DATE) {
      t += ampm[isPM(localtimenow)];
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

void sunrise_color() {
  if (alarm_sunrise_on) {
    if (sec_from_alarm > 0) {
      if (dageraad1on) {
        // you should be awake, full light
        R = dageraad1[dageraad1len-1] [0];
        G = dageraad1[dageraad1len-1] [1];
        B = dageraad1[dageraad1len-1] [2];
      } else {
        // you should be awake, full light
        R = dageraad2[dageraad2len-1] [0];
        G = dageraad2[dageraad2len-1] [1];
        B = dageraad2[dageraad2len-1] [2];
      }
    } else {
      if (dageraad1on) {
        fractie = (dageraad1len-1) -(-sec_from_alarm *dageraad1len*1.0/ (sunrise_start_min_before * 60));
        /* "leds van een bepaalde kleur" */
        //Serial.println(fractie);
        R = dageraad1[fractie] [0];
        G = dageraad1[fractie] [1];
        B = dageraad1[fractie] [2];
      } else {
        fractie = (dageraad2len-1) -(-sec_from_alarm *dageraad2len*1.0/ (sunrise_start_min_before * 60));
        /* "leds van een bepaalde kleur" */
        //Serial.println(fractie);
        R = dageraad2[fractie] [0];
        G = dageraad2[fractie] [1];
        B = dageraad2[fractie] [2];
      }
    }
    myNeo_PixelStrook.ColorSet(myNeo_PixelStrook.Color(R,G,B));
    
    if (SERIALTESTOUTPUT) {
      Serial.print("Setting Neopixel to color ( ");Serial.print(R);Serial.print(",");Serial.print(G);
      Serial.print(",");Serial.print(B);Serial.println(")");
    }
  } else {
    // no light
    for (int i = 1; i <= nrPixels; i++) {
      myNeo_PixelStrook.setPixelColor(i-1, myNeo_PixelStrook.Color(0,0,0));
      myNeo_PixelStrook.show();
    }
  }
}

