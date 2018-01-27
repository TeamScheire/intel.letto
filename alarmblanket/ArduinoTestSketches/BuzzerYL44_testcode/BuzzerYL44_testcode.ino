// Ingegno 01/2018
// Buzzer YL-44 with NodeMCU

// This is an active buzzer, it produces sound by itself without
// external frequency generator. Sound is in the audible 2 kHz range
// Taking the I/O pin LOW will turn the buzzer ON and taking this
// pin HIGH will turn the buzzer OFF (as will leaving this pin OPEN). 
// This device can be controlled by PWM.

// The buzzer is connected to NodeMCU pin D6. This pin has 3V3 login, so the I/O pin
// of the buzzer would always be LOW with buzzer on as only with 5V buzzer is off
//
// Solution: we add a BC547 NPN transistor on the GND pin of the YL-44. This
//           allows to connect the I/O pin of the YL-44 to the GND out of the
//           NodeMCU, and the VCC to the 5V pin. The D6 pin is connected to the 
//           Base of the BC547, the GND of the YL-44 is connected to the Emittor
//           (right) of the BC547, and the Collector is connected to the GND of 
//           NodeMCU.
//           It is possible to achieve softer sounds using PWM on the D6 pin

//Adapted from Code written by Samuel 
//http://www.instructables.com/id/Arduino-YL-44-Buzzer-module/
// and KjartanA, LA6SRA in the comments

/* Buzzer type YL-44 with VCC, GND and I/O pin
*  YL-44 will have sound ON when I/O is LOW
*/

#define GNDYL44_2_NPN true    // do we connect the GND to an NPN to interrupt YL-44?
int buzzerPin = D6; //Define buzzerPin
const int wpm = 48;        // Morse speed in WPM*10

int buzzeron, buzzeroff, buzzerpwmoff;

const int dotL = 12000/wpm; // Calculated dot-length
const int dashL = 3*dotL;  // Dash length = 3 x dot
const int sPause = dotL;   // Symbol pause = 1 dot
const int lPause = dashL;  // Letter pause = 3 dots
const int wPause = 7*dotL; // Word pause = 7 dots

void setup() {
  if (GNDYL44_2_NPN) {
    buzzeron = HIGH;
    buzzeroff = LOW;
    buzzerpwmoff = 0;
  } else {
    buzzeron = LOW;
    buzzeroff = HIGH;
    buzzerpwmoff = 255;
  }
  pinMode(buzzerPin, OUTPUT); //Set buzzerPin as output
  beep(500, 0, 100); //Beep
  beep(500, 0, 50); //Beep
  delay(1000); //Add a little delay

}

void loop() {
  // PWM based beep
  for (int i=0; i<5; ++i) {
    beep(50, 200, 20); 
    beep(150, 150, 80); 
    delay(500); //Beep every 500 milliseconds
    beep(50, 200, 150); 
    beep(150, 150, 249); 
    }
  // Morse code beeps
  dash();
  dot();
  dash();
  dot();
  delay(lPause-sPause); // Subtracts pause already taken
  
  dash();
  dash();
  dot();
  dash();
  delay(wPause-sPause); // Subtracts pause already taken

  //wait a second
  delay(1000);
}

void beep(unsigned long beepduration, unsigned long nobeepduration, uint8_t strength) { 
  /* beep with the YL-44 using PWM. 
   *  strength 0: no sound
   *  strength 255: full sound
   * For normal I/O pin: pwmbeep 0 is full sound, pwmbeep 255 is off.
   * If GND on NPN, vica verca! 
  */
  if (GNDYL44_2_NPN) {
    analogWrite(buzzerPin, strength); //Setting pin to high via PWM
  } else {
    analogWrite(buzzerPin, 255 - strength); //Setting pin to high via PWM
  }
  delay(beepduration); //Delaying
  analogWrite (buzzerPin, buzzerpwmoff);  // Tone OFF
  delay(nobeepduration); //Delaying
  return;
}

void dot(){
  digitalWrite(buzzerPin, buzzeron); // Tone ON
  delay(dotL); // Tone length
  digitalWrite(buzzerPin, buzzeroff); // Tone OFF
  delay(sPause); // Symbol pause
  return;
}

void dash(){
  digitalWrite(buzzerPin, buzzeron); // Tone ON
  delay(dashL); // Tone length
  digitalWrite(buzzerPin, buzzeroff); // Tone OFF
  delay(sPause); // Symbol pause
  return;
}

