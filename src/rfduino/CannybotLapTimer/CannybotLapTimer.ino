#define BOT_NAME "LapTimer"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x12ABCD99           // this needs to match the Joypad sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"

// Sensor

#define SENSOR_SIG_PIN 2
#define SENSOR_VCC_PIN 4
#define SENSOR_GND_PIN 3

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

  pinMode(SENSOR_SIG_PIN, INPUT);
  attachPinInterrupt(SENSOR_SIG_PIN, myPinCallback1, HIGH);

}


int myPinCallback1(uint32_t ulPin) {
  static unsigned long lastCalled = millis();
  unsigned long now = millis();
  if ( (now - lastCalled) < 1000) 
    return 0;
  lastCalled=now;
  
  lapTimePrevious = lapTimeCurrent;
  lapTimeStart = now;
  laps++;
  if (1==laps) {
    return 0;
  }
  showTime = true;
  return 0;
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
  Serial.begin(9600);
  led_setup();
  trigger_setup();
  radio_setup();
}
void loop() {
  radio_loop(); //read radio input
  lapTimeCurrent = millis() - lapTimeStart;

  if (showTime) {
    led_writeTime( lapTimePrevious );
    showTime = false;
  }

  if (showStartBanner) {
    banner_start_lap();
  }

}

