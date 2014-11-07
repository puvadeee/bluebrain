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
// Version:   1.3  -  21.10.2014  -  Added LineFollowing (mampetta@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "CannyBot1"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x12ABCD07           // this needs to match the Joypad sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_debug(char *fmt, ... );


// PIN ASSIGNEMENT
// total of 7 pins available of which any 4 can be defined as PWM
// Infrared sensors
#define IR1_PIN 2
#define IR2_PIN 3
#define IR3_PIN 4
// Motor Pins
#define MOTOR_A1_PIN 0	// motor direction
#define MOTOR_A2_PIN 6  // motor speed (PWM)
#define MOTOR_B1_PIN 1	// motor direction
#define MOTOR_B2_PIN 5	// motor speed (PWM)


// DEFINITIONS & ITIALISATION
#define MOTOR_MAX_SPEED 250 // max speed (0..255)
#define JOYPAD_AXIS_DEADZONE 20 //this is to eleminate jitter in 0 position
#define IR_WHITE_THRESHOLD 700 //to determinie on line or not 
//IR sensor bias
#define IR1_BIAS 0
#define IR2_BIAS 0
#define IR3_BIAS 0
// PID gain setting
#define PID_P 5  //PID gains set below in the code.. 
#define PID_I 0
#define PID_D 1
#define PID_SAMPLE_TIME 5 //determine how fast the loop is executed

//set IR initial reading to zero
int IRval1 = 0;
int IRval2 = 0;
int IRval3 = 0;

// PID working parameters
int Kp = PID_P;
int Ki = PID_I;
int Kd = PID_D;
int P_error = 0;
int I_sum = 0;
int I_error = 0;
int D_error = 0;
int error = 0;
int error_last = 0; // to calculate D_error = error - error_last
int correction = 0;

// Joypad
volatile int16_t  xAxisValue    = 0;              // (left) -255 .. 255 (right)
volatile int16_t  yAxisValue    = 0;              // (down) -255 .. 255 (up)
volatile int16_t  zAxisValue    = 0;
volatile bool     buttonPressed = 0;              // 0 = not pressed, 1 = pressed

// LineFollowing State
bool isLineFollowingMode = false;
bool forceManualMode     = false;
int justOut = 0; // to break the speed if the bot just came out of line

// Timers in milli-seconds (1/1000 of a second)
unsigned long time_Now        = millis();                    // the time at the start of the loop()
unsigned long pidLastTime     = millis();                // when the PID was calculated last
unsigned long joypadLastTime  = millis();             // the time the bot last received a joypad command
unsigned long offLineLastTime = millis();            // last time the bot came off the line
unsigned long offTheLineTime = 0;                    // how long has the bot been off the line, total since last leaving the line

// motor speeds
int lineSpeed = 0;
int speedA = 0;
int speedB = 0;

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

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors


  if (IRval2 >= IR_WHITE_THRESHOLD)
  {
    isLineFollowingMode = true;
    calculatePID();
    if (zAxisValue <= 0)
      zAxisValue = 0;
  }
  else
  {
    isLineFollowingMode = false;
    zAxisValue = 0;
    correction = 0;
  }

  speedA = (zAxisValue + correction) + (yAxisValue / 4 - xAxisValue / 4);
  speedB = (zAxisValue - correction) + (yAxisValue / 4 + xAxisValue / 4);

  motorSpeed(speedA, speedB);
  blink_loop();
}


// READ & PROCESS IR VALUES
void readIRSensors() {
  IRval1 = analogRead(IR1_PIN) + IR1_BIAS; //left looking from behind
  if (IRval1 >= 1000)
    IRval1 = 1000;

  IRval2 = analogRead(IR2_PIN) + IR2_BIAS; //centre
  if (IRval2 >= 1000)
    IRval2 = 1000;

  IRval3 = analogRead(IR3_PIN) + IR3_BIAS; //right
  if (IRval3 >= 1000)
    IRval3 = 1000;

}

// CALCULATE PID
void calculatePID() {

  // Calculate PID on a regular time basis
  if ((time_Now - pidLastTime) < PID_SAMPLE_TIME ) {
    // return if called too soon
    return;
  }
  pidLastTime = time_Now;

  // process IR readings via PID
  Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
  error_last = error; // store previous error value before new one is caluclated
  error = IRval1 - IRval3;
  P_error = error * Kp / 100.0; // calculate proportional term
  I_sum = constrain ((I_sum + error), -1000, 1000); // integral term
  I_error = I_sum * Ki / 100.0;
  D_error = (error - error_last) * Kd / 100.0;          // calculate differential term
  correction = P_error + D_error + I_error;
}


// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {

  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA > 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB > 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}


// READ INPUT FROM JOYPAD
void joypad_update(int x, int y, int z, int b) {
  // If the axis readings are small, in the 'deadzone', set them to 0
  if ( abs(x) < JOYPAD_AXIS_DEADZONE)  x = 0;
  if ( abs(y) < JOYPAD_AXIS_DEADZONE)  y = 0;
  if ( abs(z) < JOYPAD_AXIS_DEADZONE)  z = 0;

  xAxisValue = x;
  yAxisValue = y;
  zAxisValue = -z;
  buttonPressed = b;

  //radio_debug("%d,%d,%d,%d = %d,%d,%d,%d", x,y,z,b, xAxisValue,yAxisValue,zAxisValue,buttonPressed);
}


void blink_loop() {
  static unsigned long lastBlinkTime = 0;
  static bool ledsOn = false;

  //radio_debug("a=%d,b=%d,l=%d", speedA, speedB, ledsOn);
  if ( ( abs(speedA) > 0) || (abs(speedB) > 0)) {
    return;
  }
  if (  (millis() - lastBlinkTime) > 500) {
    lastBlinkTime = millis();
    digitalWrite(MOTOR_A1_PIN, ledsOn ? HIGH : LOW) ;
    digitalWrite(MOTOR_B1_PIN, ledsOn ? HIGH : LOW) ;
    ledsOn = !ledsOn;
  }
}
