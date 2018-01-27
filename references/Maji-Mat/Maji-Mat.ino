/* 
 *  Dit is de code voor Maji-Mat. De code is getest met een NodeMCU ESP-12 V1.0, een RGB kleurenled, een MPR121 en een ledstrip.
 *  Deze code meet de waarde van de 8 lijnen die over de matras verspreid zijn. De RGB kleurenled verandert van kleur wanneer er 1 
 *  van de draden veranderd. Bij de ledstrip veranderen een paar ledjes van de ledstrip naar gelang de lijn die aangeraakt wordt. 
 *  Als de eerste lijn aangeraakt wordt, veranderen de eerste 2 ledjes van groen naar rood, bij de 2de lijn die wordt aangeraakt 
 *  veranderen dan het 3de en 4de ledje van kleur. De waarde van deze lijndraden wordt ingelezen door een MPR121 en wordt verzonden 
 *  via I2C naar onze microcontroller. Deze verstuurt een tweet als er iemand een bepaalde tijd geen draad meer heeft aangeraakt.
 *  
 *  Deze code is gebaseerd op https://www.mysensors.org/build/bed_occupancy
 */


#include <Adafruit_NeoPixel.h>  // library ledstrip

#include <MPR121.h>             // library touchsensor
#include <Wire.h>

#include <ESP8266WiFi.h>        // library twitter
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

/* aan te passen aan situatie-------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------*/
#define pinled 12             // ledstrip GPIO pin
#define aantalleds 24

#define blauwLed 16           // rgb led 
#define groenLed 0

String tweetbericht ("NIC%20IS%20TE%20LANG%20UIT%20BED");    // het tweetbericht
String API = " ";              //tingspeak api 

const char* ssid = " ";    // wifi gebruikersnaam en wachtwoord
const char* password = " ";

/*----------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------*/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(aantalleds, pinled, NEO_GRB + NEO_KHZ800);  //ledstrip

boolean bedBezetNu;
boolean bedBezetVorig;
boolean kruipInBed;
boolean kruipUitBed;
boolean alarm;

int variantiewaarde = 10;   // verander bij een verschil van 10 van de lijn
int vorigewaarde[9];       // de vorige waarde van de lijnen
int opgeslagenwaarde[9];     // de vorige waarde van de lijnen als het bed bezet is
int bezet[9];          // geeft aan welke lijnen bezet zijn met een 1 bv. 01000010 als de 2de en 7de lijn aangeraakt worden

unsigned long kruipUitBedTime;    // meet hoe lang iemand al uit bed is
unsigned long StartTime;          // meet de tijd van de start
int tweetnummer = 1;

WiFiClient client;    // open client

void setup()
{
  strip.begin();
  strip.show();       // ledstrip uit

  Serial.begin(9600);
  
  pinMode(groenLed, OUTPUT);
  pinMode(blauwLed, OUTPUT);

  WiFi.begin(ssid, password);
  
   
  if (!MPR121.begin(0x5A)) {                      // 0x5A is het standaard adres van de MPR121 I2C
    Serial.println("error setting up MPR121");    // errors bij de MPR121
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
  }
}

void loop()
{
  strip.setPixelColor(2, 0, 0, 255);  //blauwe tussenleds ledstrip
  strip.setPixelColor(5, 0, 0, 255);
  strip.setPixelColor(8, 0, 0, 255);
  strip.setPixelColor(11, 0, 0, 255);
  strip.setPixelColor(14, 0, 0, 255);
  strip.setPixelColor(17, 0, 0, 255);
  strip.setPixelColor(20, 0, 0, 255);
  strip.setPixelColor(23, 0, 0, 255);

  if (bezet[0] == 1) {                   // leds groen of rood
    strip.setPixelColor(0, 255, 0, 0);
    strip.setPixelColor(1, 255, 0, 0);
  }
  else {
    strip.setPixelColor(0, 0, 255, 0);
    strip.setPixelColor(1, 0, 255, 0);
  }
  if (bezet[1] == 1) {
    strip.setPixelColor(3, 255, 0, 0);
    strip.setPixelColor(4, 255, 0, 0);
  }
  else {
    strip.setPixelColor(3, 0, 255, 0);
    strip.setPixelColor(4, 0, 255, 0);
  }
  if (bezet[2] == 1) {
    strip.setPixelColor(6, 255, 0, 0);
    strip.setPixelColor(7, 255, 0, 0);
  }
  else {
    strip.setPixelColor(6, 0, 255, 0);
    strip.setPixelColor(7, 0, 255, 0);
  }
  if (bezet[3] == 1) {
    strip.setPixelColor(9, 255, 0, 0);
    strip.setPixelColor(10, 255, 0, 0);
  }
  else {
    strip.setPixelColor(9, 0, 255, 0);
    strip.setPixelColor(10, 0, 255, 0);
  }
  if (bezet[4] == 1) {
    strip.setPixelColor(12, 255, 0, 0);
    strip.setPixelColor(13, 255, 0, 0);
  }
  else {
    strip.setPixelColor(12, 0, 255, 0);
    strip.setPixelColor(13, 0, 255, 0);
  }
  if (bezet[5] == 1) {
    strip.setPixelColor(15, 255, 0, 0);
    strip.setPixelColor(16, 255, 0, 0);
  }
  else {
    strip.setPixelColor(15, 0, 255, 0);
    strip.setPixelColor(16, 0, 255, 0);
  }
  if (bezet[6] == 1) {
    strip.setPixelColor(18, 255, 0, 0);
    strip.setPixelColor(19, 255, 0, 0);
  }
  else {
    strip.setPixelColor(18, 0, 255, 0);
    strip.setPixelColor(19, 0, 255, 0);
  }
  if (bezet[7] == 1) {
    strip.setPixelColor(21, 255, 0, 0);
    strip.setPixelColor(22, 255, 0, 0);
  }
  else {
    strip.setPixelColor(21, 0, 255, 0);
    strip.setPixelColor(22, 0, 255, 0);
  }
  strip.show();

  /**for (int x=0 ; x<8; x++){         //print de waarde van de bezette lijen bv.(00101100)
    Serial.print (bezet[x]);
    }
    Serial.println(" ");
  **/

  StartTime = millis();                   // start tijdstip
  unsigned long CurrentTime = millis();   // huidige tijdstip

  MPR121.updateFilteredData();  // lees nieuwe waarden binnen

  for (uint8_t i = 0; i < 8; i++) {             // Lees de huidige waarde van de sensor adjust for the number of sensors used
    int filtVal = MPR121.getFilteredData(i);    // neem de waarde van een lijn

    if (vorigewaarde[i] - filtVal > variantiewaarde ) {  //als de vorige staat van de sensor  - de huidige staat groter is dan variantiewaarde=10  dan is er iemand in bed
      if (bezet[i] == 0) {                                // Als hij nog op niemand in bed staat gaat hij naar bed bezet voor de pinnen
        opgeslagenwaarde[i] = vorigewaarde[i];           
        bezet[i] = 1;                                     // bed is bezet op deze lijn
      }
    }

    else if  (filtVal < opgeslagenwaarde[i] - (variantiewaarde * 2)) {   //de huidige waarde kleiner is dan  de vorige waarde voor dat er iemand in bed ging - 2x10=20
      bezet[i] = 1;                                              // bed blijft bezet
    }
    else {
      bezet[i] = 0;    // bed is niet meer bezet
    }

    vorigewaarde[i] = filtVal;
  }

  bedBezetNu = false;
  for (int x = 0 ; x < 8; x++) {       // als er 1 lijn bezet is, is het bed bezet
    if (bezet[x] == 1) {
      bedBezetNu = true;
    }
  }

  if (bedBezetNu)
  { 
    digitalWrite (blauwLed, HIGH);    // rgb kleurenled, blauwe led als er een draad aangeraakt wordt
    digitalWrite (groenLed, LOW);
  }
  else
  { 
    digitalWrite (blauwLed, LOW);     // rgb kleurenled, groene led als er geen draad aangeraakt wordt
    digitalWrite (groenLed, HIGH);
  }

  if (bedBezetNu && (!bedBezetVorig))   // iemand raakt een draad aan
    kruipInBed = true;
    
  else {
    kruipInBed = false;
  }
  
  if (bedBezetVorig && (!bedBezetNu))   // iemand laat een draad los
    kruipUitBed = true;
  else {
    kruipUitBed = false;
  }
  bedBezetVorig = bedBezetNu;

  if (kruipInBed) {                     // als er iemand in bed kruipt moet de rgb blauw oplichten en het alarm afzetten
    Serial.println("IkKruipInBed");
    digitalWrite (blauwLed, HIGH);
    alarm = false;
  }

  if (kruipUitBed) {                    // als er iemand uit bed kruipt moet de groene led weer oplichten en de tijd gemeten worden
    Serial.println("IkKruipUitBed" );
    digitalWrite (groenLed, HIGH);
    kruipUitBedTime = millis();
  }
  
  String tweet =  tweetbericht + String(tweetnummer);       // tweet bericht

  if (kruipUitBedTime!=0 & kruipUitBedTime + 10 < CurrentTime & !bedBezetNu & !alarm) {      // zend een tweet na 10 milliseconden als er niemand weer terug in bed gekropen is, als het bed niet bezet is en als het alarm nog niet afgegaan is
    Serial.println ("ALARM");

    while (WiFi.status() != WL_CONNECTED)
        Serial.println ("connected");
      if (client.connect("184.106.153.149", 80))
      {
        client.print("GET /apps/thingtweet/1/statuses/update?key=" + API + "&status=" + tweet + " HTTP/1.1\r\n");
        client.print("Host: api.thingspeak.com\r\n");
        client.print("Accept: */*\r\n");
        client.print("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n");
        client.print("\r\n");
      }

    tweetnummer += 1;
    alarm = true;
  }
}
