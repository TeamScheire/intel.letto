//#include <SPI.h>
#include <Wire.h>

#include <VL53L0X.h>
VL53L0X sensor;


#include <U8g2lib.h>

#define REPORTING_PERIOD_MS     500
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, D4, D3);



#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif

const int max_buffer = 120;
int data[max_buffer];

void setup() {
  Serial.begin(115200);
  Serial.println("\nVCNL4010 test");


  // voor arduino
  // Wire.begin(D3, D4);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL
  
  // voor esp8266
  Wire.begin(D3, D4);   // Initialiseer de I2C bus op pinnen GPIO0 (= D3) SDA en GPIO 02 (= D4) SCL
  sensor.init();
  sensor.setTimeout(500);

  u8g2.begin();
  u8g2.setCursor(78, 10);
  u8g2.setFont(u8g2_font_smart_patrol_nbp_tf);
  u8g2.print("Hans");
  u8g2.sendBuffer();

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  sensor.setMeasurementTimingBudget(200000);
#endif

  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)

}


void loop() {

  // Serial.print(sensor.readRangeSingleMillimeters());
  if (sensor.timeoutOccurred()) {
    Serial.print(" TIMEOUT");
  }
  // Serial.println();

  // text display tests
  // display.setCursor(0, 0);
  //display.setTextSize(1);
  //display.setTextColor(WHITE);

  int meting = sensor.readRangeSingleMillimeters();

  //Serial.print("meting = ");
  //Serial.println(meting);

  if (meting > 400) meting = 400;

  int pval = map(meting, 23, 400, 0, 120); // scale waarde voor horizontaal
  int pval2 = map(meting, 23, 400, 0, 29); // scale waarde voor vertikaal

  //Serial.print(" pval= ");
  //Serial.println(pval);

  u8g2.clearBuffer();

  //display.println(pval);
  u8g2.drawLine(0, 0, pval, 0); // teken horizontale lijn ifv afstand
  //u8g2.sendBuffer();            // toon data op scherm

  // array push, shift entire array to left by one spot
  for (int i = 1; i < max_buffer; i++) {
    data[i - 1] = data[i];
  }

  // add new data to the end of the array
  data[max_buffer - 1] = pval2;

  // draw bar graph
  for (int i = 0; i < max_buffer; i = i + 1) {
    u8g2.drawLine(i, 31, i, 31 - data[i]); // teken vertikale lijn ifv afstand

    //  Serial.print("data[i] = ");
    //  Serial.println(data[i]);

    //display.fillRect(i, 32 - data[i], 1, 20, WHITE);

  }

  u8g2.sendBuffer();            // toon data op scherm



  // shows the bar
  //display.fillRect(0, 10, pval, 1, WHITE);
  //display.display();

  //delay(10);
  //display.clearDisplay();
  /*

    u8g2.drawLine(0,0,  31, 60); // teken vertikale lijn ifv afstand
    //u8g2.drawLine(10, 31, 25); // teken vertikale lijn ifv afstand

    u8g2.sendBuffer();            // toon data op scherm
  */

}




