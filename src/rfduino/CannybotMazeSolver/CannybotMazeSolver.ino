//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Maze Solver
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  23.10.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////
//#define   SERIAL_DEBUG            // enablign this will print message to serial and also disable the motors.

#define BOT_NAME "CannyMaze"                   // custom name (16 chars max)
//#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the Client sketch value

#include <RFduinoGZLL.h>
#include <RFduinoBLE.h>
void radio_send_formatted(char *fmt, ... );

///////////////////////////////////////////////////////////////////////////////////////
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
#define IR_WHITE_THRESHOLD_DEFAULT 850 //to determinie on line or not 

//IR sensor bias
#define IR1_BIAS 50
#define IR2_BIAS 0
#define IR3_BIAS 0
// PID gain setting
#define PID_P 5  //PID gains set below in the code.. 
#define PID_I 0
#define PID_D 1
#define PID_SAMPLE_TIME 5 //determine how fast the loop is executed

// Line satus constants

#define LINE_STATUS_UNKNOWN           -1
#define LINE_STATUS_NO_LINE           0
#define LINE_STATUS_FOLLOWING_LINE    1
#define LINE_STATUS_RIGHT_TURN        2
#define LINE_STATUS_LEFT_TURN         3
#define LINE_STATUS_T_JUNCTION        4


//
int IRwhiteThreshold = IR_WHITE_THRESHOLD_DEFAULT;

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
volatile int16_t  zAxisValue    = 100;

// LineFollowing State
bool isLineFollowingMode = false;

// Timers in milli-seconds (1/1000 of a second)
unsigned long time_Now = millis();                    // the time at the start of the loop()
unsigned long pidLastTime = millis();                // when the PID was calculated last

// motor speeds
int lineSpeed = 0;
int speedA = 0;
int speedB = 0;


///////////////////////////////////////////////////////////////////////////////////////
// Setup

void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
  Serial.println("Cannybots Maze Solver Started");

#else
  Serial.end();
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
#endif  // Motor pins

  radio_setup();
}



void loop() {

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors

  if (IRval2 >= IRwhiteThreshold)
  {
    isLineFollowingMode = true;
    calculatePID();
    zAxisValue = 100;
    send_status(LINE_STATUS_FOLLOWING_LINE);
  }
  else
  {
    isLineFollowingMode = false;
    correction = 0;
    zAxisValue = 0;
    send_status(LINE_STATUS_NO_LINE);
  }

  speedA = (zAxisValue + correction);
  speedB = (zAxisValue - correction);
  motorSpeed(speedA, speedB);

  // TODO:  check IR sensors current value (and past?) to determine if we are at a junction/choice point


#if defined(SERIAL_DEBUG)

  Serial.print(IRval1, DEC);
  Serial.print("\t");
  Serial.print(IRval2, DEC);
  Serial.print("\t");
  Serial.print(IRval3, DEC);
  Serial.print("\n");
#endif

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
  //Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
  Kp = 50; Ki = 0; Kd = 500; //laser cut
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
#if defined(SERIAL_DEBUG)
  return;
#endif

  _speedA = constrain(_speedA, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  _speedB = constrain(_speedB, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  digitalWrite(MOTOR_A1_PIN, _speedA >= 0 ? HIGH : LOW) ;
  analogWrite (MOTOR_A2_PIN, abs(_speedA));

  digitalWrite(MOTOR_B1_PIN, _speedB >= 0 ? HIGH : LOW);
  analogWrite (MOTOR_B2_PIN, abs(_speedB));
}



///////////////////////////////////////////////////////////////////////////////////////
// Radio



int lastStatus = -1;
void send_status(int status) {
  char c = '?';
  if (status != lastStatus) {
    lastStatus = status;
    switch (status) {
      case LINE_STATUS_UNKNOWN: c = '?';  break;
      case LINE_STATUS_NO_LINE: c = 'N';  break;
      case LINE_STATUS_FOLLOWING_LINE: c = 'O';  break;
      case LINE_STATUS_RIGHT_TURN: c = 'R';  break;
      case LINE_STATUS_LEFT_TURN: c = 'L';  break;
      case LINE_STATUS_T_JUNCTION: c = 'T';  break;
      default: c = 'x';  break;
    }

    radio_send_formatted("MAZE:%c", c);
  }

}

void received_client_disconnect () {
}

// do as little in here as possible as it's called under an interrupt.
void received_client_data(char *data, int len)  {
  // Commands: find line left, find line right, halt,
  // settings: p, i d, whitethress
}

