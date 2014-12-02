//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain - PingEcho Service (assumes unpopulated BlueBrain module)
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  28.11.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "PingEchoBot"                   // custom name (16 chars max)
//#define GZLL_HOST_ADDRESS 0x12ABCD00           // this needs to match the Joypad sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

#define JOYPAD_AXIS_DEADZONE 	    10

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile int16_t  zAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile int8_t   buttonPressed = 0;              // 0 = not pressed, 1 = pressed

void setup() {
  radio_setup();
  Serial.begin(9600);
}

void loop() {
  static unsigned int lastPing = millis();
  radio_loop();    
  if (millis() - lastPing > 1000) {
    lastPing= millis();
    //snprintf(msg, MSG_LEN+1, "%c%c%c%c", xAxis, yAxis, buttonPressed, zAxis );
    radio_debug("ping!");
  }
    Serial.print(xAxisValue);
    Serial.print(", ");
    Serial.print(yAxisValue);
    Serial.print(", ");
    Serial.print(zAxisValue);
    Serial.print(", ");
    Serial.println(buttonPressed);
}


void joypad_update(int x, int y, int z,int b) {
  xAxisValue    = x;
  yAxisValue    = y;
  zAxisValue    = z;
  buttonPressed = b;
}

