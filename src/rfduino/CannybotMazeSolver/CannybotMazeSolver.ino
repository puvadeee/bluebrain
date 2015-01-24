//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cannybots Maze Solver Drone
//
// Authors:  Wayne Keenan
//
// License: http://opensource.org/licenses/MIT
//
// Version:   1.0  -  23.10.2014  -  Inital Version  (wayne@cannybots.com)
//////////////////////////////////////////////////////////////////////////////////////////////////
//#define SERIAL_DEBUG

// Notes:


// Function that are almost unchanged and can be considred the same as before:  setup, readIRSensors, motorSpeed & calculatePID

// Significant changes:
//   loop -                           first pass at a discriminator for left/right/t-junction,
//                                    notifies client of state (determined corner) and stops if not on the line.
//                                    uses smoothed IR values and a counter
//

// New functions:

//  calibrateIRSensors():              a simple IR sensor min/max (bot must be place onm the line on powerup)
//                                     1. bot spins round and records min & max values read for each IR sensor
//                                     2a) IR1 & IR3 have their whitethreshol set to 95% of max
//                                     2b) IR2 has it own whitethreshold set to 90% of max

//  send_status :                      notifies client of a trasistion betwween line discrinatorstates, e.g. LINE_STATUS_FOLLOWING_LINE, LINE_STATUS_RIGHT_TURN
//                                     surpresses duplicates

//  received_client_disconnect :       called when client radio disconneccts

//  received_client_data:              over the air data, raw input, not filterd or processed by Radio.ino  (unlike the Joypad stuf fin the racer)

//  calculate_smoothing:               perform a rolling average filter, used byt the IS_ON_LINE/NOT_ON_LINE helpers
//                                     smoothing is applied to the IR vals for the discriminator in loop(), however the orginal raw (unsmoothed) values are still used by the PID loop.

//
// Helpers
//  IS_ON_LINE(irNum:1-3) / NOT_ONLINE(irNum:1-3)        uses the smoothed values to determine is sensor x is on or of the line.






#define IR1_BIAS 0
#define IR2_BIAS 0
#define IR3_BIAS 0

// ****** IMPORTANT: Very important this is tuned!

#define IR_WHITE_THRESHOLD_DEFAULT 850 //to determinie on line or not 

// PID gain setting
#define PID_P 50  //PID gains set below in the code.. 
#define PID_I 0
#define PID_D 400
#define PID_SAMPLE_TIME 5 //determine how fast the loop is executed




#define DEFAULT_CRUISE_SPEED 80
//Kp = 30; Ki = 0; Kd = 300; //20,200 //35,300 for 50mm tail
//Kp = 50; Ki = 0; Kd = 500; //laser cut

//#define   SERIAL_DEBUG            // enablign this will print message to serial and also disable the motors.

#define BOT_NAME "CannyMaze"                   // custom name (16 chars max)
#define GZLL_HOST_ADDRESS 0x99ACB010           // this needs to match the Client sketch value

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

// Line status constants

#define LINE_STATUS_UNKNOWN           -1
#define LINE_STATUS_NO_LINE           0
#define LINE_STATUS_FOLLOWING_LINE    1
#define LINE_STATUS_RIGHT_TURN        2
#define LINE_STATUS_LEFT_TURN         3
#define LINE_STATUS_T_JUNCTION        4


// Helper Macros

//#define IS_ON_LINE(x) ( (x)>=IRwhiteThreshold )
//#define NOT_ON_LINE(x) (!IS_ON_LINE(x))




#define IS_ON_LINE(x) ( (average[(x)-1])>=IRwhiteThreshold[(x)-1] )
#define NOT_ON_LINE(x) (!IS_ON_LINE(x))




//set IR initial reading to zero
int IRvals[3] = {0};
int IRvalMin[3] = {0};
int IRvalMax[3] = {0};
int IRwhiteThreshold[3] = {0};


// Analog smoothing

const int numReadings = 5;
int readings[3][numReadings] = {0};      // the readings from the analog input
int sindex[3] = {0};                  // the index of the current reading
int total[3] = {0};                  // the running total
int average[3] = {0};                // the average for each IR





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
  Serial.println("Cannybots Maze Solver Started in Serial Debug Mode- Motors are DISABLED!");
#else
  Serial.end();
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
#endif  // Motor pins
  setup_smoothing();
  radio_setup();
  delay(500);
  calibrateIRSensors();
}



// READ & PROCESS IR VALUES
void readIRSensors() {

  //analogRead(IR1_PIN);
  //delay(20);
  IRvals[0] = analogRead(IR1_PIN) + IR1_BIAS; //left looking from behind
  if (IRvals[0] >= 1000)
    IRvals[0] = 1000;

  //analogRead(IR2_PIN);
  IRvals[1] = analogRead(IR2_PIN) + IR2_BIAS; //centre
  if (IRvals[1] >= 1000)
    IRvals[1] = 1000;

  //analogRead(IR3_PIN);
  IRvals[2] = analogRead(IR3_PIN) + IR3_BIAS; //right
  if (IRvals[2] >= 1000)
    IRvals[2] = 1000;

  calculate_smoothing();
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



int lastStatus = LINE_STATUS_UNKNOWN;
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

    radio_send_formatted("MAZE:%c,%d,%d,%d", c, IRvals[0], IRvals[1], IRvals[2]);
  }

}


void received_client_disconnect () {
}


static bool sendIRStatsFlag = 0;

// do as little in here as possible as it's called under an interrupt.
void received_client_data(char *data, int len)  {
  // Commands: find line left, find line right, halt,
  // settings: p, i d, whitethress
  if (len > 0) {
    //radio_send_formatted("RCV:%d", data[0]);

    //if (data[0] == 'c')  calibrateIRSensors();
  }

  sendIRStatsFlag = 1;
}

void sendIRStats() {
  radio_send_formatted("--------IR State");
  delay(150);
  radio_send_formatted("raw:%d,%d,%d  ", IRvals[0], IRvals[1], IRvals[2]);
  delay(150);
  radio_send_formatted("ave:%d,%d,%d  ", average[0], average[1], average[2]);
  delay(150);
  radio_send_formatted("ol?:%d,%d,%d  ", IS_ON_LINE(1), IS_ON_LINE(2), IS_ON_LINE(3));
  sendIRStatsFlag = 0;
}



// CALCULATE PID
int calculatePID() {

  // Calculate PID on a regular time basis
  if ((time_Now - pidLastTime) < PID_SAMPLE_TIME ) {
    // return previous vakyeif called too soon
    return correction;
  }
  pidLastTime = time_Now;

  // process IR readings via PID
  error_last = error; // store previous error value before new one is caluclated
  error = IRvals[0] - IRvals[2];
  P_error = error * Kp / 100.0; // calculate proportional term
  I_sum = constrain ((I_sum + error), -1000, 1000); // integral term
  I_error = I_sum * Ki / 100.0;
  D_error = (error - error_last) * Kd / 100.0;          // calculate differential term
  correction = P_error + D_error + I_error;
  return correction;
}

int counterL = 0;
int counterR = 0;
int counterT = 0;
int counterMax = 10;  // samples

void loop() {

  time_Now = millis(); //record time at start of loop
  radio_loop(); //read radio input
  readIRSensors(); //read IR sensors



  if ( IS_ON_LINE(2) )
  {
    if (IS_ON_LINE(1) && IS_ON_LINE(3) )                // T-Junction
    {
      counterT++;
      if (counterT > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
        send_status(LINE_STATUS_T_JUNCTION);
      }
    }
    else if (IS_ON_LINE(1) &&  NOT_ON_LINE(3) )          // Left Corner
    {
      counterL++;
      if (counterL > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
        send_status(LINE_STATUS_LEFT_TURN);
      }
    }
    else if (NOT_ON_LINE(1) &&  IS_ON_LINE(3) )         // Right C
    {
      counterR++;
      if (counterR > counterMax) {
        counterT = counterR = counterL = 0;
        correction = 0;
        lineSpeed = 0;
        send_status(LINE_STATUS_RIGHT_TURN);
      }
    } else {                                            // Line Following
      correction = calculatePID();
      lineSpeed = DEFAULT_CRUISE_SPEED;
      send_status(LINE_STATUS_FOLLOWING_LINE);
    }
  } else
  {
    correction = 0;
    lineSpeed = 0;
    send_status(LINE_STATUS_NO_LINE);                    // Off the line
  }

  speedA = (lineSpeed + correction);
  speedB = (lineSpeed - correction);
  motorSpeed(speedA, speedB);

  if (sendIRStatsFlag) {
    sendIRStats();
  }
}



void calibrateIRSensors() {
  radio_send_formatted("MAZE:Calibrating...");
  // take intial readings and set the min/max as those.
  readIRSensors();
  IRvalMin[0] = IRvalMax[0] = IRvals[0];
  IRvalMin[1] = IRvalMax[1] = IRvals[1];
  IRvalMin[2] = IRvalMax[2] = IRvals[2];
  for (int i = -1; i <= 1; i += 1) {
    motorSpeed( i * DEFAULT_CRUISE_SPEED / 3, -(i * DEFAULT_CRUISE_SPEED / 3));
    for (int i = 0; i < 100; i++) {

      readIRSensors();
      for (int j = 0; j < 3; j++) {
        // find min/max
        if (IRvals[j]  < IRvalMin[j]) IRvalMin[j] = IRvals[j];
        if (IRvals[j]  > IRvalMax[j]) IRvalMax[j] = IRvals[j];
        delay(10);
      }

    }
  }
  IRwhiteThreshold[0] = IRvalMax[0] * 0.98;
  IRwhiteThreshold[1] = IRvalMax[1] * 0.90;
  IRwhiteThreshold[2] = IRvalMax[2] * 0.98;
  
  radio_send_formatted("----IR Calibration");
  delay(150);
  for  (int j = 0; j < 3; j++) {
    radio_send_formatted("IR%d:%d..%d=%d", j, IRvalMin[j],  IRvalMax[j],  IRvalMax[j] - IRvalMin[j]);
    delay(150);
  }
  radio_send_formatted("WT:%d,%d,%d", IRwhiteThreshold[0], IRwhiteThreshold[1], IRwhiteThreshold[2]);
  delay(150);

}


void setup_smoothing()
{
  for (int i = 0; i < 3; i++) {
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
      readings[i][thisReading] = 0;
  }
}

void calculate_smoothing() {
  for (int i = 0; i < 3; i++) {
    total[i] = total[i] - readings[i][sindex[i]];
    readings[i][sindex[i]] = IRvals[i];
    total[i] = total[i] + readings[i][sindex[i]];
    sindex[i] = sindex[i] + 1;

    if (sindex[i] >= numReadings)
      sindex[i] = 0;

    average[i] = total[i] / numReadings; //map(total[i], IRvalMin[i], IRvalMax[i], 0, 1000) / numReadings;
  }
}

