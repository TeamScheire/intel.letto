// Ingegno 02/2017
// control massage mattress
// Wiring:
// 12V input to Arduino
// pin 3 to circuit to control motor 1 
// SLAVE: This is code on the Arduino, which is a slave device
//        of a NodeMCU via I2C. 

#include <Wire.h>

bool USE_I2C = true;
bool USE_SERIAL = ! USE_I2C;
int intervalserial = 1000;   //output to serial
unsigned long lastMsg;

// auto switch off after maxMasDuration (15 min)
unsigned long maxMasDuration = 15 * 60 *1000L;;

const byte SLAVE_ADDRESS = 9;  // Massage control slave device

//pin to the different motors
int NeckMotorsSTRONG = 2;
int NeckMotorsWEAK = 3;
int BreastMotorsSTRONG = 4;
int BreastMotorsWEAK = 5;
int BellyMotorsSTRONG = 6;
int BellyMotorsWEAK = 7;
int HipMotorsSTRONG = 8;
int HipMotorsWEAK = 9;

int curMotor = 0;
int maxMotor = 7;
int Motors[8] = {NeckMotorsSTRONG, NeckMotorsWEAK, BreastMotorsSTRONG,
                BreastMotorsWEAK, BellyMotorsSTRONG, BellyMotorsWEAK,
                HipMotorsSTRONG, HipMotorsWEAK};

enum MASSAGE {MS_NONE, 
              MS_NECKON, MS_NECKOFF, MS_NECK, MS_NECKWEAK,
              MS_BREASTON, MS_BREASTOFF, MS_BREAST, MS_BREASTWEAK, 
              MS_BELLYON, MS_BELLYOFF, MS_BELLY, MS_BELLYWEAK, 
              MS_HIPON, MS_HIPOFF, MS_HIP, MS_HIPWEAK, 
              MS_F1};
              

MASSAGE massagescenario = MS_NONE;
bool massagechanged = false;

void MotorsOff() {
   digitalWrite(NeckMotorsSTRONG, LOW);
   digitalWrite(NeckMotorsWEAK, LOW);
   digitalWrite(BreastMotorsSTRONG, LOW);
   digitalWrite(BreastMotorsWEAK, LOW);
   digitalWrite(BellyMotorsSTRONG, LOW);
   digitalWrite(BellyMotorsWEAK, LOW);
   digitalWrite(HipMotorsSTRONG, LOW);
   digitalWrite(HipMotorsWEAK, LOW);
}

void Off() {
  MotorsOff();
}

void setup() {
  pinMode(NeckMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(BreastMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(BellyMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(HipMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(NeckMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(BreastMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(BellyMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(HipMotorsWEAK, OUTPUT);   // sets the pin as output
  
  if (USE_SERIAL) {
    //Serial.begin(115200);
    Serial.begin(57600);                    // connect to the serial port
    Serial.println("Matrass Controller");
  }
  if (USE_I2C) {
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveEvent); // register event
  }
  
}

void loop() {
  unsigned long now = millis(); 
  if (USE_SERIAL) {                
    if (now - lastMsg > intervalserial) {  // print every "interval" milliseconds or longer
      lastMsg = now;
      Serial.print("Doing Motor "); Serial.println(Motors[curMotor]);
      MotorsOff();
      digitalWrite(Motors[curMotor], HIGH);
      curMotor += 1;
      if (curMotor > maxMotor) {curMotor = 0;}
    }
  } else {
    // I2C SLAVE Device operation
    if (massagescenario == MS_NONE) {
      if (massagechanged) {
       Off();
       massagechanged = false;
      }
      //wait for input
      delay(200);
    } else if (now - lastMsg > maxMasDuration) {
      //switch off
      massagescenario == MS_NONE;
      massagechanged = true;
    } else {
      handleMassageScenario();
    }
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
// we receive a single integer which indicates what massage to do.
void receiveEvent(int howMany) {
  lastMsg = millis();
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
  }
  int x = Wire.read();    // receive byte as an integer

  massagescenario = (MASSAGE) x;
  massagechanged = true;
}

void handleMassageScenario() {
  // there is one of the scenario's, we execute on it.
  if (massagescenario == MS_NECK) {
    if (massagechanged) {
      //MotorsOff();
      //switch on NECK motor
      digitalWrite(NeckMotorsWEAK, LOW);
      digitalWrite(NeckMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_NECKWEAK) {
    if (massagechanged) {
      //MotorsOff();
      //switch on NECK motor
      digitalWrite(NeckMotorsSTRONG, LOW);
      digitalWrite(NeckMotorsWEAK, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BREAST) {
    if (massagechanged) {
      //MotorsOff();
      //switch on motor
      digitalWrite(BreastMotorsWEAK, LOW);
      digitalWrite(BreastMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BREASTWEAK) {
    if (massagechanged) {
      //MotorsOff();
      //switch on motor
      digitalWrite(BreastMotorsSTRONG, LOW);
      digitalWrite(BreastMotorsWEAK, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BELLY) {
    if (massagechanged) {
      //MotorsOff();
      //switch on motor
      digitalWrite(BellyMotorsWEAK, LOW);
      digitalWrite(BellyMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BELLYWEAK) {
    if (massagechanged) {
      //MotorsOff();
      //switch on motor
      digitalWrite(BellyMotorsSTRONG, LOW);
      digitalWrite(BellyMotorsWEAK, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_HIP) {
    if (massagechanged) {
      //MotorsOff();
      //switch on motor
      digitalWrite(HipMotorsWEAK, LOW);
      digitalWrite(HipMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_HIPWEAK) {
    if (massagechanged) {
      MotorsOff();
      //switch on motor
      //digitalWrite(HipMotorsSTRONG, LOW);
      digitalWrite(HipMotorsWEAK, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_NECKOFF) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(NeckMotorsSTRONG, LOW);
      digitalWrite(NeckMotorsWEAK, LOW);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BREASTOFF) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(BreastMotorsSTRONG, LOW);
      digitalWrite(BreastMotorsWEAK, LOW);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BELLYOFF) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(BellyMotorsSTRONG, LOW);
      digitalWrite(BellyMotorsWEAK, LOW);
      massagechanged = false;
    }
  } else if (massagescenario == MS_HIPOFF) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(HipMotorsSTRONG, LOW);
      digitalWrite(HipMotorsWEAK, LOW);
      massagechanged = false;
    }
  } else if (massagescenario == MS_NECKON) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(NeckMotorsWEAK, LOW);
      digitalWrite(NeckMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BREASTON) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(BreastMotorsWEAK, LOW);
      digitalWrite(BreastMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_BELLYON) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(BellyMotorsWEAK, LOW);
      digitalWrite(BellyMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_HIPON) {
    if (massagechanged) {
      //switch off motor
      digitalWrite(HipMotorsWEAK, LOW);
      digitalWrite(HipMotorsSTRONG, HIGH);
      massagechanged = false;
    }
  } else if (massagescenario == MS_F1) {
    if (massagechanged) {
      MotorsOff();
      //switch on motor
      massagechanged = false;
    }
    // F1 is an effect! Depending on the state motors are on/off
  }
}


