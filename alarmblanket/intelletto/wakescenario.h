

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

ventstate ventilator = VENT_FREE;
bool ventchanged = false;
unsigned long mqttventmsgtime = 0;
lightstate wakelight = LIGHT_FREE;
bool wakelightchanged = false;
unsigned long mqttwakelightmsgtime = 0;
// repeat mqtt messages for safety every xx millisec
unsigned long mqttmsginterval = 40000L;

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
    // we switch ventilator on for 60 sec, then off for 20.
    if (sec_alarm % 80 < 60) {
      // ventilator ON
      if (ventilator != VENT_ON) {
        ventilator = VENT_ON;
        ventchanged = true;
      }
    } else {
      // ventilator OFF
      if (ventilator != VENT_OFF) {
        ventilator = VENT_OFF;
        ventchanged = true;
      }
    }
  }
  // 4 min before alarm we set ventilator on  
  else if (sec_alarm > -4 * 60) {
    // we switch ventilator on 
    if (ventilator != VENT_ON) {
      ventilator = VENT_ON;
      ventchanged = true;
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
}

