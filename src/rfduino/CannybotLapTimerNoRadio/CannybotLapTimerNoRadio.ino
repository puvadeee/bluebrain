
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"

// Sensor


// Board	int.0	int.1	int.2	int.3	int.4	int.5
// Uno, Ethernet	2	3	 	 	 	 
// Mega2560	2	3	21	20	19	18
// Leonardo	3	2	0	1	7
                                                                                                                                                                                                                                                                                                                                                                                   
// see http://arduino.cc/en/Reference/attachInterrupt
#ifndef ARDUINO_AVR_LEONARDO 
#define SENSOR_SIG_PIN 2
#define SENSOR_GND_PIN 3
#define SENSOR_VCC_PIN 4
#define INT_NUM 0
#else  // Arduino UNO
#define SENSOR_SIG_PIN 7
#define SENSOR_GND_PIN 8
#define SENSOR_VCC_PIN 9
#define INT_NUM 3
#endif


// LED matrix

#define MATRIX_COUNT 5

Adafruit_BicolorMatrix matrices[MATRIX_COUNT];
uint8_t matrixAddress[MATRIX_COUNT] = { 0x70, 0x71, 0x72, 0x73, 0x74 }; // i2c addresses of matrix LED backbacks


volatile unsigned long lapTimeStart    = 0;
volatile unsigned long lapTimePrevious = 0;
volatile unsigned long lapTimeCurrent  = 0;
volatile unsigned long laps =0;

volatile bool showStartBanner = true;
volatile bool showTime = false;

//////////////
void led_setup() {
  for (int i = 0; i < MATRIX_COUNT ; i++) {
    matrices[i].begin(matrixAddress[i]);
    matrices[i].setRotation(3);
    matrices[i].setTextWrap(false);
    matrices[i].setTextSize(1);
    matrices[i].setTextColor(LED_YELLOW);
  }
}

void led_writeTime(unsigned long time) {
  // do nothing if the number won't fit.
  if (time >= pow(10, MATRIX_COUNT)) {
    return;
  }

  for (int i = 0; i < MATRIX_COUNT ; i++) {
    matrices[i].clear();
    matrices[i].setCursor(1, 0);
    int d = ( time % (int)pow(10, MATRIX_COUNT - i)  ) / pow(10, MATRIX_COUNT - i - 1);
    matrices[i].setTextColor(LED_YELLOW);
    matrices[i].print(d);
  }
  // draw the decimal place
  matrices[MATRIX_COUNT - 4].drawPixel(7, 6, LED_YELLOW);

  for (int i = 0; i < MATRIX_COUNT ; i++) {
    matrices[i].writeDisplay();
  }
}



/////////////////////////////////////////
void trigger_setup() {
  pinMode(SENSOR_VCC_PIN, OUTPUT);
  pinMode(SENSOR_GND_PIN, OUTPUT);
  digitalWrite(SENSOR_VCC_PIN, HIGH);
  digitalWrite(SENSOR_GND_PIN, LOW);

  //pinMode(SENSOR_SIG_PIN, INPUT);
  attachInterrupt(INT_NUM,myPinCallback1, FALLING);

}


void myPinCallback1() {
  static unsigned long lastCalled = millis();
  unsigned long now = millis();
  if ( (now - lastCalled) < 1000) 
    return;
  lastCalled=now;
  
  lapTimePrevious = lapTimeCurrent;
  lapTimeStart = now;
  laps++;
  if (1==laps) {
    return;
  }
  showTime = true;
  return;
}



void joypad_update(int x, int y, int z, int b) {
  if (1 == b) {
    showStartBanner = true;
  }
}

//

void banner_start_lap() {
  uint8_t cols[3] = {LED_RED, LED_RED, LED_YELLOW};

  for (int col = 0; col < 3; col++) {

    for (int i = 0; i < MATRIX_COUNT ; i++) {
      matrices[i].clear();
      matrices[i].fillCircle(4,  4, 3, cols[col]);
    }

    matrices[2].clear();
    matrices[2].setTextColor(cols[col]);
    matrices[2].setCursor(1, 0);
    matrices[2].print(3 - col);

    for (int i = 0; i < MATRIX_COUNT ; i++) {
      matrices[i].writeDisplay();
    }
    delay(1000);
  }

  for (int i = 0; i < MATRIX_COUNT ; i++) {
    matrices[i].clear();
    matrices[i].fillCircle(4,  4, 3, LED_GREEN);
    matrices[i].writeDisplay();
  }
  showStartBanner = false;
  laps=0;
}


/////////////////////////////////////////
void setup() {
  //Serial.begin(9600 n);
  led_setup();
  trigger_setup();
}
void loop() {
  lapTimeCurrent = millis() - lapTimeStart;

  if (showTime) {
    led_writeTime( lapTimePrevious );
    showTime = false;
  }

  if (showStartBanner) {
    banner_start_lap();
  }

}

