/*
Library used: http://playground.arduino.cc/Main/LedControl

HW used:
Arduino:      http://www.pololu.com/product/3101
LED Segment:  https://www.sparkfun.com/products/11405
Driver IC:    https://www.sparkfun.com/products/9622

Reference:    http://playground.arduino.cc/Main/MAX72XXHardware

MAX7219 / MAX7221 IC Pinout:

| 24  23  .  .  .  14  13
)
| 1   2   .  .  .  11  12


Wiring for: A* pin  -> MAX72xx pin

GND       4
GND       9
2         1  (DIN)
8        13  (CLK
9        12  (LOAD (CS))

Wiring for: MAX72xx pin  ->  LED 4 Segment pin
2       12    digit 0
11       9    digit 1
6        8    digit 2
7        6    digit 3
22       3    Decimal Point
14      13    seg a
16      7     seg b
20      4     seg c
23      2     seg d
21      1     seg e
15      10    seg f
17      5     seg g

Passive components wiring:

9  -> 100nf -> 19  (capacitor)
9  -> 10uf  -> 19  (electrolytic capacitor)
18 -> 330k  -> +ve (resistor, used to current limit the IC output to the LED's)
19 -> +ve

*/
#include "LedControl.h"

#define SEGMENT_COUNT  4
#define MAX72XX_COUNT  1
#define DATA_PIN       2
#define CLK_PIN        8
#define LOAD_PIN       9

#define DELAY_TIME      50
LedControl lc = LedControl(DATA_PIN, CLK_PIN, LOAD_PIN, MAX72XX_COUNT);

void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness */
  lc.setIntensity(0, 15);
  /* and clear the display */
  lc.clearDisplay(0);
}

void displayTime(unsigned long time) {
  for (int i = 0; i < SEGMENT_COUNT ; i++) {
    int d = ( time % (int)pow(10, SEGMENT_COUNT - i)  ) / pow(10, SEGMENT_COUNT - i - 1);
    lc.setDigit(0, i, d, i==0);
  }
  delay(DELAY_TIME);
  lc.clearDisplay(0);
}

void loop() {
  displayTime(millis());
}
