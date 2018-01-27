// ArdLab 25/10/2015
// Lid van STEM-Academie
// http://www.ardlab.gent
// Hans Caluwaerts

// getest met Arduino IDE 1.6.5


/*************************************************************************************************/
/* FUNCTIE                                                                                       */
/* Leer de I2C bus scannen om I2C hardware te vinden.                                            */
/*************************************************************************************************/

/*************************************************************************************************/
/* MATERIAAL                                    */
/* ArdLab MEGA experimenteerkit.                */
/*************************************************************************************************/

/*************************************************************************************************/
/* OPSTELLING                                                                                    */
/* Verbind GND van de RTC met de Arduino GND.                                                    */
/* Verbind VCC van de RTC met de Arduino 5V.                                                     */
/* Verbind SDA van de RTC met de Arduino pin 22.                                                 */
/* Verbind SCL van de RTC met de Arduino pin 21.                                                 */
/*************************************************************************************************/
// --------------------------------------
// i2c_scanner
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//


/* Deze sketch zoekt enkel naar 7 bit adressen
 * Adresen met meer bits worden waarschijnlijk niet gevonden.
 *
 *
 *  Te verwachten I2C adressen op het Arduino MEGA experimenteerkit
 *  0X50  DS1307 Real Time Clock
 *  0X68  EEPROM AT24C32N (op Tiny RTC print)
 *
 */

/*********************** importeer een bibliotheek *******************************/


#include <Wire.h>                   // Importeer de Wire bibliotheek voor het gebruik van de I2C bus

/**********************************************************************************/

void setup()  // De "setup" functie wordt 1 keer doorlopen na opstarten of indrukken van reset.
{
Wire.begin(D3, D4);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL

  Serial.begin(115200);                // Initialiseer de seriele communicatie tussen de Arduino en je computer, zet de communicatiesnelheid op 9600 bits per seconde.
  Serial.println("\n I2C Scanner");  // print I2C Scanner op een nieuwe lijn in de seriele monitor
 
} // einde void setup()

/**********************************************************************************/

void loop()      // De "loop" functie wordt steeds opnieuw herhaald.
{
  byte error, address;   //
  int nDevices;

  Serial.println("Scannen is bezig...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )   // start een teller address met waarde 1, verhoog de waarde address zo lang address kleiner is dan dan 127, verhoog address met een stap van 1
  {
    Serial.println(address);                     // print het gescande adres in de seriele monitor

    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device gevonden op adres 0x");    // toon alle I2C adressen waarop en I2C hardware gevonden is. 
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Onbekende fout op adres 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("Geen I2C devices gevonden\n");   // er is op geen enkel I2C bus adres hardware gevondnen 
  else
    Serial.println("Einde\n");

  delay(5000);           // wacht 5 seconden voor de volgende scan
  
}  // einde void loop
