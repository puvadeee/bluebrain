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
#define SERIAL_DEBUG

#define BOT_NAME "RobotArm"                   // custom name (16 chars max)

#include <RFduinoBLE.h>
// Can't include GZLL, interferes with Servos (SDK v.2.1.2)

#include <Servo.h>

void radio_send_formatted(char *fmt, ... );

#define NUM_SERVOS   4

// PIN ASSIGNEMENT

#define MAX 180

#define SERVO1_PIN 2
#define SERVO2_PIN 3
#define SERVO3_PIN 4
#define SERVO4_PIN 6

const int servoPins[NUM_SERVOS] = { SERVO1_PIN, SERVO2_PIN, SERVO3_PIN, SERVO4_PIN};

Servo servos[NUM_SERVOS];


void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
  
  Serial.print("Max servos:");
  Serial.println(MAX_SERVOS);

#else
  Serial.end();
#endif  

  radio_setup();
  attachServos();
}

void attachServos() {
  for (int i = 0; i < NUM_SERVOS; i++)
    servos[i].attach(servoPins[i]);
}
void detachServos() {
  for (int i = 0; i < NUM_SERVOS; i++)
    servos[i].detach();
}


void loop() {
  radio_loop(); //read radio input                                                                                 b
  //for (int i = 0; i< NUM_SERVOS; i++)
  //    servos[i].write(MAX/2);
  firmata_check_command_queue();
  return;
  servos[0].write(45);
  servos[1].write(45);
  servos[2].write(45);
  servos[3].write(45);

  int servoPos1 = analogRead(0);
  int servoPos2 = analogRead(1);
  int servoPos3 = analogRead(5);
  Serial.print("SPOS:");
  Serial.print(servoPos1);
  Serial.print(",");
  Serial.print(servoPos2);
  Serial.print(",");
  Serial.println(servoPos3);
}



void received_client_disconnect () {
}




////////////////


// Command statuses
#define RC_OK                             0
#define RC_UNKNOWN_COMMAND                1
#define RC_COMMAND_UNIMPLEMENTED          2
#define RC_COMMAND_TOO_SHORT              3
#define RC_COMMAND_NOT_ENOUGH_PARAMS      4

// Command processing state
enum command_state_t {COMMAND_IDLE, COMMAND_READY, COMMAND_RUNNING} ;

// Comman processing
uint8_t  commandProcessingState = COMMAND_IDLE;
char     nextCommandBuffer[20 + 1];
int16_t  nextCommandLength = 0;



// do as little in here as possible as it's called under an interrupt.
void received_client_data(char *data, int len)  {
  if (commandProcessingState != COMMAND_IDLE)
    return;
  memcpy( nextCommandBuffer, data, min(sizeof(nextCommandBuffer) - 1, len));
  nextCommandLength = len;
  commandProcessingState = COMMAND_READY;
}

void firmata_check_command_queue() {
  if ( commandProcessingState == COMMAND_READY ) {
    commandProcessingState = COMMAND_RUNNING;
#if defined(SERIAL_DEBUG)
    Serial.write((const uint8_t*)nextCommandBuffer, nextCommandLength);
#endif
    uint8_t rc = firmata_run_command(nextCommandBuffer, nextCommandLength);
    send_command_status(rc);
    commandProcessingState = COMMAND_IDLE;
  }
}


void send_command_status(uint8_t rc) {
  char* msg = "??";
  switch (rc) {
    case RC_OK: msg = "OK"; break;
    case RC_UNKNOWN_COMMAND: msg = "CMD?"; break;
    case RC_COMMAND_UNIMPLEMENTED: msg = "UNIMPLEMENTED!"; break;
    case RC_COMMAND_TOO_SHORT: msg = "CMD_TOO_SHORT"; break;
    case RC_COMMAND_NOT_ENOUGH_PARAMS: msg = "NOT_ENOUGH_PARAMS"; break;
    default:
      msg = "STATUS?";
  }
#if defined(SERIAL_DEBUG)
  Serial.print("RC=");
  Serial.print(rc,DEC);
  Serial.print(", ");
  Serial.println(msg);
#endif
  radio_send(msg);
}


uint8_t firmata_run_command(char *data, int len) {
  if (len < 3) {
    return RC_COMMAND_TOO_SHORT;
  }
  uint8_t rc = RC_UNKNOWN_COMMAND;
  uint8_t command = data[2];

#if defined(SERIAL_DEBUG)
  Serial.print("Command type, len=");
  Serial.print(command);
  Serial.print(",");
  Serial.println(len);
#endif

  switch (command) {
    case '1': rc = cmd_set_servo_positions(0, data + 3, len - 3); break;
    case '2': rc = cmd_set_servo_positions(1, data + 3, len - 3); break;
    case '3': rc = cmd_set_servo_positions(2, data + 3, len - 3); break;
    case '4': rc = cmd_set_servo_positions(3, data + 3, len - 3); break;
    default:  rc = RC_UNKNOWN_COMMAND; break;
  }
  return rc;
}

uint8_t cmd_set_servo_positions(int servo, char *data, int len) {

  if (len < 1 ) { //NUM_SERVOS) {
    return RC_COMMAND_NOT_ENOUGH_PARAMS;
  }
#if defined(SERIAL_DEBUG)
  Serial.print("Servo Speeds: ");
  for (int i = 0; i < NUM_SERVOS; i++) {
    Serial.print(data[i], DEC); 
    Serial.print(", "); 
  }
  Serial.println("");
#endif
  servos[servo].write(data[1]);

  return RC_OK;
}

