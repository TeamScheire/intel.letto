// Ingegno 01/2017
// Smart Alarm clock 
// adapted from 
// example from: http://www.instructables.com/id/Simplest-ESP8266-Local-Time-Internet-Clock-With-OL/
//
// Libraries needed:
//  Time.h & TimeLib.h:  https://github.com/PaulStoffregen/Time
//  Timezone.h: (via install library) https://github.com/JChristensen/Timezone
//  NTPClient.h: (via install library) https://github.com/arduino-libraries/NTPClient
//  ESP8266WiFi.h & WifiUDP.h: (via adding esp on board manager) https://github.com/ekstrand/ESP8266wifi
//  U8g2: for OLED display via install library
//  Adafruit neopixel library: via install library
//  PubSubClient library: via install library - This is to controll devices via MQTT


/*  START USER SETTABLE OPTIONS */
#define SERIALTESTOUTPUT false  // debug info
#define TEST_ALARM true         // to test, start alarm on switch on

// alarm
uint8_t alarm_hour = 7;
uint8_t alarm_min = 45;
bool alarm_set = true;  // do alarm or not
uint8_t sunrise_start_min_before = 15;  // minutes to start alarm (=sunrise) before alarm time (max 59)
uint8_t alarm_stop_min_after = 120;  // minutes to stop alarm after alarm time (max 120)
uint8_t beep_start_min_before = 5;
bool dageraad1on = false;               // we have two sunrise algorithms, use 1st or second?
bool use_static_IP = false;         //use a static IP address
uint8_t static_IP[4] = {192, 168, 1, 42}; 
uint8_t static_gateway_IP[4] = {192, 168, 1, 1};// if you want to use static IP make sure you know what the gateway IP is, here 192.168.1.1

// wifi data
// write here your wifi credentials 
const char* ssid = "*****";   // insert your own ssid 
const char* password = "********"; // and password

//mqtt server/broker 
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "192.168.0.111";

/*  END USER SETTABLE OPTIONS */


/* WIRING OF THE ALARM */
int Drukknop1 = D7;   //pushbutton on D7 (internal pullup)

int PixelStrook = D3;   //Neopixel IN wire

// I2C on D2 = SDA and D1 =SCL 
int pSDA = D2;
int pSCL = D1;
// Use U8X8_PIN_NONE if the reset pin is not connected
#define REPORTING_PERIOD_MS     500

/* END WIRING OF THE ALARM */


//includes
#include <Wire.h>

//wifi and timing lib
#include "wifilib.h"

//Adafruit_Neopixel usage library
#include "neopattern.h"

//Buzzer YL44 on NodeMCU usaga
#include "buzzerYL44.h"

// OLED display with U8g2 lib
#include <U8g2lib.h>

// pushbutton handling code
#include "pushbuttonlib.h"

// scenario lib for waking up
#include "wakescenario.h"

// variables
alarmstatus alarm_status = ALARM_OFF;
bool alarm_sunrise_on = false;
bool alarm_over_midnight = false;

unsigned long sunrise_millis = sunrise_start_min_before * 60 * 1000;
uint8_t alarm_sunrise_start_min, alarm_sunrise_start_hour, alarm_stop_min, 
        alarm_stop_hour;
unsigned int alarm_min_hour, alarm_sunrise_start_min_hour, alarm_stop_min_hour;
long sec_from_alarm;
long millis_from_alarm;


// setup the OLED display of 128x32
// Create a display object
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, pSCL, pSDA, U8X8_PIN_NONE);
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, pSDA, pSCL);
bool displayempty = true;

bool initialized = false;

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
  // no longer than given min of doing the alarm ! 
  alarm_stop_hour = alarm_hour;
  alarm_stop_min = alarm_min + alarm_stop_min_after;
  while (alarm_stop_min >= 60) {
    alarm_stop_min -= 60;
    alarm_stop_hour += 1;
  }
  if (alarm_stop_hour > 23) alarm_stop_hour -= 24;
  alarm_min_hour = alarm_hour *  60 + alarm_min;
  alarm_sunrise_start_min_hour = alarm_sunrise_start_hour * 60 + alarm_sunrise_start_min;
  alarm_stop_min_hour = alarm_stop_hour * 60 + alarm_stop_min;
  if (alarm_sunrise_start_min_hour > alarm_stop_min_hour) {
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
  setup_pushbtn();
  // YL44 buzzer
  setup_buzzer();
  
  //set up OLED and MPR121 over I2C bus 
  Wire.begin(pSDA, pSCL);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  u8g2.begin();
  initial_display(false);
  
  // Initialize the pixelStrips
  myNeo_PixelStrook.begin();
  // show no color
  myNeo_PixelStrook.ColorSet(myNeo_PixelStrook.Color(0, 0, 0));
  myNeo_PixelStrook.show();

  while (WiFi.status() != WL_CONNECTED) {
   setupWiFi(true); // Connect to local Wifi
  }
  //set randomseed
  randomSeed(micros());
  
  // connect to NTP 
  timeClient.begin();   // Start the NTP UDP client
  // mqtt client start
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
  initial_display(true);
  obtainedwifi = true;
  delay(1000);

  obtainDateTime();
  NTPstartTijd = millis();
  huidigeTijd = NTPstartTijd;

}

void loop() {
  
  //handle pushbutton press
  handleDrukknop1Press();

  //handle MQTT messages
  handleMQTTClient();

  /* SET CORRECT CURRENT TIME - UPDATE NTP IF NEEDED */
  huidigeTijd = millis();
  if (huidigeTijd - NTPstartTijd > NTPUpdateInterval) {
    //reupdate NTP data
    obtainDateTime();
    //reset NTPstart time
    NTPstartTijd = millis();
  }
  // compute current current time with min and sec
  determine_localtimenow();

  /* TEST CODE: AUTOMATICALLY SET ALARM 5 MIN AFTER START UP */
  if (TEST_ALARM && firstcorrecttime) {
    //correct time obtained, we set alarm 5 min from now
    alarm_hour = hour(localtimenow);
    alarm_min = minute(localtimenow)+5;
    if (alarm_min > 59) {
      alarm_min -= 60;
      alarm_hour += 1;
    }
    //set the alarm data
    determine_alarm_values(alarm_hour, alarm_min, sunrise_start_min_before);
    // only do this once !
    if (firstcorrecttime) firstcorrecttime = false;
  }
  /* END TEST CODE  */
  
  /** START DETERMINE ALARM ON OR NOT */
  if (alarm_set) {
    determine_alarm_time();
    if (SERIALTESTOUTPUT) Serial.print("Alarm set, sec:");Serial.println(sec_from_alarm);
    // now sec_from_alarm and millis_from_alarm are set !
    // also alarm_status is set, and now alarm_sunrise_on is true or false
  }
  /** END DETERMINE ALARM OR NOT */

  /** TWO POSSIBLE STATES: ALARM MODE OR NORMAL MODE */
  if ((alarm_status == ALARM_OFF ) || (alarm_status == ALARM_SWITCHED_OFF)) {
    /* NORMAL MODE */ 
    do_normal_mode();
  } else {
    /* ALARM MODE */ 
    do_alarm_mode();
  }
 
  
  // if disconnected, reconnect to wifi:
  if ( WiFi.status() != WL_CONNECTED) {
      delay(1);
      setupWiFi(false);
  }

  //we publish MQTT messages as needed, and repeat them every xx sec
  if (ventchanged || huidigeTijd - mqttventmsgtime > mqttmsginterval) {
    // send message to ventilator with the required setting:
    if (ventilator == VENT_ON) {
      MQTTpublish("cmnd/sonoff_ventilator/power", "1");
    } else if (ventilator == VENT_OFF) {
      MQTTpublish("cmnd/sonoff_ventilator/power", "0");
    }
    ventchanged = false;
    mqttventmsgtime = huidigeTijd;
  }
  if (wakelightchanged || huidigeTijd - mqttwakelightmsgtime > mqttmsginterval) {
    // send message to light with the required setting:
    if (wakelight == LIGHT_ON) {
      MQTTpublish("cmnd/sonoff_light/power", "1");
    } else if (wakelight == LIGHT_OFF) {
      MQTTpublish("cmnd/sonoff_light/power", "0");
    }
    wakelightchanged = false;
    mqttwakelightmsgtime = huidigeTijd;
  }
  
  if (SERIALTESTOUTPUT) {
    delay(500);
  } 
}

void do_normal_mode() {
  //buzzer in normal mode is OFF !
  analogWrite(buzzer, 0);

  // no alarm, plugs should be in free mode. Change to free if needed
  if (ventilator != VENT_FREE) {
    // alarm was on, now it is off. Switch plugs off if they were ON
    if (ventilator == VENT_ON) {
      // switch plug off before putting in FREE MODE
      ventilator = VENT_OFF;
      ventchanged = true;
    } else {
      // switch plug to free mode from OFF
      ventilator = VENT_FREE;
      ventchanged = true;
    }
  }
  if (wakelight != LIGHT_FREE) {
    // alarm was on, now it is off. Switch plugs off if they were ON
    if (wakelight == LIGHT_ON) {
      // switch plug off before putting in FREE MODE
      wakelight = LIGHT_OFF;
      wakelightchanged = true;
    } else {
      // switch plug to free mode from OFF
      wakelight = LIGHT_FREE;
      wakelightchanged = true;
    }
  }
  
  //display in normal mode; We can be in program modes, or in different display states.
  //Long press cycles display on, display alarm or display off
  //In display alarm, short press once goes to program mode hour. Long press then to program mode min
  // one more long press exits program mode 
  if (Drukknop1_PROGMODE_H) {
    if (Drukknop1_PROGMODE_H_1MORE) {
      alarm_hour += 1; 
      if (alarm_hour > 23) alarm_hour = 0;
      Drukknop1_PROGMODE_H_1MORE = false;
      determine_alarm_values(alarm_hour, alarm_min, sunrise_start_min_before);
    }
    displayAlarmH();
  } else if (Drukknop1_PROGMODE_M) {
    if (Drukknop1_PROGMODE_M_1MORE) {
      alarm_min += 1; 
      if (alarm_hour > 59) alarm_min = 0;
      Drukknop1_PROGMODE_M_1MORE = false;
      determine_alarm_values(alarm_hour, alarm_min, sunrise_start_min_before);
    }
    displayAlarmM();
  } else {
    //normal display modes
    if (knop_longpress_waarde == 1) {
      // we show date and time 
      displayDateTime();
    } else if (knop_longpress_waarde == 2) {
      // we show alarm time 
      displayAlarm();
    } else {
      // no display
      displayEmpty();
    }
  }

  // neopixel part light in normal mode
  if (knop_waarde == 1 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    // no light
    myNeo_PixelStrook.ColorSet(myNeo_PixelStrook.Color(0, 0, 0));
  } else if (knop_waarde == 2 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    //scanner
    myNeo_PixelStrook.Scanner(myNeo_PixelStrook.Color(255,0,0), 55);  // set neopixel in scanner mode
    
  } else if (knop_waarde == 3 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    myNeo_PixelStrook.TheaterChase(myNeo_PixelStrook.Color(255,255,0), myNeo_PixelStrook.Color(0,0,50), 100);
  } else if (knop_waarde == 4 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    myNeo_PixelStrook.RainbowCycle(3);
  } else if (knop_waarde == 5 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    myNeo_PixelStrook.ColorWipe(myNeo_PixelStrook.Color(255, 214, 170), 100);
  } else if (knop_waarde == 6 && Drukknop1PressType == Drukknop1SHORTPRESS) {
    //warm white light
    myNeo_PixelStrook.ColorSet(myNeo_PixelStrook.Color(255, 214, 170));
    myNeo_PixelStrook.show();
  }
  if (knop_waarde == 2 || knop_waarde == 3 || knop_waarde == 4 || knop_waarde == 5 ) {
    myNeo_PixelStrook.Update();
  }
}

void do_alarm_mode() {
  // display part
  // In alarm mode always show the date & time
  displayDateTime();
  
  // neopixel part In alarm mode, neopixel show the color that goes with the sunrise
  sunrise_color();

  determine_wake_scenario(sec_from_alarm, millis_from_alarm, beepstrength);
  
  // buzzer part We query wake scenario and operate buzzer
  if (personinbed == false || buzzer2sound == BUZZ_OFF) {
    analogWrite(buzzer, 0);
    buzzer2sound = BUZZ_OFF;
  } else if (Drukknop1PressType == Drukknop1SHORTPRESS && sec_from_alarm > 0) {
    // on press while in alarm mode after alarm mode, we snooze buzzer
    snoozetimeon = true;
    snoozetimestart = millis();
  }
  if (buzzer2sound == BUZZ_BEEPGALLOP) {
    beepGallop();
  } else if (buzzer2sound == BUZZ_DASH) {
    dash();
  } else if (buzzer2sound == BUZZ_DOT) {
    dot();
  } else if (buzzer2sound == BUZZ_BEEP) {
    beep();
  } else if (buzzer2sound == BUZZ_SOS) {
    SOS();
  }
  
  // ALARM is ON, we can switch OFF alarm mode with two long presses while NOT in bed
  if (personinbed) {
    knop_waarde = 1;
    knop_longpress_waarde = 1;
  } else {
    if (knop_longpress_waarde == 2) {
      alarm_status = ALARM_SWITCHED_OFF;
      ventilator = VENT_OFF;
      ventchanged = true;
      wakelight = LIGHT_OFF;
      wakelightchanged = true;
      buzzer2sound = BUZZ_OFF;
    }
  }
}
/****************************************************
 *  DETERMINE sec_from_alarm AND millis_from_alarm
 *  AND alarm_status based on current time and alarm
 *  time
 *  ************************************************/
void determine_alarm_time() {
  // localtimenow is the time shown on the display. Should there be alarm? 
  // If between start and end alarm, YES!
  // We keep track of seconds into the alarm

  // alarm is not set, NO alarm needed
  if (! alarm_set) {
    alarm_status = ALARM_OFF;
    sec_from_alarm = 0;
    millis_from_alarm = 0;
    return;
  }

  // determine time at the moment
  uint16_t curyear = year(localtimenow);
  uint8_t curmin = minute(localtimenow);
  uint8_t curhour = hour(localtimenow);
  uint8_t cursec = second(localtimenow);
  uint8_t curmillis = millis() % 1000;
  unsigned long curminhour = curhour * 60 + curmin;

  //determine if year is valid, if not, no correct NTP time, NO alarm needed
  if (curyear < 2010) {
    alarm_status = ALARM_OFF;
    sec_from_alarm = 0;
    millis_from_alarm = 0;
    return;
  }
  
  // Alarm is set, time is set. Are we now in alarm mode? 
  // DETERMINE sec_from_alarm AND millis_from_alarm
  bool old_alarm_sunrise_on = alarm_sunrise_on;
  sec_from_alarm = 0;
  millis_from_alarm = 0;
  if (!alarm_over_midnight) {
    if (curminhour >= alarm_sunrise_start_min_hour && curminhour < alarm_stop_min_hour) {
      // in alarm interval
      alarm_sunrise_on = true;
      //determine seconds from alarm
      sec_from_alarm = curminhour - alarm_min_hour;
    } else {
      // outside alarm interval
      alarm_sunrise_on = false;
    }
  } else {
    if (curminhour >= alarm_sunrise_start_min_hour || curminhour < alarm_stop_min_hour) {
      // in alarm interval
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
      // outside alarm interval
      alarm_sunrise_on = false;
    }
  }
  //if outside alarm interval, alarm is off
  if (!alarm_sunrise_on) {
    alarm_status = ALARM_OFF;
    sec_from_alarm = 0;
    millis_from_alarm = 0;
    return;
  }
  //if inside alarm interval, alarm already switched off, or pre alarm or alarm on !
  if (alarm_sunrise_on) {
    if (alarm_status == ALARM_SWITCHED_OFF) {
      // alarm remains switched off
      sec_from_alarm = 0;
      millis_from_alarm = 0;
      return;
    }
    if (!old_alarm_sunrise_on && (old_alarm_sunrise_on != alarm_sunrise_on)) {
      // alarm has started for the first time! 
      // reset buttons 
      knop_waarde = 1;
      knop_longpress_waarde = 1;
      alarm_status = PRE_ALARM;
    }
    if (sec_from_alarm < 0) {
      alarm_status = PRE_ALARM;
    } else {
      if (alarm_status == PRE_ALARM) {
        // we switch from PRE alarm to normal alarm 
        // update wake scenario variables if needed !
        next_vent_change = random(30, 61); // initial random ventilator duration
        ventilator = VENT_ON;              // ventilator on immediately after true alarm start
        ventchanged = true;
      }
      alarm_status = ALARM_ON;
    }
  }
  
  //before the alarm time eg 7:39 with alarm at 7:40 gives -1 minute
  //after the alarm time eg 7:41 with alarm at 7:40 gives +1 minute
  sec_from_alarm = sec_from_alarm * 60 + cursec;  // convert min to sec
  millis_from_alarm = sec_from_alarm * 1000 + curmillis;
  
  if (SERIALTESTOUTPUT) {
    Serial.println("Computed alarm values");
    Serial.println(alarm_sunrise_start_min_hour);
    Serial.println(alarm_min_hour);
    Serial.println(alarm_stop_min_hour);
    Serial.println(curminhour);
    Serial.println(sec_from_alarm);
    Serial.print("Alarm sunrise on? ");Serial.println(alarm_sunrise_on);
  }
}

void display_plugs(){
  // display what plug should be on or off. We have V(entilator), L(light to wake), M(assage)
  u8g2.setFont(u8g2_font_pixelle_micro_tr);
  if (ventilator == VENT_ON) {
    u8g2.drawStr(1,32,"V");
  } else if (ventilator == VENT_OFF){
    u8g2.drawStr(1,32,"-");
  }
  if (wakelight == LIGHT_ON) {
    u8g2.drawStr(6,32,"L");
  } else if (ventilator == LIGHT_OFF){
    u8g2.drawStr(6,32,"-");
  }
}

void display_inbed(){
  u8g2.setFont(u8g2_font_pixelle_micro_tr);
  if (personinbed) {
    u8g2.drawStr(100,25,"In Bed");
  } else {
    u8g2.drawStr(100,25,"Uit Bed");
  }
}

void display_wifi(bool wifi){
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
void initial_display(bool wifi) {
  if (not initialized)
  {
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
    
    //u8g2.drawStr(0,10,"Time");
    u8g2.drawStr(15,25,"Intel.Letto!");  // write something to the internal memory
    display_wifi(wifi);
    u8g2.sendBuffer();          // transfer internal memory to the display
    delay(1000);  
    displayempty = false;
  }
}

// nothing on display
void displayEmpty() {
  if (displayempty) return;

  u8g2.clearBuffer();   // clear the internal memory
  u8g2.sendBuffer();    // transfer internal memory to the display
  displayempty = true;
}

// display alarm time
void displayAlarm() {
  u8g2.clearBuffer();   // clear the internal memory
  char alarmtime[5];
  sprintf(alarmtime, "%02d:%02d",alarm_hour,alarm_min);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); //9px font https://github.com/olikraus/u8g2/wiki/fntlistall
  if (alarm_set) {
    u8g2.drawStr(0, 10, "Alarm AAN! Tijd:");
  } else {
    u8g2.drawStr(0, 10, "Alarm UIT! Tijd:");
  }
  u8g2.setFont(u8g2_font_profont17_tf); //11px font https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(32, 25, alarmtime);
  u8g2.sendBuffer();    // transfer internal memory to the display
  displayempty = false;
}

// display alarm time hour to program it
void displayAlarmH() {
  u8g2.clearBuffer();   // clear the internal memory
  char alarmtime[5];
  sprintf(alarmtime, "%02d:--",alarm_hour);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); //9px font https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(0, 10, "Zet Alarm uur:");
  u8g2.setFont(u8g2_font_profont17_tf); //11px font https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(32, 25, alarmtime);
  u8g2.sendBuffer();    // transfer internal memory to the display
  displayempty = false;
}

// display alarm time min to program it
void displayAlarmM() {
  u8g2.clearBuffer();   // clear the internal memory
  char alarmtime[5];
  sprintf(alarmtime, "--:%02d",alarm_min);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); //9px font https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(0, 10, "Zet Alarm min:");
  u8g2.setFont(u8g2_font_profont17_tf); //11px font https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(32, 25, alarmtime);
  u8g2.sendBuffer();    // transfer internal memory to the display
  displayempty = false;
}

// display the time on the OLED
void displayDateTime() {
    date = "";  // clear the variables
    t = "";
    displayempty = false;
  
    // print the date and time on the OLED
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_smart_patrol_nbp_tf); // choose a suitable font
    display_wifi(obtainedwifi);
    display_inbed();
    display_plugs();

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
  if (alarm_status != ALARM_OFF) {
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
    
    if (sec_from_alarm > 15*60) {
      // we fade in and out after 15 min of full light for safety
      myNeo_PixelStrook.ActivePattern = FADE;
      myNeo_PixelStrook.Interval = 20; // shorter is faster
      myNeo_PixelStrook.TotalSteps = 100; //steps in the fade in to out. 
      myNeo_PixelStrook.Color1 = myNeo_PixelStrook.Color(0,0,0);
      myNeo_PixelStrook.Color2 = myNeo_PixelStrook.Color(R,G,B);
      // make sure we see the color
      myNeo_PixelStrook.Update();
    } else {
      myNeo_PixelStrook.ColorSet(myNeo_PixelStrook.Color(R,G,B));
    }
    
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


