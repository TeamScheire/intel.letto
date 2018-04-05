
/** START Pushbutton setup code
 */
boolean Drukknop1_PROGMODE_H = false;
boolean Drukknop1_PROGMODE_H_1MORE = false;
boolean Drukknop1_PROGMODE_M = false;
boolean Drukknop1_PROGMODE_M_1MORE = false;
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

int knop_waarde, knop_longpress_waarde;


void actionBasedOnDrukknop1Press() {
  if (Drukknop1PressType == Drukknop1SHORTPRESS) {
    //START STATEMENTS SHORT PRESS
    // in program mode, short presses are added to set timer
    if (Drukknop1_PROGMODE_H) {
      // set hour of alarm
      Drukknop1_PROGMODE_H_1MORE = true;
    } else if (Drukknop1_PROGMODE_M) {
      // set min of alarm
      Drukknop1_PROGMODE_M_1MORE = true;
    } else {
      // if in alarm mode, short press puts in programm mode
      if (knop_longpress_waarde == 2) {
        Drukknop1_PROGMODE_H = true;
      } else {
        knop_waarde = knop_waarde + 1;
        if (knop_waarde > 6) {
          knop_waarde = 1;
        }
        //END  STATEMENTS SHORT PRESS
      }
    }
  } else if (Drukknop1PressType == Drukknop1LONGPRESS) {
    //START STATEMENTS LONG PRESS
    // if in prog mode, long press stops prog mode
    if (Drukknop1_PROGMODE_H) {
      Drukknop1_PROGMODE_H = false;
      Drukknop1_PROGMODE_M = true;
    } else if (Drukknop1_PROGMODE_M) {
      Drukknop1_PROGMODE_H = false;
      Drukknop1_PROGMODE_M = false;
    } else {
      knop_longpress_waarde = knop_longpress_waarde + 1;
      if (knop_longpress_waarde > 3) {
        knop_longpress_waarde = 1;
      }
    }
    //END  STATEMENTS LONG PRESS
  } else if (!Drukknop1longPressActive && digitalRead(Drukknop1) == Drukknop1_PRESSED) {
    //START STATEMENTS PRESS
    //END  STATEMENTS PRESS
  }
}

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

  actionBasedOnDrukknop1Press();
}

void setup_pushbtn() {
  pinMode(Drukknop1, INPUT_PULLUP);
  knop_waarde = 1;
  knop_longpress_waarde = 1;
}

/** END Pushbutton setup code
 */ 
