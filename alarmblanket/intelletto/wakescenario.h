

// Buzzer sounds supported:
enum  buzzersound { BUZZ_OFF, BUZZ_BEEP, BUZZ_BEEPGALLOP, BUZZ_DASH, BUZZ_DOT, BUZZ_SOS };

buzzersound buzzer2sound;
bool snoozetimeon = false;
unsigned long snoozetimestart;
unsigned long snoozetimeduration = 10 * 60 * 1000L; // 10 min

void determine_wake_scenario(long sec_alarm, long millis_alarm, int& beepstrength) {
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
      if (sec_alarm % 150 < 50) {
        buzzer2sound = BUZZ_DASH;
        beepstrength = 200;
      }
    }
  }
}

