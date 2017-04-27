/* 
 * Simple Temperature Probe with 5x7 LED Matrix + indicator LED
 * Powell River Makerspace
 * 
 * Makers: Pat Long, Thomas Gray, Broden Gunn
 * Date: April 27, 2017
 * 
 * Hardware Components used:
 * - Arduino UNO
 * - DS18B20 based temperature probe (https://www.sparkfun.com/products/11050)
 * - 4.7kOhm resistor
 * - LTP-747E 5x7 LED dot matrix
 * - breadboard + wires
 * + 2x LED + 1k resistor (could use way less)
 * 
 * Resources:
 * 
 * Temperature Probe example:
 *  Source: https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
 *
 * LED Matrix example:
 *  Source: http://www.arduino.cc/playground/Main/DirectDriveLEDMatrix
 *
 * Modifications for 5x7 LED Matrix element
 * by Stefan Wolfrum in July 2012.
 * ----------------------------------------
 *
 * Uses FrequencyTimer2 library to
 * constantly run an interrupt routine
 * at a specified frequency. This
 * refreshes the display without the
 * main loop having to do anything.
 *
 */

#define REFRESH_RATE 2.0 // (frames per second)
#define SMILE_DELAY 3 * REFRESH_RATE
#define SMILE_EVERY 30 * REFRESH_RATE
#define INDICATE_LOW 20
#define INDICATE_HIGH 26
//#define DEBUG

// Headers for Temperature Probe
#include <DallasTemperature.h>
#include <OneWire.h>
#define ONE_WIRE_BUS 14 // (aka A0 - source: http://forum.arduino.cc/index.php?topic=96465.0)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Headers for LED Matrix
#include <FrequencyTimer2.h>

// custom pixel definitions
#include "num_defs_3x5_90ccw.h"
#define PIX_BLANK_ROW \
{0, 0, 0, 0, 0}
#define PIX_BLANK { \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0}, \
{0, 0, 0, 0, 0} \
}
#define PIX_SMILE { \
{0, 0, 0, 0, 0}, \
{1, 1, 0, 1, 0}, \
{1, 1, 0, 0, 1}, \
{0, 0, 0, 0, 1}, \
{1, 1, 0, 0, 1}, \
{1, 1, 0, 1, 0}, \
{0, 0, 0, 0, 0} \
}
#define MAX_NUM 99

#define INDICATOR_PIN_LOW 15 // (aka A1)
#define INDICATOR_PIN_HIGH 16 // (aka A2)
#define INDICATOR_PIN_OK 17 // (aka A3)

int indicateLow;
int indicateHigh;

// configurations for LED matrix
byte col = 0;
byte leds[5][7]; // columns x rows
 
// pin[xx] on led matrix connected to nn on Arduino
// (-1 is dummy to make array start at pos 1)
int pins[13]= {-1, 2, 9, 3, 11, 12, 13, 5, 6, 10, 4, 8, 7};
/* LTP-747E physical pin layout:
 *        1 --- 12
 *        2 --- 11
 *        3 --- 10
 *        4 --- 9
 *        5 --- 8
 *        6 --- 7
 */
// col[xx] of leds = pin yy on led matrix
int cols[5] = {pins[1], pins[3], pins[10], pins[7], pins[8]};
 
// row[xx] of leds = pin yy on led matrix
int rows[7] = {pins[12], pins[11], pins[2], pins[9],
 pins[4], pins[5], pins[6]};

// configure digit patterns
const int numDigits = 10;
byte digits[numDigits][3][5] = {
  PIX_0, PIX_1, PIX_2, PIX_3, PIX_4, PIX_5, PIX_6, PIX_7, PIX_8, PIX_9
};
byte digitSpacer[5] = PIX_BLANK_ROW;

// configure special patterns
const int numPatterns = 2;
byte patterns[numPatterns][7][5] = {
  PIX_BLANK, PIX_SMILE
};

// Debugging Configuration
#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.print (x)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTLN(x)
#endif

void setup()
{
 // initialize the Temperature Sensor
 // start serial port 
 Serial.begin(9600); 
 // start the sensor library
 sensors.begin(); 

  // initialize the LED Matrix
  // sets the pins as output
  for (int i = 1; i <= 12; i++) {
    pinMode(pins[i], OUTPUT);
  }
 
  // set up cols and rows
  for (int i = 1; i <= 5; i++) {
    digitalWrite(cols[i - 1], LOW);
  }
  for (int i = 1; i <= 7; i++) {
    digitalWrite(rows[i - 1], LOW);
  }
 
  clearLeds();

  indicateLow = INDICATE_LOW;
  pinMode(INDICATOR_PIN_LOW, OUTPUT);

  indicateHigh = INDICATE_HIGH;
  pinMode(INDICATOR_PIN_HIGH, OUTPUT);

  pinMode(INDICATOR_PIN_OK, OUTPUT);
 
  // Turn off toggling of pin 11
  FrequencyTimer2::disable();
  // Set refresh rate (interrupt timeout period)
  FrequencyTimer2::setPeriod(2000);
  // Set interrupt routine to be called
  FrequencyTimer2::setOnOverflow(display);
 
  setPattern(1); // service with a smile
}

void loop()
{
  static int frame = 0; // track the frame number
  static bool smiling = true; // smiling?
  
  // read the temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0); // Why "byIndex"?  
  // You can have more than one DS18B20 on the same bus.  
  // 0 refers to the first IC on the wire

  int temperatureN = (int)(temperature + 0.5); // Why + 0.5?
  // (int) cast always rounds down
  // this bumps it up when it's already past the 0.5 rounding point

  DEBUG_PRINT("FRAME #");
  DEBUG_PRINT(frame);
  DEBUG_PRINT(" @ ");
  DEBUG_PRINT((float)frame/(float)REFRESH_RATE);
  DEBUG_PRINTLN(" sec");
  DEBUG_PRINT(" * Temperature: ");
  DEBUG_PRINT(temperature);
  DEBUG_PRINTLN(" C");

  if (temperatureN <= indicateLow) {
    digitalWrite(INDICATOR_PIN_LOW, HIGH);
  }
  else if (temperatureN > indicateLow) {
    digitalWrite(INDICATOR_PIN_LOW, LOW);
  }
  
  if (temperatureN >= indicateHigh) {
    digitalWrite(INDICATOR_PIN_HIGH, HIGH);
  }
  else if (temperatureN < indicateHigh) {
    digitalWrite(INDICATOR_PIN_HIGH, LOW);
  }
  if (temperatureN > indicateLow && temperatureN < indicateHigh && frame % 2 != 0) {
    digitalWrite(INDICATOR_PIN_OK, HIGH);
  }
  else {
    digitalWrite(INDICATOR_PIN_OK, LOW);
  }

  // SMILE a bit at startup
  if (frame > SMILE_DELAY) {
    if (frame % SMILE_EVERY == 0) {
      DEBUG_PRINTLN("* SMILE *");
      // SMILE again every so often
      setPattern(1);
      smiling = true;
    }
    else if (smiling) {
      // finished smiling
      DEBUG_PRINTLN("* CLEAR SMILE *");
      setPattern(0);
      smiling = false;
    }
    else {
      // back to business of displaying temperature
      setNumber(temperatureN);
    }
  }

  // finish frame
  frame++;
  delay(1000/REFRESH_RATE);
}
 
void clearLeds()
{
  // Clear display array
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 7; j++) {
      leds[i][j] = 0;
    }
  }
}

void setNumber(int value)
{
  if (value < 0) {
    value = 0;
  }
  if (value > MAX_NUM) {
    value = MAX_NUM;
  }
  int ones = value % 10;
  int tens = value / 10;
  byte (*digit)[5];
  bool hasDigit = ((tens || ones) && ones < numDigits);
  if (hasDigit) {
    digit = digits[ones];
  }

  int rowOffset = 0;
  // "ones" column
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      if (hasDigit && digit) {
        leds[i][j+rowOffset] = digit[j][i];
      }
      else {
        leds[i][j+rowOffset] = 0;
      }
    }
  }
  rowOffset += 3;
  
  // a blank row between the digits
  for (int i = 0; i < 5; i++) {
    leds[i][rowOffset] = digitSpacer[i];
  }
  rowOffset++;

  // "tens" column
  hasDigit = (tens && tens < numDigits);
  if (hasDigit) {
    digit = digits[tens];
  }
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      if (hasDigit && digit) {
        leds[i][j+rowOffset] = digit[j][i];
      }
      else {
        leds[i][j+rowOffset] = 0;
      }
    }
  }
}
 
void setPattern(int pattern)
{
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 7; j++) {
      leds[i][j] = patterns[pattern][j][i];
    }
  }
}
 
// Interrupt routine
void display()
{
  // Turn whole previous column off:
  digitalWrite(cols[col], LOW);
  col++;
  if (col == 5) {
    col = 0;
  }
  for (int row = 0; row < 7; row++) {
    if (leds[col][row] == 1) {
      digitalWrite(rows[row], LOW); // Turn on this led
    }
    else {
      digitalWrite(rows[row], HIGH); // Turn off this led
    }
  }
  // Turn whole column on at once (for equal lighting times):
  digitalWrite(cols[col], HIGH);
}

