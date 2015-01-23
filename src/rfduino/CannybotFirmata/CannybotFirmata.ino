//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Blue Brain Firmata
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  04.10.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////
//#define   SERIAL_DEBUG

// TODO: add client alive ping in serial radio proxy

#define SEND_IRVALS_UPDATE_INTERVAL 15
#define SEND_TEMP_UPDATE_INTERVAL   1000

#define MAX_COMMAND_BUFFER_SIZE 20

#define BOT_NAME "CannyFirmata"                   // custom name (16 chars max)
//#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the Client sketch value


#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_send_formatted(char *fmt, ... );

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

// motor speeds
#define MOTOR_MAX_SPEED 255


// state

bool toggle_ir_send = false;

void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
#else
  Serial.end();
#endif  // Motor pins
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
  radio_setup();
#if defined(SERIAL_DEBUG)
  Serial.println("Cannybots Firmata Started");
#endif
}

void loop() {
  radio_loop();
  firmata_check_command_queue();

  if (toggle_ir_send) {
    readAndSendIRSensorValues();
  }

  readAndSendTempValue();
}

// READ & PROCESS IR VALUES
void readAndSendIRSensorValues() {
  int IRval1 = analogRead(IR1_PIN); //left looking from behind
  if (IRval1 >= 1000)
    IRval1 = 1000;

  int IRval2 = analogRead(IR2_PIN); //centre
  if (IRval2 >= 1000)
    IRval2 = 1000;

  int IRval3 = analogRead(IR3_PIN); //right
  if (IRval3 >= 1000)
    IRval3 = 1000;

  // keep alive ping
  static unsigned long nextSendTime = millis() + SEND_IRVALS_UPDATE_INTERVAL;
  if (millis() > nextSendTime) {
    radio_send_formatted("IR:%d,%d,%d", IRval1, IRval2, IRval3);
    nextSendTime = millis() + SEND_IRVALS_UPDATE_INTERVAL;
  }

}

void readAndSendTempValue() {
  static unsigned long nextSendTime = millis() + SEND_TEMP_UPDATE_INTERVAL;
  if (millis() > nextSendTime) {
    float temp = RFduino_temperature(CELSIUS);
    radio_send_formatted("TEMP:%d.%d", (int)temp, (int) ((int)(temp*100.0) % 100));
    nextSendTime = millis() + SEND_TEMP_UPDATE_INTERVAL;
  }
}


// MOTOR DRIVER
void motorSpeed(int _speedA, int _speedB) {

  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
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


void received_client_disconnect () {
}

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
    default:
      msg = "STATUS?";
  }
#if defined(SERIAL_DEBUG)
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
    case 'm': rc = cmd_set_motor_speeds(data + 3, len - 3); break;
    case 's': rc = cmd_set_motor_speeds(0, 0); break;
    default:  rc = RC_UNKNOWN_COMMAND; break;
  }
  return rc;
}

uint8_t cmd_set_motor_speeds(char *data, int len) {
  if (len < 2) {
    return RC_COMMAND_NOT_ENOUGH_PARAMS;
  }
  int speedA =  map(data[0], 0, 255, -255, 255);
  int speedB =  map(data[1], 0, 255, -255, 255);
#if defined(SERIAL_DEBUG)
  Serial.print("Motor speed A,B=");
  Serial.print(speedA);
  Serial.print(",");
  Serial.print(speedB);
#endif
  motorSpeed( speedA,  speedB );
  return RC_OK;
}

