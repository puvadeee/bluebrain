//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  04.10.2014  -  Inital Version  (wayne@cannybots.com)
// Version:   1.1  -  11.10.2014  -  Customisable bot name advertised over BLE (wayne@cannybots.com)
//                                   Do not set a custom GZLL_HOST_ADDRESS by default
// Version:   1.2  -  15.10.2014  -  Make use of joypad z axis (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "CannyBot1"                   // custom name (16 chars max)
//#define GZLL_HOST_ADDRESS 0x12ABCD00           // this needs to match the Joypad sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

// Motor Pins
#define MOTOR_A1_PIN                 0		// motor direction
#define MOTOR_A2_PIN                 6  	// motor speed (PWM)
#define MOTOR_B1_PIN                 1		// motor direction
#define MOTOR_B2_PIN                 5		// motor speed (PWM)
#define MOTOR_MAX_SPEED            255		// max speed (0..255)

#define JOYPAD_AXIS_DEADZONE 	    10

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile bool     buttonPressed = 0;              // 0 = not pressed, 1 = pressed

void setup() {
  Serial.end();

  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  radio_setup();
}

void loop() {
  radio_loop();    
  motorSpeed(yAxisValue, xAxisValue);
  //radio_debug("hello!");
}

void motorSpeed(int _speedA, int _speedB) {
  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}


void joypad_update(int x, int y, int z,int b) {
  // If the axis readings are small, in the 'deadzone', set them to 0
  if ( abs(x) < JOYPAD_AXIS_DEADZONE)  x = 0;
  if ( abs(y) < JOYPAD_AXIS_DEADZONE)  y = 0;

  xAxisValue    = x;
  yAxisValue    = y;
  buttonPressed = b;
}

