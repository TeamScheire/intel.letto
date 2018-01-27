int knop_waarde;
int beepstrength;
int beepDuration;
int i;
int beepDurationTotal;
int dotDuration;
int Drukknop1 = D7;
int buzzer = D6;

boolean Drukknop1_PRESSED = LOW;

long Drukknop1buttonTimer = 0;
#define Drukknop1minShortPressTime 80
#define Drukknop1longPressTime 750
boolean Drukknop1buttonActive = false;
boolean Drukknop1longPressActive = false;
#define Drukknop1NOPRESS    0
#define Drukknop1SHORTPRESS 1
#define Drukknop1LONGPRESS  2
int Drukknop1PressType = Drukknop1NOPRESS;


void handleDrukknop1Press() {
  Drukknop1PressType = Drukknop1NOPRESS;
      if (digitalRead(Drukknop1) == Drukknop1_PRESSED) {
        if (Drukknop1buttonActive == false) {
          Drukknop1buttonActive = true;
          Drukknop1buttonTimer = millis();
        }
        if ((millis() - Drukknop1buttonTimer > Drukknop1longPressTime) && (Drukknop1longPressActive == false)) {
          Drukknop1longPressActive = true;
          Drukknop1PressType = Drukknop1LONGPRESS;
        }
      } else {
        if (Drukknop1buttonActive == true) {
          if (Drukknop1longPressActive == true) {
            Drukknop1longPressActive = false;
          } else {
            //avoid fast fluctuations to be identified as a click
            if (millis() - Drukknop1buttonTimer > Drukknop1minShortPressTime)
              Drukknop1PressType = Drukknop1SHORTPRESS;
          }
          Drukknop1buttonActive = false;
        }
      }
}


// Deze functie beschrijven...
int ard_effect0_status = -1;
unsigned long ard_effect0_start, ard_effect0_time;
#define EFFECT0_PERIOD (dotDuration * 2)
#define EFFECT0_1_DURATION dotDuration

void dot() {
  //Variables of this effect are reffered to with ard_effect0
  boolean restart = false;
  ard_effect0_time = millis() - ard_effect0_start;
  if (ard_effect0_time > EFFECT0_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect0_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect0_status = -1;
    ard_effect0_start = ard_effect0_start + ard_effect0_time;
    ard_effect0_time = 0;
  }
  if (not restart && ard_effect0_status == -1) {
    ard_effect0_status = 0;
    ard_effect0_start = ard_effect0_start + ard_effect0_time;
    ard_effect0_time = 0;
  analogWrite(buzzer, beepstrength);
  }
  if (ard_effect0_time > EFFECT0_1_DURATION && ard_effect0_status < 1) {
   ard_effect0_status = 1;
  analogWrite(buzzer, 0);
  }
}


// Deze functie beschrijven...
int ard_effect1_status = -1;
unsigned long ard_effect1_start, ard_effect1_time;
#define EFFECT1_PERIOD (dotDuration * 4)
#define EFFECT1_1_DURATION dotDuration * 3

void dash() {
  //Variables of this effect are reffered to with ard_effect1
  boolean restart = false;
  ard_effect1_time = millis() - ard_effect1_start;
  if (ard_effect1_time > EFFECT1_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect1_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect1_status = -1;
    ard_effect1_start = ard_effect1_start + ard_effect1_time;
    ard_effect1_time = 0;
  }
  if (not restart && ard_effect1_status == -1) {
    ard_effect1_status = 0;
    ard_effect1_start = ard_effect1_start + ard_effect1_time;
    ard_effect1_time = 0;
  analogWrite(buzzer, beepstrength);
  }
  if (ard_effect1_time > EFFECT1_1_DURATION && ard_effect1_status < 1) {
   ard_effect1_status = 1;
  analogWrite(buzzer, 0);
  }
}


// Deze functie beschrijven...
int ard_effect2_status = -1;
unsigned long ard_effect2_start, ard_effect2_time;
#define EFFECT2_PERIOD (dotDuration * 3)

void letterpause() {
  //Variables of this effect are reffered to with ard_effect2
  boolean restart = false;
  ard_effect2_time = millis() - ard_effect2_start;
  if (ard_effect2_time > EFFECT2_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect2_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect2_status = -1;
    ard_effect2_start = ard_effect2_start + ard_effect2_time;
    ard_effect2_time = 0;
  }
  if (not restart && ard_effect2_status == -1) {
    ard_effect2_status = 0;
    ard_effect2_start = ard_effect2_start + ard_effect2_time;
    ard_effect2_time = 0;
  analogWrite(buzzer, 0);
  }
}


// Deze functie beschrijven...
int ard_effect3_status = -1;
unsigned long ard_effect3_start, ard_effect3_time;
#define EFFECT3_PERIOD (dotDuration * 33)
#define EFFECT3_1_DURATION dotDuration * 1
#define EFFECT3_2_DURATION dotDuration * 2
#define EFFECT3_3_DURATION dotDuration * 3
#define EFFECT3_4_DURATION dotDuration * 4
#define EFFECT3_5_DURATION dotDuration * 5
#define EFFECT3_6_DURATION dotDuration * 8
#define EFFECT3_7_DURATION dotDuration * 11
#define EFFECT3_8_DURATION dotDuration * 12
#define EFFECT3_9_DURATION dotDuration * 15
#define EFFECT3_10_DURATION dotDuration * 16
#define EFFECT3_11_DURATION dotDuration * 19
#define EFFECT3_12_DURATION dotDuration * 22
#define EFFECT3_13_DURATION dotDuration * 23
#define EFFECT3_14_DURATION dotDuration * 24
#define EFFECT3_15_DURATION dotDuration * 25
#define EFFECT3_16_DURATION dotDuration * 26
#define EFFECT3_17_DURATION dotDuration * 27

void SOS() {
  //Variables of this effect are reffered to with ard_effect3
  boolean restart = false;
  ard_effect3_time = millis() - ard_effect3_start;
  if (ard_effect3_time > EFFECT3_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect3_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect3_status = -1;
    ard_effect3_start = ard_effect3_start + ard_effect3_time;
    ard_effect3_time = 0;
  }
  if (not restart && ard_effect3_status == -1) {
    ard_effect3_status = 0;
    ard_effect3_start = ard_effect3_start + ard_effect3_time;
    ard_effect3_time = 0;
  analogWrite(buzzer, beepstrength);
  }
  if (ard_effect3_time > EFFECT3_1_DURATION && ard_effect3_status < 1) {
   ard_effect3_status = 1;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_2_DURATION && ard_effect3_status < 2) {
   ard_effect3_status = 2;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_3_DURATION && ard_effect3_status < 3) {
   ard_effect3_status = 3;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_4_DURATION && ard_effect3_status < 4) {
   ard_effect3_status = 4;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_5_DURATION && ard_effect3_status < 5) {
   ard_effect3_status = 5;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_6_DURATION && ard_effect3_status < 6) {
   ard_effect3_status = 6;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_7_DURATION && ard_effect3_status < 7) {
   ard_effect3_status = 7;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_8_DURATION && ard_effect3_status < 8) {
   ard_effect3_status = 8;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_9_DURATION && ard_effect3_status < 9) {
   ard_effect3_status = 9;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_10_DURATION && ard_effect3_status < 10) {
   ard_effect3_status = 10;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_11_DURATION && ard_effect3_status < 11) {
   ard_effect3_status = 11;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_12_DURATION && ard_effect3_status < 12) {
   ard_effect3_status = 12;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_13_DURATION && ard_effect3_status < 13) {
   ard_effect3_status = 13;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_14_DURATION && ard_effect3_status < 14) {
   ard_effect3_status = 14;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_15_DURATION && ard_effect3_status < 15) {
   ard_effect3_status = 15;
  analogWrite(buzzer, 0);
  } else if (ard_effect3_time > EFFECT3_16_DURATION && ard_effect3_status < 16) {
   ard_effect3_status = 16;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect3_time > EFFECT3_17_DURATION && ard_effect3_status < 17) {
   ard_effect3_status = 17;
  analogWrite(buzzer, 0);
  }
}


// Deze functie beschrijven...
int ard_effect4_status = -1;
unsigned long ard_effect4_start, ard_effect4_time;
#define EFFECT4_PERIOD 250
#define EFFECT4_1_DURATION 50

void beep() {
  //Variables of this effect are reffered to with ard_effect4
  boolean restart = false;
  ard_effect4_time = millis() - ard_effect4_start;
  if (ard_effect4_time > EFFECT4_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect4_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect4_status = -1;
    ard_effect4_start = ard_effect4_start + ard_effect4_time;
    ard_effect4_time = 0;
  }
  if (not restart && ard_effect4_status == -1) {
    ard_effect4_status = 0;
    ard_effect4_start = ard_effect4_start + ard_effect4_time;
    ard_effect4_time = 0;
  analogWrite(buzzer, beepstrength);
  }
  if (ard_effect4_time > EFFECT4_1_DURATION && ard_effect4_status < 1) {
   ard_effect4_status = 1;
  analogWrite(buzzer, 0);
  }
}


// Deze functie beschrijven...
int ard_effect5_status = -1;
unsigned long ard_effect5_start, ard_effect5_time;
#define EFFECT5_PERIOD 450
#define EFFECT5_1_DURATION 50
#define EFFECT5_2_DURATION 150
#define EFFECT5_3_DURATION 200
#define EFFECT5_4_DURATION 250
#define EFFECT5_5_DURATION 300
#define EFFECT5_6_DURATION 350
#define EFFECT5_7_DURATION 375
#define EFFECT5_8_DURATION 400

void beepCris() {
  //Variables of this effect are reffered to with ard_effect5
  boolean restart = false;
  ard_effect5_time = millis() - ard_effect5_start;
  if (ard_effect5_time > EFFECT5_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect5_status > -1) {
    //END STATEMENTS
      analogWrite(buzzer, 0);
    }
    restart = true;
    ard_effect5_status = -1;
    ard_effect5_start = ard_effect5_start + ard_effect5_time;
    ard_effect5_time = 0;
  }
  if (not restart && ard_effect5_status == -1) {
    ard_effect5_status = 0;
    ard_effect5_start = ard_effect5_start + ard_effect5_time;
    ard_effect5_time = 0;
  beepstrength = 20;
  analogWrite(buzzer, beepstrength);
  }
  if (ard_effect5_time > EFFECT5_1_DURATION && ard_effect5_status < 1) {
   ard_effect5_status = 1;
  beepstrength = 20;
  analogWrite(buzzer, 0);
  } else if (ard_effect5_time > EFFECT5_2_DURATION && ard_effect5_status < 2) {
   ard_effect5_status = 2;
  beepstrength = 80;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect5_time > EFFECT5_3_DURATION && ard_effect5_status < 3) {
   ard_effect5_status = 3;
  beepstrength = 20;
  analogWrite(buzzer, 0);
  } else if (ard_effect5_time > EFFECT5_4_DURATION && ard_effect5_status < 4) {
   ard_effect5_status = 4;
  beepstrength = 120;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect5_time > EFFECT5_5_DURATION && ard_effect5_status < 5) {
   ard_effect5_status = 5;
  beepstrength = 20;
  analogWrite(buzzer, 0);
  } else if (ard_effect5_time > EFFECT5_6_DURATION && ard_effect5_status < 6) {
   ard_effect5_status = 6;
  beepstrength = 50;
  analogWrite(buzzer, beepstrength);
  } else if (ard_effect5_time > EFFECT5_7_DURATION && ard_effect5_status < 7) {
   ard_effect5_status = 7;
  beepstrength = 20;
  analogWrite(buzzer, 0);
  } else if (ard_effect5_time > EFFECT5_8_DURATION && ard_effect5_status < 8) {
   ard_effect5_status = 8;
  beepstrength = 180;
  analogWrite(buzzer, beepstrength);
  }
}



void setup() {
  pinMode(Drukknop1, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  ard_effect0_status = -1;
  ard_effect0_start = millis();

  ard_effect1_status = -1;
  ard_effect1_start = millis();

  ard_effect2_status = -1;
  ard_effect2_start = millis();

  ard_effect3_status = -1;
  ard_effect3_start = millis();

  ard_effect4_status = -1;
  ard_effect4_start = millis();

  ard_effect5_status = -1;
  ard_effect5_start = millis();


  knop_waarde = 1;
  beepstrength = 255;
  beepDuration = 250;
  i = 250;
  beepDurationTotal = 250;
  dotDuration = 250;

}

void loop() {
  handleDrukknop1Press();

  if (Drukknop1PressType == Drukknop1SHORTPRESS) {
    //START STATEMENTS SHORT PRESS
    knop_waarde = knop_waarde + 1;
    if (knop_waarde > 6) {
      knop_waarde = 1;
    }
    //END  STATEMENTS SHORT PRESS
  } else if (Drukknop1PressType == Drukknop1LONGPRESS) {
    //START STATEMENTS LONG PRESS
    //END  STATEMENTS LONG PRESS
  } else if (!Drukknop1longPressActive && digitalRead(Drukknop1) == Drukknop1_PRESSED) {
    //START STATEMENTS PRESS
    //END  STATEMENTS PRESS
  }

  if (knop_waarde == 1) {
    beepstrength = 20;
    beep();
  } else if (knop_waarde == 2) {
    beepstrength = 120;
    beep();
  } else if (knop_waarde == 3) {
    beepCris();
  } else if (knop_waarde == 4) {
    beepstrength = 120;
    dash();
  } else if (knop_waarde == 5) {
    beepstrength = 255;
    SOS();
  } else if (knop_waarde == 6) {
    analogWrite(buzzer, 0);
  }

  // Deze functie beschrijven...

  // Deze functie beschrijven...

  // Deze functie beschrijven...

  // Deze functie beschrijven...

  // Deze functie beschrijven...

  // Deze functie beschrijven...

}
