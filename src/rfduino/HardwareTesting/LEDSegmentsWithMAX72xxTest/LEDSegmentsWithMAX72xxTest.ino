#include "LedControl.h"

/*

Library used: http://playground.arduino.cc/Main/LedControl

HW used:
Arduino:      http://www.pololu.com/product/3101
LED Segment:  https://www.sparkfun.com/products/11405
DRiver IC:    https://www.sparkfun.com/products/9622

Reference:    http://playground.arduino.cc/Main/MAX72XXHardware

 ***** These pin numbers will probably need ot be changed for your Arduino *****

 pin 12 is connected to the DataIn
 pin 11 is connected to the CLK
 pin 10 is connected to LOAD
 We have only a single MAX72XX.

MAX7219 / MAX7221 IC Pinout:

| 24  23  .  .  .  14  13
)
| 1   2   .  .  .  11  12

Passivle component wiring:

9  -> 100nf -> 19  (capacitor)
9  -> 10uf  -> 19  (electrolytic capactir)
18  -> 330k    -> +ve (resistor, used to current limit the IC output t the LED's)
19  +ve

Wiring for: 
A*   -> MAX72**

GND       4
GND       9
2         1  DIN
8        13  CLK
9        12  LOAD (CS)

Wiringg for
MAX72**   ->  LED 4 Segment DIsplay pin
2       12    dig 0
11      9
6        8
7      6

22       3    Decimal Point

14      13    seg a
16      7     seg b
20      4     seg c
23      2     seg d
21      1     seg e
15      10    seg f
17      5     seg g
*/


#define SEGMENT_COUNT  4


LedControl lc = LedControl(2, 8, 9, 1);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 250;

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
  delay(delaytime);
  lc.clearDisplay(0);
}

void loop() {
  displayTime(millis());
}
