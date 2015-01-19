//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Logo Turtle
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  15.01.2015  -  Inital Version  (wayne@cannybots.com)

//////////////////////////////////////////////////////////////////////////////////////////////////

#define BOT_NAME "Turtle"                   // custom name (16 chars max)
#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>

// Time based hacks!
#define MAX_TURTLE_MOTOR_SPEED  80
#define MOTION_DELAY_MULTIPLIER 60
#define TURN_DELAY_MULTIPLIER 3


// Pin Assignments

// Motors
#define MOTOR_A1_PIN 0	// motor direction
#define MOTOR_A2_PIN 6  // motor speed (PWM)
#define MOTOR_B1_PIN 1	// motor direction
#define MOTOR_B2_PIN 5	// motor speed (PWM)
#define MOTOR_MAX_SPEED 250 // max speed (0..255)

// Wheel encoders
#define ENCODER1A_PIN 2
#define ENCODER2A_PIN 3
#define ENCODER2B_PIN 4


// Wheel encoder state
volatile unsigned int encoder1ACount = 0;
volatile unsigned int encoder2ACount = 0;
volatile unsigned int encoder2BCount = 0;


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
  Serial.end();
  //Serial.begin(9600);

  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  // encoder
  pinMode(ENCODER1A_PIN, INPUT);
  pinMode(ENCODER2A_PIN, INPUT);
  pinMode(ENCODER2B_PIN, INPUT);
  //attachPinInterrupt(ENCODER1A_PIN, encoderCallbackSensor1A, HIGH);
  //attachPinInterrupt(ENCODER2A_PIN, encoderCallbackSensor2A, HIGH);
  //attachPinInterrupt(ENCODER2B_PIN, encoderCallbackSensor2B, LOW);


  radio_setup();
  /*
    motorSpeed(255, 0);
    delay(1000);
    motorSpeed(0, 255);
    delay(1000);
    motorSpeed(0, 0);
  */
}


int encoderCallbackSensor1A(uint32_t ulPin) {
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




// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {
  _speedA = -constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? LOW : HIGH) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));
  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}



///////////////

void loop() {
  radio_loop(); //read radio input
  logo_check_command_queue();
}

void show_counters() {
  Serial.print("Enc 1A, 2A, 2B:\t");
  Serial.print(encoder1ACount);
  Serial.print("\t");
  Serial.print(encoder2ACount);
  Serial.print("\t");
  Serial.println(encoder2BCount);
}

uint8_t cmd_stop(int16_t p1) {
  motorSpeed(0, 0);
  return RC_OK;
}


uint8_t cmd_forward(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED,MAX_TURTLE_MOTOR_SPEED);
  delay(p1*MOTION_DELAY_MULTIPLIER);
  motorSpeed(0,0);
  return RC_OK;
}

uint8_t cmd_backward(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED,MAX_TURTLE_MOTOR_SPEED);
  delay(p1*MOTION_DELAY_MULTIPLIER);
  motorSpeed(0,0);
  return RC_OK;
}

uint8_t cmd_left(int16_t p1) {
  motorSpeed(MAX_TURTLE_MOTOR_SPEED,-MAX_TURTLE_MOTOR_SPEED);
  delay(p1*TURN_DELAY_MULTIPLIER);
  motorSpeed(0,0);
  return RC_OK;
}

uint8_t cmd_right(int16_t p1) {
  motorSpeed(-MAX_TURTLE_MOTOR_SPEED,MAX_TURTLE_MOTOR_SPEED);
  delay(p1*TURN_DELAY_MULTIPLIER);
  motorSpeed(0,0);
  return RC_OK;
}

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
  Serial.print("Logo cmd:\t");
  Serial.print((char)command);
  Serial.print("\t");
  Serial.println(p1);
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
