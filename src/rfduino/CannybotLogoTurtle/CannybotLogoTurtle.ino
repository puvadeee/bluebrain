//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Logo Turtle
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  15.01.2015  -  Inital Version  (wayne@cannybots.com)

////////////////////////////////////////////////////////////////////////////////////////// ///////

// WArning: enabling serial debug will cause the motors to rotate only one way.
//#define   SERIAL_DEBUG
//#define   SERIAL_DEBUG_DETAILED       // Show encoder counts
#define GZLL_HOST_ADDRESS 0x99ACB010    // TURTLE

#define BOT_NAME "Turtle"                   // custom name (16 chars max)
#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_send_formatted(char *fmt, ... );

// configure depending on physical characteristics
#define COUNTS_PER_REVOLUTION 430
#define WHEEL_DIAMETER        38      // mm

#define MOTOR_MAX_SPEED 250 // max speed (0..255)

// Time based hacks! (for dev use without encoders)
//#define USE_DELAYS_FOR_MOVEMENT
#define MAX_TURTLE_MOTOR_SPEED  80
#define MOTION_DELAY_MULTIPLIER 60
#define TURN_DELAY_MULTIPLIER 3

#define ANGLE_CALC 1
#define DISTANCE_CALC 2

// Pin Assignments

// Motors
#define MOTOR_A1_PIN 0	// motor direction
#define MOTOR_A2_PIN 6  // motor speed (PWM)
#define MOTOR_B1_PIN 1	// motor direction
#define MOTOR_B2_PIN 5	// motor speed (PWM)

// Wheel encoders
#define ENCODER1A_PIN 2
#define ENCODER1B_PIN x
#define ENCODER2A_PIN 3
#define ENCODER2B_PIN 4


#define COUNTER_NONE 0
#define COUNTER_1A 1
#define COUNTER_1B 2
#define COUNTER_2A 3
#define COUNTER_2B 4


// Wheel encoder state
volatile unsigned int encoder1ACount = 0;
volatile unsigned int encoder1BCount = 0;
volatile unsigned int encoder2ACount = 0;
volatile unsigned int encoder2BCount = 0;
int currentCounter = 0;
int currentCounterTarget = 0;



// Command statuses
#define RC_OK                     0
#define RC_UNKNOWN_COMMAND        1
#define RC_COMMAND_UNIMPLEMENTED  2

// Command processing state
enum command_state_t {COMMAND_IDLE, COMMAND_READY, COMMAND_RUNNING} ;

// Comman processing
uint8_t commandProcessingState = COMMAND_IDLE;
uint8_t nextCommand = 0;
int16_t nextP1 = 0;


void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
#else
  Serial.end();
#endif

  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  // encoder
  //pinMode(ENCODER1A_PIN, INPUT);
  pinMode(ENCODER2A_PIN, INPUT);
  pinMode(ENCODER2B_PIN, INPUT);
  //attachPinInterrupt(ENCODER1A_PIN, encoderCallbackSensor1A, HIGH);
  //attachPinInterrupt(ENCODER1B_PIN, encoderCallbackSensor1B, HIGH);
  attachPinInterrupt(ENCODER2A_PIN, encoderCallbackSensor2A, HIGH);
  attachPinInterrupt(ENCODER2B_PIN, encoderCallbackSensor2B, HIGH);

  radio_setup();
}

void motorTestPattern() {
  motorSpeed(255, 255);
  delay(500);
  motorSpeed(-255, -255);
  delay(500);
  motorSpeed(0, 0);
  delay(1000);

  motorSpeed(MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);  delay(500); motorSpeed(0, 0); delay(500);
  motorSpeed(-MAX_TURTLE_MOTOR_SPEED, -MAX_TURTLE_MOTOR_SPEED); delay(500);  motorSpeed(0, 0); delay(1000);

  radio_send("TurtleUp!");
  cmd_forward(50);  delay(500);
  cmd_backward(50); delay(500);
  cmd_forward(50);  delay(500);

  motorSpeed(0, 0);

  cmd_left(90);     delay(500);
  cmd_right(90);    delay(500);

  cmd_left(90);     delay(500);
  cmd_right(90);    delay(500);

  for (int i = 0; i < 4; i++) {
    cmd_forward(100);
    cmd_right(90);
  }

  motorSpeed(0, 0);
}


int encoderCallbackSensor1A(uint32_t ulPin) {
  encoder1ACount++;
  return 0;
}

int encoderCallbackSensor1B(uint32_t ulPin) {
  encoder1ACount++;
  return 0;
}

int encoderCallbackSensor2A(uint32_t ulPin) {
  encoder2ACount++;
  return 0;
}

int encoderCallbackSensor2B(uint32_t ulPin) {
  encoder2BCount++;
  return 0;
}


void resetEnocderCounters() {
  encoder1ACount = 0;
  encoder1BCount = 0;
  encoder2ACount = 0;
  encoder2BCount = 0;
}


void setTargetCounter(int counter, int target) {
  currentCounter = counter;
  currentCounterTarget = target;
}

int checkEncoderCountsForStopping() {
  bool stopMotors = false;
  switch (currentCounter) {
    case COUNTER_1A: stopMotors = encoder1ACount > currentCounterTarget; break;
    case COUNTER_1B: stopMotors = encoder1BCount > currentCounterTarget; break;
    case COUNTER_2A: stopMotors = encoder2ACount > currentCounterTarget; break;
    case COUNTER_2B: stopMotors = encoder2BCount > currentCounterTarget; break;
  }
  if (stopMotors) {
    motorSpeed(0, 0);
  }
  return stopMotors;
}

int calculateEncoderCountForDistance(int distance) {
  int count = 0;
  float wheelCircumference = WHEEL_DIAMETER * M_PI;                       // =  32mm x Pi =  100.53
  float ticksPerMilliMeter = COUNTS_PER_REVOLUTION / wheelCircumference;  // =  300 ticks /  100.53 = 2.98
  // => fwd 100 mm =    distance * ticksPerMilliMeter =  100 * 2.98 = 298 ticks

  count = (float)fabs(distance) * (float)ticksPerMilliMeter;
  return count;
}

int calculateEncoderCountForAngle(int angle) {
  angle = abs(angle % 360);
  int count = 0;
  // ticksPerAngle = 300/360 = 0.83
  float ticksPerAngle = (float)COUNTS_PER_REVOLUTION / (float)360;                // =  300 ticks / 360 = 0.83
  // => right 90 =    angle * ticksPerAngle =  90 * 0.83 = 74.7 ticks

  count = (float)angle * (float)ticksPerAngle * 2;

  return count;
}



int setupCounterFor(int calcMode, int distanceOrAngle) {

  resetEnocderCounters();
  int ticks = 0;
  if (calcMode == ANGLE_CALC) {
    ticks = calculateEncoderCountForAngle(distanceOrAngle);

    if (distanceOrAngle > 0) {
      setTargetCounter(COUNTER_2A, ticks);
    } else if (distanceOrAngle < 0) {
      setTargetCounter(COUNTER_2B, ticks);
    } else {
      ticks = 0;
    }
  } else {
    ticks = calculateEncoderCountForDistance(distanceOrAngle);

    if (distanceOrAngle > 0) {
      setTargetCounter(COUNTER_2B, ticks);
    } else if (distanceOrAngle < 0) {
      setTargetCounter(COUNTER_2A, ticks);
    } else {
      ticks = 0;
    }
  }

  return ticks;
}


// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {
#if defined(SERIAL_DEBUG)
  Serial.print("Motors (Not sent) A,B:\t");
  Serial.print(_speedA, DEC);
  Serial.print("\t");
  Serial.println(_speedB, DEC);
  return;
#endif
  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));
  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}



///////////////



uint8_t cmd_stop(int16_t p1) {
  motorSpeed(0, 0);
  return RC_OK;
}

#if defined(USE_DELAYS_FOR_MOVEMENT)

// used as a fallback, this is a very poor approximation method, no good for drawing just debug, maybe only navigating a bit
uint8_t cmd_forward(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);
  delay(p1 * MOTION_DELAY_MULTIPLIER);
  motorSpeed(0, 0);
  return RC_OK;
}

uint8_t cmd_backward(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);
  delay(p1 * MOTION_DELAY_MULTIPLIER);
  motorSpeed(0, 0);
  return RC_OK;
}

uint8_t cmd_left(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED, -MAX_TURTLE_MOTOR_SPEED);
  delay(p1 * TURN_DELAY_MULTIPLIER);
  motorSpeed(0, 0);
  return RC_OK;
}

uint8_t cmd_right(int16_t p1) {
  motorSpeed(-MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);
  delay(p1 * TURN_DELAY_MULTIPLIER);
  motorSpeed(0, 0);
  return RC_OK;
}
#else

uint8_t cmd_forward(int16_t p1) {
  if (p1 == 0) {
    motorSpeed(0, 0);
    return RC_OK;
  }

  setupCounterFor(DISTANCE_CALC, p1);
  motorSpeed(MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);
  // block for movement, possible cout return a 'pending' rc that doesnt return OK immediately to the client
  while (!checkEncoderCountsForStopping()) {
  }
  motorSpeed(0, 0);

  return RC_OK;
}

uint8_t cmd_backward(int16_t p1) {
  if (p1 == 0) {
    motorSpeed(0, 0);
    return RC_OK;
  }
  setupCounterFor(DISTANCE_CALC, -p1);
  motorSpeed(-MAX_TURTLE_MOTOR_SPEED, -MAX_TURTLE_MOTOR_SPEED);
  while (!checkEncoderCountsForStopping()) {
  }
  motorSpeed(0, 0);
  return RC_OK;
}

uint8_t cmd_left(int16_t p1) {
  if (p1 == 0) {
    motorSpeed(0, 0);
    return RC_OK;
  }
  setupCounterFor(ANGLE_CALC, -p1);
  motorSpeed(MAX_TURTLE_MOTOR_SPEED, -MAX_TURTLE_MOTOR_SPEED);
  while (!checkEncoderCountsForStopping()) {
  }
  motorSpeed(0, 0);
  return RC_OK;
}

uint8_t cmd_right(int16_t p1) {
  if (p1 == 0) {
    motorSpeed(0, 0);
    return RC_OK;
  }
  setupCounterFor(ANGLE_CALC, p1);
  motorSpeed(-MAX_TURTLE_MOTOR_SPEED, MAX_TURTLE_MOTOR_SPEED);
  while (!checkEncoderCountsForStopping()) {
  }
  motorSpeed(0, 0);
  return RC_OK;
}
#endif


uint8_t cmd_pen_up(int16_t p1) {
  return RC_COMMAND_UNIMPLEMENTED;
}
uint8_t cmd_pen_down(int16_t p1) {
  return RC_COMMAND_UNIMPLEMENTED;
}
uint8_t cmd_audio_alert(int16_t p1) {
  return RC_COMMAND_UNIMPLEMENTED;
}
uint8_t cmd_visual_alert(int16_t p1) {
  return RC_COMMAND_UNIMPLEMENTED;
}

uint8_t cmd_check_turtle_status() {
  return RC_OK;
}

void logo_received_command(uint8_t command, int16_t p1) {
  if (commandProcessingState != COMMAND_IDLE)
    return;

  nextCommand = command;
  nextP1 = p1;
  commandProcessingState = COMMAND_READY;
}

void logo_check_command_queue() {
  if ( commandProcessingState == COMMAND_READY ) {
    commandProcessingState = COMMAND_RUNNING;
    uint8_t rc = logo_run_command(nextCommand, nextP1);
    send_command_status(rc);
    commandProcessingState = COMMAND_IDLE;
  }
}

uint8_t logo_run_command(uint8_t command, int16_t p1) {
#if defined(SERIAL_DEBUG)

  Serial.print("Logo cmd:\t");
  Serial.print((char)command);
  Serial.print("\t");
  Serial.println(p1, DEC);
#endif
  uint8_t rc = RC_UNKNOWN_COMMAND;
  switch (command) {
    case 's': rc = cmd_stop(p1); break;
    case 'f': rc = cmd_forward(p1); break;
    case 'b': rc = cmd_backward(p1); break;
    case 'l': rc = cmd_left(p1); break;
    case 'r': rc = cmd_right(p1); break;
    case 'u': rc = cmd_pen_up(p1); break;
    case 'd': rc = cmd_pen_down(p1); break;
    case 'a': rc = cmd_audio_alert(p1); break;
    case 'v': rc = cmd_visual_alert(p1); break;
    case '?': rc = cmd_check_turtle_status(); break;
    default:  rc = RC_UNKNOWN_COMMAND; break;
  }
  return rc;
}


void send_command_status(uint8_t rc) {
  char* msg = "??";
  switch (rc) {
    case RC_OK: msg = "OK"; break;
    case RC_UNKNOWN_COMMAND: msg = "CMD?"; break;
    case RC_COMMAND_UNIMPLEMENTED: msg = "UNIMPLEMENTED!"; break;
    default:
      msg = "STATUS?";
  }
  radio_send(msg);
}


void loop() {
  radio_loop(); //read radio input
  logo_check_command_queue();
  show_counters();
}

void show_counters() {
#if defined(SERIAL_DEBUG_DETAILED)
  Serial.print("Enc 1A, 2A, 2B:\t");
  Serial.print(encoder1ACount);
  Serial.print("\t");
  Serial.print(encoder2ACount);
  Serial.print("\t");
  Serial.println(encoder2BCount);
#endif
}

