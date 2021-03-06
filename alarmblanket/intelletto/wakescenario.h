
// Alarm zones
enum alarmstatus { ALARM_OFF, PRE_ALARM, ALARM_ON, ALARM_SWITCHED_OFF};

// Buzzer sounds supported:
enum  buzzersound { BUZZ_OFF, BUZZ_BEEP, BUZZ_BEEPGALLOP, BUZZ_DASH, BUZZ_DOT, BUZZ_SOS };

// Ventilator states supported: on, off, or uncontrolled (free)
enum ventstate {VENT_ON, VENT_OFF, VENT_FREE};

// Light states supported: on, off, or uncontrolled (free)
enum lightstate {LIGHT_ON, LIGHT_OFF, LIGHT_FREE};

buzzersound buzzer2sound;
bool snoozetimeon = false;
unsigned long snoozetimestart;
unsigned long snoozetimeduration = 10 * 60 * 1000L; // 10 min

unsigned long next_vent_change = 30;
ventstate ventilator = VENT_FREE;
bool ventchanged = false;
unsigned long mqttventmsgtime = 0;
lightstate wakelight = LIGHT_FREE;
bool wakelightchanged = false;
unsigned long mqttwakelightmsgtime = 0;
unsigned long next_massage_change = 20;
int nrmassagestates = 5;
static const char* const massagestates[] = {"O", "N", "B", "T", "H"};
const char* newmassagestate;
bool massagechanged = false;
unsigned long mqttmassagemsgtime = 0;
// repeat mqtt messages for safety every xx millisec
unsigned long mqttmsginterval = 5000L;

//variables for the  speak waking 
bool speachprogplaying = false, speachprognewtrack = false;
unsigned long speachprogtimestart = 0;
int speachprogdir = 0, speachprogtrack = 0, speachprogvolume=255;
#define SPEACHDIR_PREALARM  5 //directory with pre alarm messages
#define SPEACHDIR_PREALARM_NRTRACKS 7 //tracks from 001 to XXX
#define SPEACHDIR_TODAY 1 //directory with messages why to stand up today
#define SPEACHDIR_TODAY_NRTRACKS 4 //tracks from 001 to XXX
#define SPEACHDIR_GENERAL 2 //directory with general messages why to stand up
#define SPEACHDIR_GENERAL_NRTRACKS 3 //tracks from 001 to XXX
#define SPEACHDIR_ANIMAL 3 //directory with messages why to stand up today
#define SPEACHDIR_ANIMAL_NRTRACKS 4 //tracks from 001 to XXX
#define SPEACHDIR_FUNNY 4 //directory with general messages why to stand up
#define SPEACHDIR_FUNNY_NRTRACKS 6 //tracks from 001 to XXX


void determine_wake_scenario(long sec_alarm, long millis_alarm, int& beepstrength) {
  /* FIRST WE DETERMINE BUZZER WAKE SCENARIO
   *  
   */
  buzzer2sound = BUZZ_OFF;
  // Before alarm, buzz for 5 sec starting 5 min before
  int min_before= 5;
  if (sec_alarm < 0 && sec_alarm > -min_before * 60L && -sec_alarm % 60 < 5) {
    buzzer2sound = BUZZ_BEEPGALLOP;
    //pre alarm, we should increase the power as we are closer to alarm
    unsigned long beep_sec = min_before * 60 + sec_alarm;
    if (beep_sec > 0) {
      //beep_sec is value between and beep_start_min_before * 60, we map to 0 to 200
      beepstrength = map(beep_sec, 0, min_before * 60, 0, 200);
    }
  }
  if (sec_alarm > 0) {
    if (snoozetimeon) {
      // determine if snooze is finished 
      if (millis() - snoozetimestart > snoozetimeduration) {
        snoozetimeon = false;
      }
    }
    if (snoozetimeon) {
      buzzer2sound = BUZZ_OFF;
    } else {
      //we beep 30 seconds, then 1 min not
      if (sec_alarm % 90 < 30) {
        buzzer2sound = BUZZ_DASH;
        beepstrength = 200;
      }
    }
  }
  /* WE DETERMINE VENTILATOR WAKE SCENARIO
   *  
   */
  if (sec_alarm > 0) {
    // we randomly switch off and on ventilator for periods of 30 to 60 sec
    if (sec_alarm > next_vent_change) {
      // change ventilator status 
      if (ventilator != VENT_ON) {
        ventilator = VENT_ON;
        ventchanged = true;
      } else {
        ventilator = VENT_OFF;
        ventchanged = true;
      }
      // set a new time to change ventilator
      next_vent_change = sec_alarm + random(30, 60);
    }
  }
  // 1 min before alarm we set ventilator on  
  else if (sec_alarm > -1 * 60) {
    // we switch ventilator on 
    if (ventilator != VENT_ON) {
      ventilator = VENT_ON;
      ventchanged = true;
    }
  }
  // 5 min before alarm we alternate between on and off every 30 sec
  else if (sec_alarm > -5 * 60) {
    if ((-sec_alarm) % 60 < 30) {
      if (ventilator != VENT_ON) {
        ventilator = VENT_ON;
        ventchanged = true;
      }
    } else {
      if (ventilator != VENT_OFF) {
        ventilator = VENT_OFF;
        ventchanged = true;
      }
    }
  }
  else {
    // we are in alarm mode, but not yet time of alarm.
    // now: ventilation off!
    if (ventilator != VENT_OFF) {
      ventilator = VENT_OFF;
      ventchanged = true;
    }
  }

  /* WE DETERMINE WAKELIGHT WAKE SCENARIO
   *  
   */
  if (sec_alarm > 0) {
    // we switch wakelight on 
    if (wakelight != LIGHT_ON) {
      wakelight = LIGHT_ON;
      wakelightchanged = true;
    }
  }
  else {
    // we are in alarm mode, but not yet time of alarm.
    // now: wakelight off!
    if (wakelight != LIGHT_OFF) {
      wakelight = LIGHT_OFF;
      wakelightchanged = true;
    }
  }
  
  /* WE DETERMINE MASSAGE WAKE SCENARIO
   *  
   */
  if (sec_alarm > 0) {
    // we randomly switch off and on massage motors for periods of 10 to 20 sec
    if (sec_alarm > next_massage_change) {
      // change massage status
      massagechanged = true;
      // select a new state
      int newstate = random(0, nrmassagestates);
      int intensity = random(0, 3);
      //following not working, no time to debug, just code it ...
      /*      
      char buf[3]="";
      strcpy(buf, massagestates[newstate]);
      const char *zero = "0";
      const char *one = "1";
      const char *two = "2";
      if (intensity == 0) {
        //switch off
        strcat(buf, zero);
      } else if (intensity == 1) {
        strcat(buf, one);
      } else {
        strcat(buf, two);
      }
      newmassagestate = buf;  */ 
      if (massagestates[newstate] == "O") {
        newmassagestate = "O0";
      } else if (massagestates[newstate] == "N") {
        if (intensity == 0) {
          //switch off
          newmassagestate = "N0";
        } else if (intensity == 1) {
          newmassagestate = "N1";
        } else {
          newmassagestate = "N2";
        }
      } else if (massagestates[newstate] == "B") {
        if (intensity == 0) {
          //switch off
          newmassagestate = "B0";
        } else if (intensity == 1) {
          newmassagestate = "B1";
        } else {
          newmassagestate = "B2";
        }
      } else if (massagestates[newstate] == "T") {
        if (intensity == 0) {
          //switch off
          newmassagestate = "T0";
        } else if (intensity == 1) {
          newmassagestate = "T1";
        } else {
          newmassagestate = "T2";
        }
      }else if (massagestates[newstate] == "H") {
        if (intensity == 0) {
          //switch off
          newmassagestate = "H0";
        } else if (intensity == 1) {
          newmassagestate = "H1";
        } else {
          newmassagestate = "H2";
        }
      }
      
      // set a new time to change ventilator
      next_massage_change = sec_alarm + random(10, 21);
    } 
  } else if (sec_alarm > -15 * 60 && sec_alarm < -14 * 60) {
    //all motors on for one minute
    if (newmassagestate[0] != 'A') {
      newmassagestate = "A2";
      massagechanged = true;
    }
  } else if (sec_alarm > -10 * 60 && sec_alarm < -8 * 60) {
    // execute program 1 
    if (newmassagestate[0] != 'P') {
      newmassagestate = "P1";
      massagechanged = true;
    }
  } else if (sec_alarm > -4 * 60 && sec_alarm < -2 * 60) {
    // execute program 1 
    if (newmassagestate[0] != 'P') {
      newmassagestate = "P1";
      massagechanged = true;
    }
  } else {
    //no massage
    if (newmassagestate[0] != 'O') {
      newmassagestate = "O0";
      massagechanged = true;
    }
  }

  /* WE DETERMINE SPEACH WAKE SCENARIO
   *  
   */
  // 15 min before alarm a first test. Every 3 min another message
  // when alarm is actually started, do the texts from D001, + random texts every 
  // 30 sec
   
  // when playing a track, we do a 25 sec exclusion zone in which no new tracks are started
  speachprogdir = 0; //nothing to say
  speachprognewtrack = false;
  
  if (!speachprogplaying && sec_alarm < 0) {
    if (sec_alarm > -15*60 && sec_alarm < -15*60+20 ) {
      //say something from directory PREALARM
      speachprogdir = SPEACHDIR_PREALARM;
      //a random track between first and last track
      speachprogtrack = random(1, SPEACHDIR_PREALARM_NRTRACKS +1);
      // not too loud
      speachprogvolume = 100;
    } else if (sec_alarm > -12*60 && sec_alarm < -12*60+20 ) {
      //say something from directory PREALARM
      speachprogdir = SPEACHDIR_PREALARM;
      //a random track between first and last track
      speachprogtrack = random(1, SPEACHDIR_PREALARM_NRTRACKS +1);
      speachprogvolume = 80;
    } else if (sec_alarm > -9*60 && sec_alarm < -9*60+20 ) {
      //say something from directory PREALARM
      speachprogdir = SPEACHDIR_PREALARM;
      //a random track between first and last track
      speachprogtrack = random(1, SPEACHDIR_PREALARM_NRTRACKS +1);
      speachprogvolume = 60;
    } else if (sec_alarm > -6*60 && sec_alarm < -6*60+20 ) {
      //say something from directory PREALARM
      speachprogdir = SPEACHDIR_PREALARM;
      //a random track between first and last track
      speachprogtrack = random(1, SPEACHDIR_PREALARM_NRTRACKS +1);
      speachprogvolume = 40;
    } else  if (sec_alarm > -3*60 && sec_alarm < -3*60+20 ) {
      //say something from directory PREALARM
      speachprogdir = SPEACHDIR_PREALARM;
      //a random track between first and last track
      speachprogtrack = random(1, SPEACHDIR_PREALARM_NRTRACKS +1);
      speachprogvolume = 30;
    }
  }

  if (!speachprogplaying && sec_alarm > 0) {
    //alarm time, every min +0 sec an important reminder, every min+30 sec funny reminder
    if (sec_alarm % 60 < 20 ) {
      speachprogdir = random(1, 3);
      speachprogvolume = random(0, 10);
      if (speachprogdir == SPEACHDIR_TODAY) {
        //a random track between first and last track
        speachprogtrack = random(1, SPEACHDIR_TODAY_NRTRACKS +1);
      } else if (speachprogdir == SPEACHDIR_GENERAL) {
        //a random track between first and last track
        speachprogtrack = random(1, SPEACHDIR_GENERAL_NRTRACKS +1);
      }
    } else if (sec_alarm % 60 > 30 && sec_alarm % 60 < 50) {
      speachprogdir = random(3, 5);
      speachprogvolume = random(0, 20);
      if (speachprogdir == SPEACHDIR_ANIMAL) {
        //a random track between first and last track
        speachprogtrack = random(1, SPEACHDIR_ANIMAL_NRTRACKS +1);
      } else if (speachprogdir == SPEACHDIR_FUNNY) {
        //a random track between first and last track
        speachprogtrack = random(1, SPEACHDIR_FUNNY_NRTRACKS +1);
      }
    }
  }
  
  if (speachprogplaying) {
    //after 25 sec we should indicate playing stopped
    if (millis() < speachprogtimestart) {
      // millis looped around and restarted, reset
      speachprogplaying = false;
    } else if (millis() > speachprogtimestart + 25000) {
      // 25 sec passed since starting
      speachprogplaying = false;
    }
  }
  if (speachprogdir != 0) {
    //we should play a track
    speachprogplaying = true;
    speachprognewtrack = true;
    speachprogtimestart = millis();
  }
}

