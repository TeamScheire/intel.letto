// Ingegno 02/2017
// control massage mattress
// Wiring:
// 12V input to Arduino
// pin 3 to circuit to control motor 1 


//pin to the different motors
int NeckMotorsSTRONG = 2;
int NeckMotorsWEAK = 3;
int BreastMotorsSTRONG = 4;
int BreastMotorsWEAK = 5;
int BellyMotorsSTRONG = 6;
int BellyMotorsWEAK = 7;
int HipMotorsSTRONG = 8;
int HipMotorsWEAK = 9;

void setup() {
  pinMode(NeckMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(BreastMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(BellyMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(HipMotorsSTRONG, OUTPUT);   // sets the pin as output
  pinMode(NeckMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(BreastMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(BellyMotorsWEAK, OUTPUT);   // sets the pin as output
  pinMode(HipMotorsWEAK, OUTPUT);   // sets the pin as output
}

void loop() {
   digitalWrite(NeckMotorsWEAK, LOW);
   digitalWrite(NeckMotorsSTRONG, HIGH);
   delay(4000);
   digitalWrite(NeckMotorsSTRONG, LOW);
   digitalWrite(NeckMotorsWEAK, HIGH);
   delay(4000);
   digitalWrite(NeckMotorsSTRONG, LOW);
   digitalWrite(NeckMotorsWEAK, LOW);
   delay(4000);

}
