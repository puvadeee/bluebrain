//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain RobotArm
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  19.11.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "RobotArm"                   // custom name (16 chars max)

#include <RFduinoBLE.h>

// Can't include GZLL, interferes with Servos (SDK v.2.1.2)

void radio_debug(char *fmt, ... );

#include <Servo.h>

// PIN ASSIGNEMENT
// total of 7 pins available of which any 4 can be defined as PWM
// Infrared sensors
#define SERVO1_PIN 2
#define SERVO2_PIN 3
#define SERVO3_PIN 4
Servo servos[3];

// DEFINITIONS & ITIALISATION
#define JOYPAD_AXIS_DEADZONE 20 //this is to eleminate jitter in 0 position

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile int16_t  zAxisValue    = 0;
volatile int8_t   buttonPressed = 0;              // 0 = not pressed, 1 = pressed



// READ INPUT FROM JOYPAD
void joypad_update(int x, int y, int z, int b) {
  // If the axis readings are small, in the 'deadzone', set them to 0
  if ( abs(x) < JOYPAD_AXIS_DEADZONE)  x = 0;
  if ( abs(y) < JOYPAD_AXIS_DEADZONE)  y = 0;
  if ( abs(z) < JOYPAD_AXIS_DEADZONE)  z = 0;

  xAxisValue = x;
  yAxisValue = y;
  zAxisValue = z;
  buttonPressed = b;

  //radio_debug("%d,%d,%d,%d = %d,%d,%d,%d", x,y,z,b, xAxisValue,yAxisValue,zAxisValue,buttonPressed);
}


void setup() {
  Serial.begin(9600);
  radio_setup();
  attachServos();
}

void attachServos() {
  servos[0].attach(SERVO1_PIN);
  servos[1].attach(SERVO2_PIN);
  servos[2].attach(SERVO3_PIN);
}
void detachServos() {
  servos[0].detach();
  servos[1].detach();
  servos[2].detach();
}

#define MAX 255

void loop() {
  radio_loop(); //read radio input
  
  if (buttonPressed & 1) {
    servos[0].write(map(xAxisValue, -255, 255, 0, MAX));
  }
  if (buttonPressed & 2) {
    servos[1].write(map(xAxisValue, -255, 255, 0, MAX));
  }
  if (buttonPressed & 4) {
    servos[2].write(map(xAxisValue, -255, 255, 0, MAX));
  }
  //servos[0].write(map(xAxisValue, -255, 255, 0, 179));
  //servos[1].write(map(yAxisValue, -255, 255, 0, 179));
  //servos[2].write(map(zAxisValue, -255, 255, 0, 179));

  Serial.println(buttonPressed);
}


